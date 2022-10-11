# Scheduler

The Twomes generic firmware library has a scheduler to which tasks can be added. The scheduler will then run the tasks according to each task's specified interval.

## Task time

This documentation uses some terms regarding task time. A task time is a timestamp, and is always at a full minute and zero seconds.

### Current task time

The curent task time is the timestamp at which the scheduler's current task interval began. 

### Next task time

The next task time is the timestamp at which the scheduler's next task interval will begin. This is calculated to determine how long the device can sleep. When using an M5Stack CoreInk, it will turn itself off and restart using its internal RTC[^rtc] when the next task time is further then 2 minutes in the future, to save battery power.

[^rtc]: Real-time clock.

## Task intervals

The scheduler runs any task with a specified interval. The interval is used in a modulus operation, which means for example, that an interval of 10 minutes will run at minute `0, 10, 20, 30, 40 and 50` of every hour.

The scheduler supports the following intervals:

| Interval   | Name       |
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

> Intervals of less then 2 minutes are not recommended. When using an M5Stack CoreInk, it will not shut down when the next task time is less then 2 minutes.

## Add a task to the scheduler

A task can be added to the scheduler as follows:

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

## Start the scheduler

After all the tasks are added to the scheduler, it has to be started:

```cpp title="Starting the scheduler"
#include <scheduler.hpp>

Scheduler::Start();
```

## Scheduler API

Besides adding a task to the scheduler and starting the scheduler, the scheduler exposes some useful functions that tasks can use.

### `Scheduler::GetName(taskInfo)`

This returns a string with the task name given to it when it was added to the scheduler.

```cpp title="Get the task name from the scheduler"
#include <scheduler.hpp>

auto taskName = Scheduler::GetName(taskInfo);
ESP_LOGD(taskName.c_str(), "This is a log message with my task name.");
```

### `Scheduler::GetParams(taskInfo)`

This returns a void pointer with the parameters that were passed when the task was added to the scheduler.

```cpp title="Get the task parameters from the scheduler"
#include <scheduler.hpp>

void* params = Scheduler::GetParams(taskInfo);
int testNumber = (int)params;
```

### `Scheduler::GetParams<T>(taskInfo)`

This returns a type T with the parameters that were passed when the task was added to the scheduler. No need to cast it yourself.

```cpp title="Get the task parameters from the scheduler without needing to cast"
#include <scheduler.hpp>

auto testNumber = Scheduler::GetParams<int>(taskInfo); // (1)!
```

1. `testNumber` type will be an `int` since that is the type used in the template parameter of `Scheduler::GetParams<T>()`.

### `Scheduler::GetID(taskInfo)`

This returns the ID of the task that was automatically given to it by the scheduler.

```cpp title="Get the task ID from the scheduler"
#include <scheduler.hpp>

auto id = Scheduler::GetParams<int>(taskInfo);
```

### `Scheduler::WaitForOtherTasks(taskInfo)`

This will block the current task until all other tasks are finished.

```cpp title="Wait for all other tasks to finish"
#include <scheduler.hpp>

Scheduler::WaitForOtherTasks(taskInfo);

ESP_LOGD(TAG, "I am the last task that is running."); // (1)!
```

1. This will only run when the other tasks are finished..
