# Scheduling

The needforheat-generic-firmware library has a scheduler to which measurement tasks can be added. The scheduler will then run the tasks according to each task's specified interval.

## Task time

A task time indicates when a round of measurements should be initiated. We only provide support surrently for tasks that start at full minutes, i.e. when the seconds read `00`. For [Unix timestamps](https://en.wikipedia.org/wiki/Unix_time), this implies that the timestamp is divisible by 60. 

### Current task time

The curent task time is the task time for the current round of measurements. 

### Next task time

The next task time is the earliest task time for all upcoming measurements according to their respective measurement intervals. Shortly after all mesurements that were started by the scheduler at the current task time are completed, the remaining time until the next task time is calculated. When using an [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk), it will turn itself off and restart using its internal RTC[^rtc] when remaining time is more than 2 minutes, to save battery power.

[^rtc]: Real-time clock.

## Measurement intervals

The scheduler runs any measurement task with a specified interval. The interval is used in a modulus operation, which means for example, that an interval of 10 minutes will run at minute `0, 10, 20, 30, 40 and 50` of every hour.

The scheduler supports the following intervals:

| Measurement interval   | Name       |
|------------|------------|
| 1 minute   | MINUTES_1  |
| 2 minutes  | MINUTES_2  |
| 5 minutes  | MINUTES_5  |
| 10 minutes | MINUTES_10 |
| 15 minutes | MINUTES_15 |
| 30 minutes | MINUTES_30 |
| 1 hour     | HOURS_1    |
| 2 hours    | HOURS_2    |
| 6 hours    | HOURS_6    |
| 12 hours   | HOURS_12   |
| 24 hours   | HOURS_24   |
| 48 hours   | HOURS_48   |
| 1 week     | WEEKS_1    |

> Using shorter intervals consume more energy and will shorten the run time of battery-operated measuement devices. Some easurement devices can be powered off in between measurement tasks, in order to energy. For such devices, we calsulate a `POWER_OFF_THRESHOLD_s` value. After all measurements for the current task time are one, if the remaining time until the next task time is below the  `POWER_OFF_THRESHOLD_s`, such a device will not turn off, since the additional energy required to boot no longer outweighs the saved energy to power off the system. For an [M5Stack CoreInk](https://github.com/m5stack/M5-CoreInk) device, this threshold lies betweeen 1 and 2 minutes.

## Add a task to the scheduler

A measurement task can be added to the scheduler as follows:

```cpp title="Example: adding a task to the scheduler"
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

## Start the scheduler

After all the measurement tasks are added to the scheduler, the scheduler can be started:

```cpp title="Example: atarting the scheduler"
#include <scheduler.hpp>

Scheduler::Start();
```

## Scheduler API

Besides adding a measurement task to the scheduler and starting the scheduler, the scheduler exposes some useful functions that measurement tasks can use.

### `Scheduler::GetName(taskInfo)`

This returns a string with the task name given to it when it was added to the scheduler.

```cpp title="Example: get the task name from the scheduler"
#include <scheduler.hpp>

auto taskName = Scheduler::GetName(taskInfo);
ESP_LOGD(taskName.c_str(), "This is a log message with my task name.");
```

### `Scheduler::GetParams(taskInfo)`

This returns a void pointer with the parameters that were passed when the task was added to the scheduler.

```cpp title="Example: get the task parameters from the scheduler"
#include <scheduler.hpp>

void* params = Scheduler::GetParams(taskInfo);
int testNumber = (int)params;
```

### `Scheduler::GetParams<T>(taskInfo)`

This returns a type T with the parameters that were passed when the task was added to the scheduler. No need to cast it yourself.

```cpp title="Example: get the task parameters from the scheduler without needing to cast"
#include <scheduler.hpp>

auto testNumber = Scheduler::GetParams<int>(taskInfo); // (1)!
```

1. `testNumber` type will be an `int` since that is the type used in the template parameter of `Scheduler::GetParams<T>()`.

### `Scheduler::GetID(taskInfo)`

This returns the ID of the task that was automatically given to it by the scheduler.

```cpp title="Example: get the task ID from the scheduler"
#include <scheduler.hpp>

auto id = Scheduler::GetParams<int>(taskInfo);
```

### `Scheduler::WaitForOtherTasks(taskInfo)`

This will block the current task until all other tasks are finished.

```cpp title="Example: wait for all other tasks to finish"
#include <scheduler.hpp>

Scheduler::WaitForOtherTasks(taskInfo);

ESP_LOGD(TAG, "I am the last task that is running."); // (1)!
```

1. This will only run when the other tasks are finished.
