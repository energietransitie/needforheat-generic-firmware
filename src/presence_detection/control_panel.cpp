#include <control_panel.hpp>

#include <cstdlib>
#include <vector>
#include <set>
#include <regex>
#include <string>

#include <esp_log.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <scheduler.hpp>
#include <measurements.hpp>
#include <secure_upload.hpp>
#include <util/error.hpp>
#include <util/delay.hpp>
#include <util/format.hpp>
#include <util/nvs.hpp>
#include <util/strings.hpp>
#include <util/buttons.hpp>
#include <util/screen.hpp>
#include <driver/gpio.h>


#define LEFT_INPUT_PIN 37
#define PRESS_INPUT_PIN 38
#define RIGHT_INPUT_PIN 39

enum class Event {idle, select, remove};
enum class ButtonActions {left, press, right};


namespace ControlPanel
{
    
    Event state;
    Screen sc;
  
    void Panelstate(ButtonActions button)
    {

        switch (state)
        {
        case Event::idle:
                if (button == ButtonActions::press)
                {
                    ESP_LOGI("State", "button pressed");
                    state = Event::select;
                    sc.Clear();
                }
                
            break;
        case Event::select:
                if (button == ButtonActions::left)
                {
                    ESP_LOGI("State", "left button pressed");
                }
                else if (button == ButtonActions::right)
                {
                    ESP_LOGI("State", "right button pressed");
                }
                
            break;
        case Event::remove:
            /* code */
            break;
        
        default:
            break;
        }


    }

    void left()
    {
        Panelstate(ButtonActions::left);
    }

    void press()
    {
        Panelstate(ButtonActions::press);
    }

    void right()
    {
        Panelstate(ButtonActions::right);
    }

    

    void initialzeButtons()
    {
        // Configure GPIO for calibration button.
        gpio_config_t leftButton = {};
        leftButton.intr_type = GPIO_INTR_NEGEDGE;
        leftButton.mode = GPIO_MODE_INPUT;
        leftButton.pin_bit_mask = GPIO_SEL_37;
        leftButton.pull_down_en = GPIO_PULLDOWN_DISABLE;
        leftButton.pull_up_en = GPIO_PULLUP_ENABLE;
        auto err = gpio_config(&leftButton);
        Error::CheckAppendName(err, "Main", "An error occured when configuring GPIO for calibration button.");

        gpio_config_t pressButton = {};
        pressButton.intr_type = GPIO_INTR_NEGEDGE;
        pressButton.mode = GPIO_MODE_INPUT;
        pressButton.pin_bit_mask = GPIO_SEL_38;
        pressButton.pull_down_en = GPIO_PULLDOWN_DISABLE;
        pressButton.pull_up_en = GPIO_PULLUP_ENABLE;
        err = gpio_config(&pressButton);
        Error::CheckAppendName(err, "Main", "An error occured when configuring GPIO for calibration button.");

        gpio_config_t rightButton = {};
        rightButton.intr_type = GPIO_INTR_NEGEDGE;
        rightButton.mode = GPIO_MODE_INPUT;
        rightButton.pin_bit_mask = GPIO_SEL_39;
        rightButton.pull_down_en = GPIO_PULLDOWN_DISABLE;
        rightButton.pull_up_en = GPIO_PULLUP_ENABLE;
        err = gpio_config(&rightButton);
        Error::CheckAppendName(err, "Main", "An error occured when configuring GPIO for calibration button.");

        Buttons::ButtonPressHandler::AddButton(GPIO_NUM_37, "Calibrate C02", 0, left, nullptr);
        Buttons::ButtonPressHandler::AddButton(GPIO_NUM_38, "Calibrate C02", 0, press, nullptr);
        Buttons::ButtonPressHandler::AddButton(GPIO_NUM_39, "Calibrate C02", 0, right, nullptr);

        state = Event::idle;
    }    
    
}
