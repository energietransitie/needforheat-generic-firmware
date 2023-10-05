#include "presence_detection.hpp"
#include "control_panel.hpp"

#include <cstdlib>
#include <vector>
#include <set>
#include <regex>
#include <string>

#include <esp_log.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>
#include <esp_gap_bt_api.h>
#include <esp_a2dp_api.h>

#include <esp_bt_device.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>

#include <scheduler.hpp>
#include <measurements.hpp>
#include <secure_upload.hpp>
#include <util/error.hpp>
#include <util/delay.hpp>
#include <util/format.hpp>
#include <util/nvs.hpp>
#include <util/strings.hpp>

constexpr const char *TAG = "Presence detection";
constexpr const char *NVS_NAMESPACE = "twomes_storage";

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION_PARALLEL
constexpr int RESPONSE_MAX_WAIT_MS = 20 * 1000; // 20 seconds.
#else
constexpr int RESPONSE_MAX_WAIT_MS = 10 * 1000; // 10 seconds.
#endif // CONFIG_TWOMES_PRESENCE_DETECTION_PARALLEL

constexpr const char *MEASUREMENT_PROPERTY_NAME = "occupancy__p";


// Event for when all sent responses have returned.
constexpr EventBits_t EVENT_RESPONSES_FINISHED = 1 << 0;
constexpr EventBits_t EVENT_RESPONSE_RECEIVED = 1 << 1;

namespace PresenceDetection
{
	namespace
	{
		/**
		 * This class holds a MAC-address and can be
		 * saved on the stack (no pointers used).
		 *
		 * It can be used just like a esp_bd_addr_t.
		 */
		class MACAddress
		{
		public:
			MACAddress() = default;

			/**
			 * Construct a new MACAddress type with an esp_bd_addr_t.
			 */
			MACAddress(const esp_bd_addr_t &mac)
				: m_octet1(mac[0]),
				  m_octet2(mac[1]),
				  m_octet3(mac[2]),
				  m_octet4(mac[3]),
				  m_octet5(mac[4]),
				  m_octet6(mac[5]) {}

			/**
			 * Construct a new MACAddress type from a string.
			 *
			 * The string needs to look like this "AB:FF:0C:FC:BA:5C".
			 */
			MACAddress(const std::string &mac)
			{
				std::regex regexString("^([0-9a-fA-F]{2})[:-]([0-9a-fA-F]{2})[:-]([0-9a-fA-F]{2})[:-]([0-9a-fA-F]{2})[:-]([0-9a-fA-F]{2})[:-]([0-9a-fA-F]{2})$");

				std::smatch matches;
				auto matchFound = std::regex_search(mac, matches, regexString);
				if (!matchFound)
				{
					ESP_LOGE(TAG, "Provided string in MACAddress constructor is not valid.");
					return;
				}

				// Convert the matches from strings that hold HEX values into uint8_t
				// and assign them to this class' members.
				m_octet1 = std::strtoul(matches.str(1).c_str(), nullptr, 16);
				m_octet2 = std::strtoul(matches.str(2).c_str(), nullptr, 16);
				m_octet3 = std::strtoul(matches.str(3).c_str(), nullptr, 16);
				m_octet4 = std::strtoul(matches.str(4).c_str(), nullptr, 16);
				m_octet5 = std::strtoul(matches.str(5).c_str(), nullptr, 16);
				m_octet6 = std::strtoul(matches.str(6).c_str(), nullptr, 16);
			}

			/**
			 * Assign a MACAddress using an esp_bd_addr_t type.
			 */
			MACAddress &operator=(const esp_bd_addr_t &mac)
			{
				m_octet1 = mac[0];
				m_octet2 = mac[1];
				m_octet3 = mac[2];
				m_octet4 = mac[3];
				m_octet5 = mac[4];
				m_octet6 = mac[5];

				return *this;
			}

			/**
			 * Define conversion from MACAddress to uint8_t* (esp_bd_addr_t).
			 */
			operator uint8_t *()
			{
				// It is expected that we return an array.
				// Return the first octet, because all octets are stored next to each other.
				// This was tested and is works without memory errors, but if something
				// fails, check this first.
				return &m_octet1;
			}

