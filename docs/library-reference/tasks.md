# Tasks

The Twomes generic firmware library has a [scheduler](scheduler.md) that runs tasks according to a specified interval. The library has some generic tasks that can be started, but custom tasks can also be easily added.

## Generic tasks

### Overview

The library has some generic tasks that can be started. These tasks are:

| Task                                          | Purpose                                                                        | Default interval |
|-----------------------------------------------|--------------------------------------------------------------------------------|------------------|
| [Upload task](#upload-task)                   | Upload the contents of the secure upload queue to a twomes server.             | 10 minutes       |
| [Heartbeat task](#heartbeat-task)             | Create a heartbeat measurement and place it on the queue.                      | 10 minutes       |
| [Presence detection task](#presence-detection-task) | Check for presence of bluetooth MAC-addresses and place the count on the queue. | 10 minutes |
| [Battery voltage task](#battery-voltage-task) | (On an M5Stack CoreInk) measure the battery voltage and place it on the queue. | 10 minutes       |
| [Time sync task](#time-sync-task)             | Synchronize the system time (on an M5Stack CoreInk).                           | 24 hours         |
| [OTA firmware update task](#ota-firmware-update-task) | Periodically check for OTA firmware updates and install them.          | 48 hours         |

#### Upload task

The upload task uploads all the contents of the [secure upload queue](measurements-and-uploading.md#secure-upload-queue) to a Twomes server.

#### Heartbeat task

The heartbeat task creates a heartbeat measurement and places it on the [secure upload queue](measurements-and-uploading.md#secure-upload-queue). The value of the heartbeat measurement can give insight into the device's operation.

#### Presence detection task

The presence detection task does a measurement of the amount of people present. It tries to do this according to a list of known Bluetooth MAC-addresses that it searches for. Bluetooth MAC-addresses of people's smartphones, which can be retrieved with their consent, can then be used to detect the amount of people. Only the amount of people present, not who specifically, is known and placed on the [secure upload queue](measurements-and-uploading.md#secure-upload-queue).

#### Battery voltage task

The battery voltage measures the M5Stack CoreInk's internal battery voltage and places it on the [secure upload queue](measurements-and-uploading.md#secure-upload-queue).

#### Time sync task

The time sync tasks synchronizes the system time and updates the RTC[^rtc] of the M5Stack CoreInk. This task is only used on the M5Stack CoreInk to update the time periodically, instead of every reboot. The M5Stack CoreInk has needs this, because the RTC[^rtc] already has a much higher accuracy then the stock RTC[^rtc] in an ESP32, but is restarted more than we want a time sync to occur.

[^rtc]: Real-time clock.

#### OTA firmware update task

The OTA firmware update tasks periodically checks for new OTA firmware updates, if enabled. When a new update is found, it will be downloaded and installed. It will then restart once the other tasks are done, and boot the new firmware.

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

## Create a custom task

Beside generic tasks, more tasks can be added to the scheduler. Custom tasks can be created, with some rules, and added to the scheduler.

### Rules

A task is mostly a freeRTOS task, but with some other rules and added information that is used by the [scheduler](scheduler.md).

Any task needs to adhere to these rules:

1. A task is a function that takes a `void *taskInfo`[^taskInfo] as a parameter and does not return anything.
    ```cpp title="The task takes a void pointer and returns nothing"
    void ExampleTask(void *taskInfo /* (1)! */) { ... }
    ```
    1. We use `taskInfo` here instead of `params`, since these are not the parameters that were passed to the task when adding it to the scheduler.
    [^taskInfo]: We use `taskInfo` here instead of `params`, since these are not the parameters that were passed to the task when adding it to the scheduler.
2. A task does not contain infinite loops and it must return.
3. The task should be as short as possible. This will maximize the time that the system can sleep.

### Usage of the scheduler

The scheduler makes some [convenience functions]() available to any task. If your custom task is added to the scheduler, it can use any of those functions.

### Adding a custom task to the scheduler

The scheduler can run your task automatically according to a set interval:

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
	// "example" is the property name. "%d" is how the value must be formatted.
	// Formatting happens according to printf formatting rules: https://cplusplus.com/reference/cstdio/printf/
	Measurements::Measurement::AddFormatter("example", "%d");

	static int exampleValue = 0;
	exampleValue++;

	// Create a new measurement with property type "example" and the value of exampleCounter.
	// The timestamp of this measurement will be the current task time.
	Measurements::Measurement exampleMeasurement("example", exampleValue);

	// Add the measurement to the queue.
	secureUploadQueue.AddMeasurement(exampleMeasurement);
}
```
