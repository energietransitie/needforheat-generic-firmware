#include "platform_m5stack_coreink.hpp"

#include <ctime>

#include <esp_log.h>
#include <esp_sleep.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_wifi.h>
#include <driver/uart.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <util/delay.hpp>
#include <specific_m5coreink/powerpin.h>
#include <specific_m5coreink/rtc.h>
#include <power_manager.hpp>

constexpr const char *TAG = "Scheduler";

constexpr double BOOT_ENERGY_mJ = 2.54e2;
constexpr double BOOT_DURATION_s = 1.523;
constexpr double SETUP_ENERGY_mJ = 9.91e2;
constexpr double SETUP_DURATION_s = 4.437;
constexpr double BOOT_SETUP_ENERGY_mJ = BOOT_ENERGY_mJ + SETUP_ENERGY_mJ;
constexpr double BOOT_SETUP_DURATION_s = BOOT_DURATION_s + SETUP_DURATION_s;
constexpr double POWER_OFF_mW = 2.33e-3;
constexpr double LIGHT_SLEEP_mW = 1.15e1;
constexpr double POWER_OFF_THRESHOLD_s = (BOOT_SETUP_ENERGY_mJ - POWER_OFF_mW * BOOT_SETUP_DURATION_s) / (LIGHT_SLEEP_mW - POWER_OFF_mW);

// TODO: how to calculate this value?
// And is POWER_OFF_THRESHOLD_s correct?
constexpr double LIGHT_SLEEP_THRESHOLD_s = 10;

constexpr uint64_t FACTOR_s_TO_uS = 1000000;

namespace M5StackCoreInk
{
    void WaitUntilNextTaskTime(time_t nextTaskTime_s)
    {
        // Calculate remaining time until nextTaskTime_s.
        time_t inactiveTime_s = nextTaskTime_s - time(nullptr);

        if (inactiveTime_s <= 0)
        {
            ESP_LOGD(TAG, "We already passed the nextTaskTime_s. Continueing immediately.");

            // We already passed the next task time. Continue immediately.
            return;
        }
        else if (inactiveTime_s < LIGHT_SLEEP_THRESHOLD_s)
        {
            ESP_LOGD(TAG, "Delaying for %li seconds", inactiveTime_s);

            // Delay until the next task time.
            auto lastWakeTime = xTaskGetTickCount();
            vTaskDelayUntil(&lastWakeTime, Delay::Seconds(inactiveTime_s));
            return;
        }
        else if (/*inactiveTime_s >= LIGHT_SLEEP_THRESHOLD_s && inactiveTime_s < POWER_OFF_THRESHOLD_s*/ false)
        {
            // TODO: Make light sleep work. At this time, the following crash happens:
            // abort() was called at PC 0x4008331e on core 1
            // Backtrace:0x400906bb:0x3ffdc1d0 0x40090d59:0x3ffdc1f0 0x400979d2:0x3ffdc210 0x4008331e:0x3ffdc280 0x40083441:0x3ffdc2b0 0x401d4b66:0x3ffdc2d0 0x401d7bc5:0x3ffdc5e0 0x401e3f49:0x3ffdc610 0x40094aa5:0x3ffdc640 0x400dba69:0x3ffdc690 0x400dbd50:0x3ffdc6d0 0x400d4c2f:0x3ffdc710 0x400d4794:0x3ffdc740 0x400d4867:0x3ffdc760 0x40090d61:0x3ffdc790
            //   #0  0x400906bb:0x3ffdc1d0 in panic_abort at /home/vscode/.platformio/packages/framework-espidf/components/esp_system/panic.c:330
            //   #1  0x40090d59:0x3ffdc1f0 in esp_system_abort at /home/vscode/.platformio/packages/framework-espidf/components/esp_system/system_api.c:106
            //   #2  0x400979d2:0x3ffdc210 in abort at /home/vscode/.platformio/packages/framework-espidf/components/newlib/abort.c:46
            //   #3  0x4008331e:0x3ffdc280 in lock_acquire_generic at /home/vscode/.platformio/packages/framework-espidf/components/newlib/locks.c:142
            //   #4  0x40083441:0x3ffdc2b0 in _lock_acquire_recursive at /home/vscode/.platformio/packages/framework-espidf/components/newlib/locks.c:170
            //   #5  0x401d4b66:0x3ffdc2d0 in _vfprintf_r at /builds/idf/crosstool-NG/.build/xtensa-esp32-elf/src/newlib/newlib/libc/stdio/vfprintf.c:853 (discriminator 2)
            //   #6  0x401d7bc5:0x3ffdc5e0 in vprintf at /builds/idf/crosstool-NG/.build/xtensa-esp32-elf/src/newlib/newlib/libc/stdio/vprintf.c:34 (discriminator 5)
            //   #7  0x401e3f49:0x3ffdc610 in esp_log_writev at /home/vscode/.platformio/packages/framework-espidf/components/log/log.c:189
            //   #8  0x40094aa5:0x3ffdc640 in esp_log_write at /home/vscode/.platformio/packages/framework-espidf/components/log/log.c:199
            //   #9  0x400dba69:0x3ffdc690 in get_power_down_flags at /home/vscode/.platformio/packages/framework-espidf/components/esp32/sleep_modes.c:677
            //   #10 0x400dbd50:0x3ffdc6d0 in esp_light_sleep_start at /home/vscode/.platformio/packages/framework-espidf/components/esp32/sleep_modes.c:294
            //   #11 0x400d4c2f:0x3ffdc710 in M5StackCoreInk::WaitUntilNextTaskTime(long) at lib/scheduler/platform_m5stack_coreink.cpp:73 (discriminator 5)
            //   #12 0x400d4794:0x3ffdc740 in Scheduler::(anonymous namespace)::WaitUntilNextTaskTime() at lib/scheduler/scheduler.cpp:121
            //   #13 0x400d4867:0x3ffdc760 in Scheduler::(anonymous namespace)::SchedulerTask(void*) at lib/scheduler/scheduler.cpp:181
            //   #14 0x40090d61:0x3ffdc790 in vPortTaskWrapper at /home/vscode/.platformio/packages/framework-espidf/components/freertos/xtensa/port.c:143

            // Use light sleep.
            ESP_LOGD(TAG, "Entering light sleep for %li seconds", inactiveTime_s);
            uint64_t inactiveTime_us = uint64_t(inactiveTime_s) * FACTOR_s_TO_uS;

            esp_sleep_enable_timer_wakeup(inactiveTime_us);

            // Disable Bluetooth and Wi-Fi.
            esp_bluedroid_disable();
            esp_bt_controller_disable();
            esp_wifi_stop();

            esp_light_sleep_start();
            return;
        }
        else
        {
            // Turn off the ESP32 and wake it up using the extenal RTC.
            ESP_LOGD(TAG, "Powering off for %li seconds", inactiveTime_s);
            rtc_set_alarm(nextTaskTime_s);
            vTaskDelay(Delay::MilliSeconds(250));

            PowerManager::GetInstance().PowerOff();

            ESP_LOGD(TAG, "Unable to shutdown because USB power is connected. Delaying instead.");

            // Delay until the next task time
            // in case the device did not power off.
            auto lastWakeTime = xTaskGetTickCount();
            vTaskDelayUntil(&lastWakeTime, Delay::Seconds(inactiveTime_s));
        }
    }
} // namespace M5StackCoreInk
