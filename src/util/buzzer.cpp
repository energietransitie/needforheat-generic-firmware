#include <util/buzzer.hpp>

#include <driver/gpio.h>
#include <driver/ledc.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <util/error.hpp>
#include <util/delay.hpp>

namespace Buzzer
{
    namespace
    {
        void BuzzerOn()
        {
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0x3ff);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        }

        void BuzzerOff()
        {
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        }
    } // namespace

    void Configure()
    {
        // Configure GPIO for buzzer.
        gpio_config_t gpioConfigBuzzer = {};
        gpioConfigBuzzer.intr_type = GPIO_INTR_DISABLE;
        gpioConfigBuzzer.mode = GPIO_MODE_OUTPUT;
        gpioConfigBuzzer.pin_bit_mask = GPIO_SEL_2;
        gpioConfigBuzzer.pull_down_en = GPIO_PULLDOWN_ENABLE;
        auto err = gpio_config(&gpioConfigBuzzer);
        Error::CheckAppendName(err, "Main", "An error occured when configuring GPIO for buzzer.");

        // Configure LEDC (PWM) timer for buzzer.
        ledc_timer_config_t timerConfig{};
        timerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
        timerConfig.timer_num = LEDC_TIMER_0;
        timerConfig.duty_resolution = LEDC_TIMER_13_BIT;
        timerConfig.freq_hz = 1000;
        timerConfig.clk_cfg = LEDC_AUTO_CLK;
        err = ledc_timer_config(&timerConfig);
        Error::CheckAppendName(err, "Main", "An error occured when configuring LEDC timer for buzzer.");

        ledc_channel_config_t channelConfig{};
        channelConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
        channelConfig.channel = LEDC_CHANNEL_0;
        channelConfig.timer_sel = LEDC_TIMER_0;
        channelConfig.intr_type = LEDC_INTR_DISABLE;
        channelConfig.gpio_num = GPIO_NUM_2;
        channelConfig.duty = 0;
        channelConfig.hpoint = 0x1fff;
        err = ledc_channel_config(&channelConfig);
        Error::CheckAppendName(err, "Main", "An error occured when configuring LEDC channel for buzzer.");
    }

    void Buzz(int milliseconds)
    {
        BuzzerOn();

        vTaskDelay(Delay::MilliSeconds(milliseconds));

        BuzzerOff();        
    }

    void Buzz(int milliseconds, int count)
    {
        for (int i = 0; i < count; i++)
        {
            Buzz(milliseconds);
            vTaskDelay(Delay::MilliSeconds(milliseconds));
        }
    }
} // namespace Buzzer
