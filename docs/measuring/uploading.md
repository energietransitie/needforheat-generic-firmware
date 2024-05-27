# Uploading

All queued measurments will be uploaded to a [NeedForHeat server](https://github.com/energietransitie/needforheat-server-configuration) via the [NeedForHeat API](https://github.com/energietransitie/needforheat-server-api) when the [upload task](running-tasks.md#default-tasks) runs.

In a single upload, one or more measurements for one or more properties can be uploaded. The payload of such an upload is a JSON-formatted string with the following structure:

```json5 title="Example: JSON payload of a single heartbeat measurement upload"
{
  "upload_time": "1622237550",
  "property_measurements": [
    {
      "property_name": "heartbeat__0",
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

<i>Currently, on battery-powerd measurement measurement devices that are completely powered down between measurement tasks, to prevent measurement data loss in the measurement queue, the upload task needs to run after each measurement task. This can be achieved by setting the upload task time equal to the greatest common denominator of all measurement tasks.

> In a future version of the needforheat-generic-firmware library, we intend to add persistent memory support for the measurement queue. This would allow you to batch the measurement values and upload measurement values in the queue to the server much less often. This can help to extend the run time of battery-powered measurement devices. It also helps make the measurement devices more robust against intermittent loss of internet connectivity.