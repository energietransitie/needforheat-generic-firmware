#include <generic_esp_32.hpp>
#include <generic_tasks.hpp>
#include <scheduler.hpp>

constexpr const char *DEVICE_TYPE_NAME = "Generic-Test";

extern "C" void app_main(void)
{
	// Add the generic tasks to the scheduler,
	// heartbeat, timesync and optionally presence detection and OTA updates.
	GenericTasks::AddTasksToScheduler();

	GenericESP32Firmware::Initialize(DEVICE_TYPE_NAME);

	// Start the scheduler.
	Scheduler::Start();
}
