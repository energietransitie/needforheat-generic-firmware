#include <driver/gpio.h>
#include "powerpin.h"

// setup and set GPIO 12
void powerpin_set()
{
    // setup power pin
    gpio_config_t config = {
        .pin_bit_mask = GPIO_SEL_12,
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&config);

    // turn battery power on
    gpio_set_level(GPIO_NUM_12, 1);
}

// reset powerpin
void powerpin_reset()
{
    gpio_set_level(GPIO_NUM_12,0);
}