		private:
			// Storage for the octets.
			// They are stored as 6 'normal' variables,
			// to make this class easy to copy (when saving to NVS).
			uint8_t m_octet1, m_octet2, m_octet3, m_octet4, m_octet5, m_octet6;
		};

		// List of MAC-addresses to check, retrieved from NVS.
		static std::vector<MACAddress> s_macAddresses;

		static bool s_initialized = false;

		static int s_responseCount = 0;

		static EventGroupHandle_t s_events = xEventGroupCreate();

		static QueueHandle_t s_btMutex = xSemaphoreCreateMutex();

		auto secureUploadQueue = SecureUpload::Queue::GetInstance();

		/**
		 * Callback that runs on a BT event.
		 */
		void GapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
		{
			if (event == ESP_BT_GAP_READ_REMOTE_NAME_EVT)
			{
				std::string remoteName = reinterpret_cast<const char *>(param->read_rmt_name.rmt_name);

				if (!remoteName.empty())
					s_responseCount++;

				// Signal that a response came in.
				xEventGroupSetBits(s_events, EVENT_RESPONSE_RECEIVED);

				// Signal that ALL responses have come in.
				if (s_responseCount == s_macAddresses.size())
				{
					xEventGroupSetBits(s_events, EVENT_RESPONSES_FINISHED);
					ESP_LOGD(TAG, "All name requests have returned.");
				}
			}
			else if (event == ESP_BT_GAP_AUTH_CMPL_EVT)
			{
				MACAddres::addOnboardedSmartphone(param);
			}
		}

		/**
		 * Callback that runs on A2DP event.
		 */
		void A2DPCallback(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
		{
			if (event == ESP_A2D_CONNECTION_STATE_EVT && param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED)
			{
				// Device connected. Immediately disconnect.
				esp_bt_gap_remove_bond_device(param->conn_stat.remote_bda);
			}
		}

		/**
		 * Retrieve MAC-addresses from NVS.
		 */
		esp_err_t InitializeMacAddresses()
		{
			std::string macAddressList;
			auto err = NVS::Get(NVS_NAMESPACE, NVS_ONBOARDED_MAC_ADDRESSES_KEY, macAddressList);
			if (err == ESP_ERR_NVS_NOT_FOUND)
			{
				ESP_LOGW(TAG, "Retrieving Bluetooth MAC-addresses for presence detection from NVS was unsuccessful. "
							  "It is possible that no MAC-addresses were added to NVS storage.");
				// Not critical and we already logged this.
			}
			else if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "An error occured when reading MAC-addresses from NVS.");
				return err;
			}

			// empty s_macAddresses to prevent mulitple occasions of the same phone
			while (!s_macAddresses.empty())
			{
				s_macAddresses.pop_back();
			}

			auto macAddressStrings = Strings::Split(macAddressList, ';');

			// Add all MAC-address strings to s_macAddresses,
			// which converts them to esp_bd_addr_t.
			for (const auto &address : macAddressStrings)
			{
				s_macAddresses.push_back(MACAddress(address));
			}

			return ESP_OK;
		}

