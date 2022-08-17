#pragma once

namespace M5CoreInkSpecific
{
    /**
     * Battery voltage task.
     * 
     * This will measure the battery voltage
     * and add the value to the secure upload queue.
     */
    void BatteryVoltageTask(void *taskInfo);
} // namespace M5CoreInkSpecific
