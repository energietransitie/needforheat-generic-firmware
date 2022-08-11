#include <generic_esp_32.hpp>
#include <generic_tasks.hpp>
#include <scheduler.hpp>

constexpr const char *DEVICE_TYPE_NAME = "Presence-Detector";

extern "C" void app_main(void)
{
	GenericESP32Firmware::Initialize(DEVICE_TYPE_NAME);

	// Add the generic tasks to the scheduler,
	// heartbeat, timesync and optionally presence detection and OTA updates.
	GenericTasks::AddTasksToScheduler();

	// Start the scheduler.
	Scheduler::Start();
}