		/**
		 * Send BT name requests to all addresses in s_macAddresses.
		 */
		esp_err_t SendNameRequests()
		{
			for (auto &macAddress : s_macAddresses)
			{
				auto err = esp_bt_gap_read_remote_name(macAddress);
				if (Error::CheckAppendName(err, TAG, "An error occured when sending BT name request"))
					return err;

#ifndef CONFIG_TWOMES_PRESENCE_DETECTION_PARALLEL
				// Wait for the current name request to respond or timeout.
				xEventGroupWaitBits(s_events, EVENT_RESPONSE_RECEIVED, pdTRUE, pdTRUE, Delay::MilliSeconds(RESPONSE_MAX_WAIT_MS));
#endif // CONFIG_TWOMES_PRESENCE_DETECTION_PARALLEL
			}

			ESP_LOGD(TAG, "Sent %d name request(s).", s_macAddresses.size());

			return ESP_OK;
		}
	} // namespace

	esp_err_t InitializeBluetooth(InitializeOptions options)
	{
		ESP_LOGD(TAG, "Enabling Bluetooth");

		xSemaphoreTake(s_btMutex, portMAX_DELAY);

		esp_bt_controller_config_t bluetoothConfig = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

		auto err = esp_bt_controller_init(&bluetoothConfig);
		if (Error::CheckAppendName(err, TAG, "An error occured when initializing BT controller"))
			return err;

		err = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
		if (Error::CheckAppendName(err, TAG, "An error occured when enabling BT controller"))
			return err;

		err = esp_bluedroid_init();
		if (Error::CheckAppendName(err, TAG, "An error occured when initializing bluedroid"))
			return err;

		err = esp_bluedroid_enable();
		if (Error::CheckAppendName(err, TAG, "An error occured when enabling bluedroid"))
			return err;

		err = esp_ble_gatt_set_local_mtu(500);
		if (Error::CheckAppendName(err, TAG, "An error occured when setting local MTU"))
			return err;

		err = esp_bt_dev_set_device_name(getDevName().c_str());
		if (Error::CheckAppendName(err, TAG, "An error occured when setting device name"))
			return err;

		err = esp_bt_gap_register_callback(GapCallback);
		if (Error::CheckAppendName(err, TAG, "An error occured when registering GAP callback"))
			return err;

		if (options.EnableA2DPSink)
		{
			ESP_LOGD(TAG, "Enabling A2DP");

			static bool callBackRegistered = false;

			if (!callBackRegistered)
			{
				err = esp_a2d_register_callback(A2DPCallback);
				if (Error::CheckAppendName(err, TAG, "An error occured when registering A2DP callback"))
					return err;

				callBackRegistered = true;
			}

			err = esp_a2d_sink_init();
			if (Error::CheckAppendName(err, TAG, "An error occured when initializing A2DP sink"))
				return err;
		}

		if (options.EnableDiscoverable)
		{
			ESP_LOGD(TAG, "Enabling Bluetooth discoverable");

			err = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
			if (Error::CheckAppendName(err, TAG, "An error occured when setting scan mode"))
				return err;
		}

		return ESP_OK;
	}

	esp_err_t DeinitializeBluetooth(DeinitializeOptions options)
	{
		esp_err_t err;

		if (options.DisableDiscoverable)
		{
			ESP_LOGD(TAG, "Disabling Bluetooth discoverable");

			err = esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
			if (Error::CheckAppendName(err, TAG, "An error occured when setting scan mode"))
				return err;
		}

		if (options.DisableA2DPSink)
		{
			ESP_LOGD(TAG, "Disabling A2DP");

			err = esp_a2d_sink_deinit();
			if (Error::CheckAppendName(err, TAG, "An error occured when deinitializing A2DP sink"))
				return err;
		}

		if (options.DisableBluetooth)
		{
			ESP_LOGD(TAG, "Disabling Bluetooth");

			err = esp_bluedroid_disable();
			if (Error::CheckAppendName(err, TAG, "An error occured when disabling bluedroid"))
				return err;

			err = esp_bluedroid_deinit();
			if (Error::CheckAppendName(err, TAG, "An error occured when deinitializing bluedroid"))
				return err;

			err = esp_bt_controller_disable();
			if (Error::CheckAppendName(err, TAG, "An error occured when disabling BT controller"))
				return err;

			auto err = esp_bt_controller_deinit();
			if (Error::CheckAppendName(err, TAG, "An error occured when deinitializing BT controller"))
				return err;

			xSemaphoreGive(s_btMutex);
		}

		return ESP_OK;
	}

	int UseBluetooth::s_useCount = 0;
	// Keep track of how many users want A2DP to be enabled.
	int UseBluetooth::s_a2dpCount = 0;
	// Keep track how many users want the device to be discoverable.
	int UseBluetooth::s_discoverableCount = 0;

	UseBluetooth::UseBluetooth(InitializeOptions options)
		: m_optionsUsed(options)
	{
		s_useCount++;

		if (options.EnableA2DPSink)
			s_a2dpCount++;

		if (options.EnableDiscoverable)
			s_discoverableCount++;

		InitializeOptions initOptions{};

		// A2DP used for the first time.
		if (s_a2dpCount == 1)
			initOptions.EnableA2DPSink = true;

		// Discoverable used for the first time.
		if (s_discoverableCount == 1)
			initOptions.EnableDiscoverable = true;

		// Any option was just used for the first time.
		if (s_useCount == 1 || s_a2dpCount == 1 || s_discoverableCount == 1)
		{
			auto err = InitializeBluetooth(initOptions);
			Error::CheckAppendName(err, TAG, "An error occured when initializing Bluetooth with UseBluetooth");
		}
	}

	UseBluetooth::~UseBluetooth()
	{
		DeinitializeOptions deinitOptions{};

		s_useCount--;
		if (s_useCount == 0)
			deinitOptions.DisableBluetooth = true;

		if (m_optionsUsed.EnableA2DPSink)
		{
			s_a2dpCount--;

			// A2DP no longer needed.
			if (s_a2dpCount == 0)
				deinitOptions.DisableA2DPSink = true;
		}

		if (m_optionsUsed.EnableDiscoverable)
		{
			s_discoverableCount--;

			// Discoverable no longer needed.
			if (s_discoverableCount == 0)
				deinitOptions.DisableDiscoverable = true;
		}

		// Any option was just used for the last time.
		if (deinitOptions.DisableBluetooth || deinitOptions.DisableA2DPSink || deinitOptions.DisableDiscoverable)
		{
			auto err = DeinitializeBluetooth(deinitOptions);
			Error::CheckAppendName(err, TAG, "An error occured when deinitializing Bluetooth with UseBluetooth");
		}
	}

	void WaitIfBluetoothActive()
	{
		// Wait until mutex is available.
		xSemaphoreTake(s_btMutex, portMAX_DELAY);
		// Immediately release.
		xSemaphoreGive(s_btMutex);
	}

	void addOnboardedSmartphoneToNVS(const esp_bd_addr_t &mac)
	{
		s_macAddresses.push_back(mac);
	}

	void addOnboardedSmartphoneToNVS(const std::string &mac)
	{
		s_macAddresses.push_back(mac);
	}

	void PresenceDetectionTask(void *taskInfo)
	{
		esp_err_t err;

		if (!s_initialized)
		{
			// Add a formatted for the countPresence property.
			Measurements::Measurement::AddFormatter(MEASUREMENT_PROPERTY_NAME, "%d");

			s_initialized = true;
		}

		InitializeOptions initOptions{};
		initOptions.EnableA2DPSink = false;
		initOptions.EnableDiscoverable = false;
		UseBluetooth bt(initOptions);

		err = InitializeMacAddresses();
		Error::CheckAppendName(err, TAG, "An error occured inside PresenceDetection::<anonymous>::InitializeMacAddresses()");

		// Reset responses.
		s_responseCount = 0;

		err = SendNameRequests();
		Error::CheckAppendName(err, TAG, "An error occured inside PresenceDetection::<anonymous>::SendNameRequests()");

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION_PARALLEL
		// Wait for all requests to return or timeout.
		xEventGroupWaitBits(s_events, EVENT_RESPONSES_FINISHED, pdTRUE, pdTRUE, Delay::MilliSeconds(RESPONSE_MAX_WAIT_MS));
#endif // CONFIG_TWOMES_PRESENCE_DETECTION_PARALLEL

		ESP_LOGD(TAG, "Received %d name request response(s).", s_responseCount);

		// Send data to queue.
		Measurements::Measurement measurement(MEASUREMENT_PROPERTY_NAME, s_responseCount);
		secureUploadQueue.AddMeasurement(measurement);

		ESP_LOGD(TAG, "task finished.");
	}

	std::string getDevName()
	{
		constexpr const char * ONBOARDING_PAIR_NAME = "NeedForHeat_OK"; // change also in screen.cpp
		return ONBOARDING_PAIR_NAME;
	}
} // namespace PresenceDetection
