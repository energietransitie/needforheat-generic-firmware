#include <generic_tasks.hpp>

#include <scheduler.hpp>
#include <measurements.hpp>
#include <secure_upload.hpp>
#include <generic_esp_32.hpp>

#ifdef M5STACK_COREINK
#include <specific_m5coreink/battery_voltage.hpp>
#endif // M5STACK_COREINK

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
#include <presence_detection.hpp>
#endif // CONFIG_TWOMES_PRESENCE_DETECTION

#ifdef CONFIG_TWOMES_OTA_FIRMWARE_UPDATE
#include <ota_firmware_updater.hpp>
#endif // CONFIG_TWOMES_OTA_FIRMWARE_UPDATE

namespace GenericTasks
{
	namespace
	{
		auto secureUploadQueue = SecureUpload::Queue::GetInstance();
	} // namespace

	void UploadTask(void *taskInfo)
	{
		auto taskName = Scheduler::GetName(taskInfo).c_str();

		ESP_LOGD(taskName, "Waiting for the other tasks to finish.");

		// Calling WaitforOtherTasks will block this task
		// until the other tasks have finished.
		Scheduler::WaitForOtherTasks(taskInfo);

		secureUploadQueue.Upload();
	}

	void HeartbeatTask(void *taskInfo)
	{
		// Add a measurement formatter for the heartbeat property.
		Measurements::Measurement::AddFormatter("heartbeat", "%d");

		static int heartbeatCounter = 0;
		heartbeatCounter++;

		Measurements::Measurement measurement("heartbeat", heartbeatCounter);
		secureUploadQueue.AddMeasurement(measurement);
	}

	void TimeSyncTask(void *taskInfo)
	{
		GenericESP32Firmware::InitializeTimeSync();
	}

	void AddTasksToScheduler()
	{
#ifdef M5STACK_COREINK

		Scheduler::AddTask(TimeSyncTask,
						   "Time sync task",
						   4096,
						   nullptr,
						   1,
						   Scheduler::Interval::HOURS_24);

		Scheduler::AddTask(M5CoreInkSpecific::BatteryVoltageTask,
						   "Battery voltage task",
						   4096,
						   nullptr,
						   1,
						   Scheduler::Interval::MINUTES_10);
#endif // M5STACK_COREINK

#ifdef CONFIG_TWOMES_PRESENCE_DETECTION
#ifdef CONFIG_TWOMES_STRESS_TEST
		Scheduler::AddTask(PresenceDetection::PresenceDetectionTask,
						   "Presence detection task",
						   4096,
						   nullptr,
						   1,
						   Scheduler::Interval::MINUTES_1);
#else
		Scheduler::AddTask(PresenceDetection::PresenceDetectionTask,
						   "Presence detection task",
						   4096,
						   nullptr,
						   1,
						   Scheduler::Interval::MINUTES_10);
#endif // CONFIG_TWOMES_STRESS_TEST
#endif // CONFIG_TWOMES_PRESENCE_DETECTION

#ifdef CONFIG_TWOMES_OTA_FIRMWARE_UPDATE
		// Add the firmware updater task to the scheduler.
		Scheduler::AddTask(OTAFirmwareUpdater::OTAFirmwareUpdaterTask,
						   "OTA Firmware update task",
						   16384,
						   nullptr,
						   1,
						   Scheduler::Interval::HOURS_48);
#endif // CONFIG_TWOMES_OTA_FIRMWARE_UPDATE
		// Add tasks to the scheduler. Add the UploadTask as the last task.
		Scheduler::AddTask(HeartbeatTask,
						   "Heartbeat task",
						   4096,
						   nullptr,
						   1,
						   Scheduler::Interval::MINUTES_10);
		Scheduler::AddTask(UploadTask,
						   "Upload task",
						   4096,
						   nullptr,
						   1,
						   Scheduler::Interval::MINUTES_10);
	}
} // namespace GenericTasks
