# Measurement tasks

The twomes-generic-esp-firmware library has a [scheduler](scheduler.md) that runs tasks according to a specified interval. The library has some generic tasks that can be started. Your measurement device firmware code can add custom measurement tasks for custom properties.

## Generic tasks

The twomes-generic-esp-firmware library comes with support for the following generic tasks:

| Task                                          | Purpose                                                                        | Default interval |
|-----------------------------------------------|--------------------------------------------------------------------------------|------------------|
| [Heartbeat](#heartbeat-task)             | Creates a heartbeat measurement and places it on the queue.                      | 10 minutes       |
| [Battery voltage](#battery-voltage-task) | Measures the battery voltage and places it on the queue. | 10 minutes       |
| [Upload](#upload-task)                   | Uploads the contents of the secure upload queue to a [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration).             | 10 minutes       |
| [Time sync](#time-sync-task)             | Synchronizes the device clock via NTP                           | 24 hours         |
| [OTA firmware update](#ota-firmware-update-task) | Checks for OTA firmware updates and installs them          | 48 hours         |
| [Presence detection](#presence-detection-task) | Sends Bluetooth name requests to specific static MAC addresses and places a response count on the queue. | 10 minutes |

#### Upload task

The upload task uploads all the contents of the [secure upload queue](measurements-and-uploading.md#secure-upload-queue) to a [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration).

#### Heartbeat task

The heartbeat task creates a heartbeat measurement and places it on the [secure upload queue](measurements-and-uploading.md#secure-upload-queue). The timestamp of a heartbeat and its value are intended to give insight in the basic status of a measurement device. This may be useful for monitoring purposes during a measurement campaign. It also may be useful to get insight in the status of measurement devices during the analysis phase after a measurement campaign.

#### Battery voltage task

The battery voltage measures the closed circuit voltage of the battery of a battery-powered measurement device, such as the [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk) and places it on the [secure upload queue](measurements-and-uploading.md#secure-upload-queue).

#### Time sync task

The time sync tasks synchronizes the system time over the internet using [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) and updates the device clock. It also updates the time on the separate RTC[^rtc] chip, for supported devices that have it,like the [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk).

[^rtc]: Real-time clock.

#### OTA firmware update task

If enabled, the Over The Air (OTA) firmware update task periodically checks for new firmware updates, if enabled. When a new firmware update is found, it will be downloaded and installed. It will then restart once the other tasks are done, and boot the new firmware.

#### Presence detection task

If enabled, the presence detection task measures the number of people present nearby via Bluetooth name requests sent to a list of known static MAC-addresses. Such addresses of people's smartphones should only used after obtaining informed consent of each subject involved. 

For each task time, the total number of unique smartphones that respond to such Bluetooth name requests is counted and this count is placed as a measurement on the [secure upload queue](measurements-and-uploading.md#secure-upload-queue). 

In this way, privacy is respected both for people that gave permission to be counted (no individual information about their presence is sent in measurement values, only aggregate information) as well as other persons that are not participating in your measuremetn campaign as subjects (their smartphones don't even get a Bluetooth name request).

### Starting generic tasks

The generic tasks can be added to the [scheduler](scheduler.md) in the `main` function of your code:

```cpp title="Starting generic tasks"
#include <generic_tasks.hpp>

extern "C" void app_main(void)
{
	GenericTasks::AddTasksToScheduler();

	Scheduler::Start(); // (1)!
}
```

1. `AddTasksToScheduler()` only adds the generic tasks to the scheduler. They will not run until the scheduler is started.

## Create a custom measurement task

Beside generic tasks, more tasks can be added to the scheduler. In your firmware code you can create custom measurement tasks for properties that your device measures, use the scheduler to make sure your specific measurements are done at specific times.

### Rules

A Twomes measurement task is freeRTOS task, that adheres to specific rules and for which you need to add specific information that is used by the Twomes [scheduler](scheduler.md).

Any custom Twomes task needs to adhere to these rules:

1. A task is a function that takes a `void *taskInfo`[^taskInfo] as a parameter and should not return anything.
    ```cpp title="The task takes a void pointer and returns nothing"
    void ExampleTask(void *taskInfo /* (1)! */) { ... }
    ```
    1. Use `taskInfo` here instead of `params`; these are not the parameters that were passed to the task when adding it to the scheduler.
    [^taskInfo]: Use `taskInfo` here instead of `params`; these are not the parameters that were passed to the task when adding it to the scheduler.
2. A task should not contain infinite loops and it must return.
3. The task should be as short as possible. This will maximize the time that the system can sleep and maximize the battery lifetime if your measurement device is powered by batteries.

### Usage of the scheduler

The scheduler makes some [convenience functions]() available to any task. If your custom task is added to the scheduler, it can use any of those functions.

### Scheduling your custom task

When you add your custom task to the scheduler, it will run your task automatically according to a specific interval that you choose:

```cpp title="Adding a task to the scheduler"
#include <scheduler.hpp>

Scheduler::AddTask(ExampleTask, // (1)!
	               "Example task", // (2)!
	               4096, // (3)!
	               nullptr, // (4)!
	               1, // (5)!
	               Scheduler::Interval::MINUTES_10); // (6)!
```

1. Task function.
2. Task name.
3. Stack size.
4. Parameters passed into task.
5. Priority.
6. Interval.

### Example task

```cpp title="An example task that increments a counter and adds it to the secure upload queue"
#include <scheduler.hpp>
#include <measurements.hpp>
#include <secure_upload.hpp>

// Get access tot the secure upload queue.
auto secureUploadQueue = SecureUpload::Queue::GetInstance();

void ExampleTask(void *taskInfo)
{
	// "example" is the property name. 
	// "%d" is how the value must be formatted.
	// Formatting happens according to printf formatting rules:
	// see also https://cplusplus.com/reference/cstdio/printf/
	Measurements::Measurement::AddFormatter("example", "%d");

	static int exampleValue = 0;
	exampleValue++;

	// Create a new measurement with property type "example" 
	// and the value of exampleValue,
	// formatted according to the printf specifier "%d".
	// Note that in this example, the timestamp is not explicitely specified
	// so in this case, the secure upload queue will 
	// automatically attach the current task time to this measurement.
	Measurements::Measurement exampleMeasurement("example", exampleValue);

	// Add the measurement to the queue.
	secureUploadQueue.AddMeasurement(exampleMeasurement);
}
```
