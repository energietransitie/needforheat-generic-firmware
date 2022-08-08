#include <generic_esp_32.hpp>
#include <generic_tasks.hpp>
#include <scheduler.hpp>

#ifdef CONFIG_TWOMES_OTA_FIRMWARE_UPDATE
#include <ota_firmware_updater.hpp>
#endif

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
constexpr const char *TAG = "Twomes ESP32 presence detector";
constexpr const char *DEVICE_TYPE_NAME = "Presence-Detector";
constexpr const char *DEVICE_TYPE_OTA_ORG = "energietransitie";
constexpr const char *DEVICE_TYPE_OTA_REPO = "twomes-generic-esp-firmware";
#else
constexpr const char *TAG = "Twomes ESP32 generic test device";
constexpr const char *DEVICE_TYPE_NAME = "Generic-Test";
constexpr const char *DEVICE_TYPE_OTA_ORG = "energietransitie";
constexpr const char *DEVICE_TYPE_OTA_REPO = "twomes-generic-esp-firmware";
#endif

#if defined ESP32DEV
constexpr const char *DEVICE_TYPE_OTA_FILENAME = "firmware-signed_ESP32DEV.bin";
#elif defined M5STACK_COREINK
constexpr const char *DEVICE_TYPE_OTA_FILENAME = "firmware-signed_M5STACK_COREINK.bin";
#endif

extern "C" void app_main(void)
{
	GenericESP32Firmware::Initialize(DEVICE_TYPE_NAME);

#ifdef CONFIG_TWOMES_OTA_FIRMWARE_UPDATE
	// Set location for the OTA firmware update to check for releases.
	OTAFirmwareUpdater::SetLocation(DEVICE_TYPE_OTA_ORG, DEVICE_TYPE_OTA_REPO, DEVICE_TYPE_OTA_FILENAME);
#endif // CONFIG_TWOMES_OTA_FIRMWARE_UPDATE

	// Add the generic tasks to the scheduler,
	// heartbeat, timesync and optionally presence detection and OTA updates.
	GenericTasks::AddTasksToScheduler();

	// Start the scheduler.
	Scheduler::Start();
}
