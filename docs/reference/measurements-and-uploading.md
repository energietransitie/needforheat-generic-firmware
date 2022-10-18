# Measurements and uploading

The [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware) library allows for easy creation of new measurements. Any measurement value for a property supported by the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) can be created and uploaded to it.

## Measurements

In a single upload, one or more measurements for one or more properties can be uploaded to the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api). The payload of such an upload is a JSON-formatted string with the following structure:

```json5 title="Example: JSON payload of a single heartbeat measurement upload"
{
  "upload_time": "1622237550",
  "property_measurements": [
    {
      "property_name": "heartbeat",
      "measurements": [
        {
          "timestamp": "1622237550",
          "value": "1"
        }
      ]
    }
  ]
}
```

An upload consists of an `upload_time`, which indicates the [Unix timestamp](https://en.wikipedia.org/wiki/Unix_time) taken from the device clock of the measurement device just before it uploads the content, followed by array `property-measurements`, where each element consists of a `property_name` and an array `measurements`. Each measurement in this array needs to have a `timestamp` and a `value`. 
The [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware) library provides various convenience functions for your firmware to securely upload timestamped measurement values to your [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) via a [Twomes API](https://github.com/energietransitie/twomes-backoffice-api). 

Your measurement device can measure data for one or more properties. Each property has its own:

- property_name
- format
- unit

### Registering a property at the [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) 
Before a [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) accepts a property from any measurement device, the property first needs to be [registered on that server](https://github.com/energietransitie/twomes-backoffice-api/blob/main/README.md#creating-new-admin-accounts-to-apitstenergietransitiewindesheimnl).

> In a future version of the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) and [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration), we may drop this requirement.

### Property value formatting


To format your measurement values, your firmware only needs to specify a [printf format string](https://en.wikipedia.org/wiki/Printf_format_string) for each property_name.

```cpp title="Example: adding a property formatter"
Measurements::Measurement::AddFormatter("example", "%d"); // (1)!
```

1. `example` is the property name. `%d` is the [printf format string](https://en.wikipedia.org/wiki/Printf_format_string) that this property uses to format its value before it is sent to the [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) via the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) in a JSON string.

### Create a new measurement instance

Before your firmware creates a measurement instance, it should have added a formatter for the measurement's property, as illustrated in the sectione [Property value formatting](#property-value-formatting) above.

Two ways are available in the  [twomes-generic-esp-firmware](https://github.com/energietransitie/twomes-generic-esp-firmware) library for your firmware to attach a timestamp to a measurement.

#### Using the current task time timestamp
 If your firmware code does NOT specify a timestamp, the measurement timestamp will be set to the [current task time](scheduler.md#current-task-time) before the measurement is uploaded securely. This is often a good default choice: all measurements of all properties that were started at the same time are guaranteed to have the same timestamp, which often makes analysis easier. 

```cpp title="Example: creating a measurement with the current task time (the default)"
#include <measurements.hpp>

int exampleValue = 1;

Measurements::Measurement exampleMeasurement("example", exampleValue); // (1)!
```

1. Create a new measurement with property type `example` and the value `exampleValue`. Since no timestamp is provided by the firmware, the generic firmware will attach the current task time to this measuerment value before it is uploaded to a [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) via the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api).

#### Specifying a custom timestamp
If your firmware code specifies a custom timestamp, then this timestamp will be used when the measurement is uploaded securely. This may be a good choice for measurements that take such a long time to establish that using the [current task time](scheduler.md#current-task-time) no longer seems appropriate. A disadvantage of this option is that it becomes harder during analysis to group measurements of different properties that were started at the same.  

```cpp title="Example: creating a measurement with a custom timestamp"
#include <measurements.hpp>

int exampleValue = 1;
time_t now = time(nullptr);

Measurements::Measurement exampleMeasurement("example", exampleValue, now); // (1)!
```

1. Create a new measurement with property type `example` and the value  `exampleCounter` and timestamp `now`, i.e. the [Unix timestamp](https://en.wikipedia.org/wiki/Unix_time) of the device clock when the code evaluates `time(nullptr)`.

## Secure upload queue

The twomes-generic-esp-firmware library provides a secure upload queue. This is a queue in volatile memory of the device, to which measurements can be added. The secure upload queue will automatically upload queued measurements to a [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) via the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) at a later moment. 

### Adding a measurement to the upload queue

In order to gain access to the secure upload queue, your measurement task first needs to get an instance of the queue. Each measurement device only has one instance of such a queue, shared by all measurement tasks, which may run in parallel.

A [measurement](#creating-a-new-measurement-object) can be added to the secure upload queue as shown below:

```cpp title="Getting access to the queue and adding a measurement"
#include <secure_upload.hpp>

auto secureUploadQueue = SecureUpload::Queue::GetInstance();

secureUploadQueue.AddMeasurement(exampleMeasurement);
```

### Uploading measurements to a [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) via the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api)

All queued measurments will be uploaded to a [Twomes server](https://github.com/energietransitie/twomes-backoffice-configuration) when the [upload task](tasks.md#default-tasks) runs.

<i>Currently, on battery-powerd measurement measurement devices that are completely powered down between measurement tasks, to prevent measurement data loss in the measurement queue, the upload task needs to run after each measurement task. This can be achieved by setting the upload task time equal to the greatest common denominator of all measurement tasks.

> In a future version of the twomes-generic-esp-firmware library, we intend to add persistent memory support for the measurement queue. This would allow you to batch the measurement values and upload measurement values in the queue to the server much less often. This can help to extend the run time of battery-powered measurement devices. It also helps make the meeasurement devices more robust against intermittent loss of internet connectivity.