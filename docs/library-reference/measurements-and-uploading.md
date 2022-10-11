# Measurements and uploading

The Twomes generic firmware library allows for easy creation of new measurements. Any property supported by the [Twomes API](https://github.com/energietransitie/twomes-backoffice-api) can be created and uploaded to it.

## Measurements

Any measurement that is uploaded to the Twomes API has the following structure:

```json5 title="Heartbeat measurement example"
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

In the Twomes generic firmware library, all measurements have a propery name with a timestamp and a value associated to it.

### Configure property value formatting

Each measurement has a value, which is formatted in a specific way. The way that a value is formatted is dependent upon what kind of property it is. You can specify a new formatter for a specific property, according to the [printf formatting rules](https://en.wikipedia.org/wiki/Printf_format_string).

```cpp title="Adding a property formatter"
Measurements::Measurement::AddFormatter("example", "%d"); // (1)!
```

1. "example" is the property name. "%d" is how the value must be formatted.

### Create a new measurement object

Before creating a measurement, a formatter for the measurement's property has to be added. See [configuring property value formatting](#configuring-property-value-formatting) on how to do that.

A measurement can be created in two ways. By default, the measurement timestamp will be set to the [current task time](scheduler.md#current-task-time). A measurement can also be created with a custom timestamp.

```cpp title="Creating a measurement with the current task time"
#include <measurements.hpp>

int exampleValue = 1;

Measurements::Measurement exampleMeasurement("example", exampleValue); // (1)!
```

1. Create a new measurement with property type "example" and the value of exampleCounter. The timestamp of this measurement will be the current task time.

```cpp title="Creating a measurement with a custom timestamp"
#include <measurements.hpp>

int exampleValue = 1;
time_t now = time(nullptr);

Measurements::Measurement exampleMeasurement("example", exampleValue, now); // (1)!
```

1. Create a new measurement with property type "example" and the value of exampleCounter. The timestamp of this measurement will be `now`.

## Secure upload queue

The Twomes generic firmware library has a "secure upload queue", to which measurements can be added. The entire queue will be uploaded to a twomes server when the [upload task](tasks.md#default-tasks) runs.

### Upload a measurement

In order to gain access tot he secure upload queue, you will first need to get an instance of the queue. The entire library has only a single queue.

A [measurement](#creating-a-new-measurement-object) can be added to the secure upload queue as shown below:

```cpp title="Getting access to the queue and adding a measurement"
#include <secure_upload.hpp>

auto secureUploadQueue = SecureUpload::Queue::GetInstance();

secureUploadQueue.AddMeasurement(exampleMeasurement);
```
