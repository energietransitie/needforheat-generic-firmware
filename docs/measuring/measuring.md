# Measuring

Your measurement device can measure data for one or more properties. For each properties, your measurement device cen take multiple measurements in in time. The [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware) library provides various convenience functions you can use to:

- [add a value formatter](#property-value-formatting)
- [schedule measurement tasks](./scheduling.md)
- [runing measurement tasks for multiple properties](./running_tasks.md)
- [creating and timestamping a measurement](#creating-and-timestamping-a-measurement)
- [adding a timestamped measurement to the secure upload queue](#adding-a-measurement-to-the-upload-queue)
- [securely upload your queued measurements](./uploading.md)

## Registering a measurement property  

Each property has its own:

- property_name
- format
- unit

Before a [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) accepts a property from any measurement device, the property first needs to be [registered on that server](https://github.com/energietransitie/twomes-backoffice-api/blob/main/README.md#creating-new-admin-accounts-to-apitstenergietransitiewindesheimnl).

> In a future version of the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) and [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration), we may drop this requirement.

## Property value formatting

To format your measurement values, your firmware only needs to specify a [printf format string](https://en.wikipedia.org/wiki/Printf_format_string) for each property_name.

```cpp title="Example: adding a property formatter"
Measurements::Measurement::AddFormatter("example", "%d"); // (1)!
```

1. `example` is the property name. `%d` is the [printf format string](https://en.wikipedia.org/wiki/Printf_format_string) that this property uses to format its value before it is sent to the [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) via the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) in a JSON string.

## Creating and timestamping a measurement

Before your firmware creates a measurement instance, it should have added a formatter for the measurement's property, as illustrated in the sectione [Property value formatting](#property-value-formatting) above.

Two ways are available in the  [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware) library for your firmware to attach a timestamp to a measurement.

### Using the current task time timestamp
 If your firmware code does NOT specify a timestamp, the measurement timestamp will be set to the [current task time](scheduling.md#current-task-time) before the measurement is uploaded securely. This is often a good default choice: all measurements of all properties that were started at the same time are guaranteed to have the same timestamp, which often makes analysis easier. 

```cpp title="Example: creating a measurement with the current task time (the default)"
#include <measurements.hpp>

int exampleValue = 1;

Measurements::Measurement exampleMeasurement("example", exampleValue); // (1)!
```

1. Create a new measurement with property type `example` and the value `exampleValue`. Since no timestamp is provided by the firmware, the generic firmware will attach the current task time to this measuerment value before it is uploaded to a [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) via the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api).

### Specifying a custom timestamp
If your firmware code specifies a custom timestamp, then this timestamp will be used when the measurement is uploaded securely. This may be a good choice for measurements that take such a long time to establish that using the [current task time](scheduling.md#current-task-time) no longer seems appropriate. A disadvantage of this option is that it becomes harder during analysis to group measurements of different properties that were started at the same.  

```cpp title="Example: creating a measurement with a custom timestamp"
#include <measurements.hpp>

int exampleValue = 1;
time_t now = time(nullptr);

Measurements::Measurement exampleMeasurement("example", exampleValue, now); // (1)!
```

1. Create a new measurement with property type `example` and the value  `exampleCounter` and timestamp `now`, i.e. the [Unix timestamp](https://en.wikipedia.org/wiki/Unix_time) of the device clock when the code evaluates `time(nullptr)`.


## Adding a measurement to the upload queue

The secure upload queue is a queue in volatile memory of the device, to which measurements can be added. [Uploading queued measurements](./uploading.md) is not done immediately, but at a later moment. 

In order to gain access to the secure upload queue, your measurement task first needs to get an instance of the queue. Each measurement device only has one instance of such a queue, shared by all measurement tasks, which may run in parallel.

A [measurement](#creating-a-new-measurement-object) can be added to the secure upload queue as shown below:

```cpp title="Getting access to the queue and adding a measurement"
#include <secure_upload.hpp>

auto secureUploadQueue = SecureUpload::Queue::GetInstance();

secureUploadQueue.AddMeasurement(exampleMeasurement);
```
