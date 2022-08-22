#include "presence_detection.hpp"

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

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

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

constexpr int RESPONSE_MAX_WAIT_MS = 10 * 1000; // 10 seconds.

constexpr const char *MEASUREMENT_PROPERTY_NAME = "countPresence";

// Event for when all sent responses have returned.
constexpr EventBits_t EVENT_RESPONSES_FINISHED = 1 << 0;

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

		auto secureUploadQueue = SecureUpload::Queue::GetInstance();

		/**
		 * Callback that runs on a BT event.
		 */
		void GapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
		{
			if (event == ESP_BT_GAP_READ_REMOTE_NAME_EVT)
			{
				std::string remoteName = reinterpret_cast<const char *>(param->read_rmt_name.rmt_name);

				if (remoteName.empty())
					return;

				s_responseCount++;

				// When the amount of responses match the amount of MAC-addresses, we are done.
				if (s_responseCount == s_macAddresses.size())
				{
					xEventGroupSetBits(s_events, EVENT_RESPONSES_FINISHED);
					ESP_LOGD(TAG, "All name requests have returned.");
				}
			}
		}

		/**
		 * Initialize bluetooth.
		 */
		esp_err_t InitializeBluetooth()
		{
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

			err = esp_bt_gap_register_callback(GapCallback);
			if (Error::CheckAppendName(err, TAG, "An error occured when registering GAP callback"))
				return err;

			return ESP_OK;
		}

		/**
		 * Retrieve MAC-addresses from NVS.
		 */
		esp_err_t InitializeMacAddresses()
		{
			std::string macAddressList;
			auto err = NVS::Get(NVS_NAMESPACE, "mac_addresses", macAddressList);
			if (err == ESP_ERR_NVS_NOT_FOUND)
			{
				ESP_LOGW(TAG, "Retrieving Bluetooth MAC-addresses for presence detection from NVS was unsuccessful. "
							  "It is possible that no MAC-addresses were added to NVS storage.");
				// Not critical and we already logged this. Return ESP_OK.
				return ESP_OK;
			}
			else if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "An error occured when reading MAC-addresses from NVS.");
				return err;
			}

			auto macAddressStrings = Strings::Split(macAddressList, ';');

			// Add all MAC-address strings to s_macAddresses,
			// which converts them to esp_bd_addr_t.
			for (const auto &address: macAddressStrings)
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
			}

			ESP_LOGD(TAG, "Sent %d name request(s).", s_macAddresses.size());

			return ESP_OK;
		}
	} // namespace

	void AddMacAddress(const esp_bd_addr_t &mac)
	{
		s_macAddresses.push_back(mac);
	}

	void AddMacAddress(const std::string &mac)
	{
		s_macAddresses.push_back(mac);
	}

	void PresenceDetectionTask(void *taskInfo)
	{
		esp_err_t err;

		if (!s_initialized)
		{
			err = InitializeBluetooth();
			Error::CheckAppendName(err, TAG, "An error occured inside PresenceDetection::<anonymous>::InitializeBluetooth()");

			err = InitializeMacAddresses();
			Error::CheckAppendName(err, TAG, "An error occured inside PresenceDetection::<anonymous>::InitializeMacAddresses()");

			// Add a formatted for the countPresence property.
			Measurements::Measurement::AddFormatter(MEASUREMENT_PROPERTY_NAME, "%d");

			s_initialized = true;
		}

		// Reset responses.
		s_responseCount = 0;

		err = SendNameRequests();
		Error::CheckAppendName(err, TAG, "An error occured inside PresenceDetection::<anonymous>::SendNameRequests()");

		// Wait for all requests to return or timeout.
		xEventGroupWaitBits(s_events, EVENT_RESPONSES_FINISHED, pdTRUE, pdTRUE, Delay::MilliSeconds(RESPONSE_MAX_WAIT_MS));

		ESP_LOGD(TAG, "Received %d name request response(s).", s_responseCount);

		// Send data to queue.
		Measurements::Measurement measurement(MEASUREMENT_PROPERTY_NAME, s_responseCount);
		secureUploadQueue.AddMeasurement(measurement);

		ESP_LOGD(TAG, "task finished.");
	}
} // namespace PresenceDetection
