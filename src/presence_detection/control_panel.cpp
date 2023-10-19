#include <control_panel.hpp>
#include <mac_address.hpp>

#include <cstdlib>
#include <vector>
#include <set>
#include <regex>
#include <string>

#include <esp_log.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>
#include <esp_gap_bt_api.h>

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

constexpr const char *NVS_NAMESPACE = "twomes_storage";
constexpr const char *TAG = "MAC Address";


namespace ControlPanel
{
    Screen sc;
    Event state;
    
    void Panelstate(ButtonActions button)
    {
        static uint8_t selectedItem = 0, SelectedItemForRemoval;
        switch (state)
        {
        case Event::idle:
                // go out of idle the moment of interaction 
                if (button == ButtonActions::press || button == ButtonActions::up || button == ButtonActions::down)
                {
                    state = Event::select;
                    sc.Clear();
                    sc.DisplaySmartphones(getSmartphones(), selectedItem);
                }
                
            break;
        case Event::select:
                if (button == ButtonActions::up)
                {
                    if(selectedItem)
                    {
                        selectedItem--;
                    }
                }
                else if (button == ButtonActions::down)
                {
                    selectedItem++;
                }

                if (button == ButtonActions::press && selectedItem == 0)
                {   
                    sc.Clear();
                    sc.InfoScreen();
                    state = Event::info;
                    break;
                }
                else if (button == ButtonActions::press)
                {
                    state = Event::remove;
                    sc.Clear();
                    SelectedItemForRemoval = selectedItem-1;//-1 because info is also a selectedItem and the phone list should start at 0
                    selectedItem = 0; 
                    sc.RemoveSmartphone(getSmartphones(),selectedItem, SelectedItemForRemoval);
                    break;
                }

                sc.DisplaySmartphones(getSmartphones(), selectedItem);
                
            break;
        case Event::info:
                if(button == ButtonActions::press || button == ButtonActions::up || button == ButtonActions::down)
                {
                    state = Event::select;
                    sc.Clear();
                    selectedItem = 0;
                    sc.DisplaySmartphones(getSmartphones(), selectedItem);
                    break;
                }
        case Event::remove:
                if (button == ButtonActions::up)
                {
                    if(!selectedItem)
                    {
                        selectedItem--;
                    }
                }
                else if (button == ButtonActions::down)
                {
                    selectedItem++;
                }

                if((button == ButtonActions::press && selectedItem == 1) || selectedItem == 2)
                {   
                    state = Event::select;
                    sc.Clear();
                    selectedItem = 0;
                    sc.DisplaySmartphones(getSmartphones(), selectedItem);
                    break;
                }
                else if(button == ButtonActions::press && selectedItem == 0)
                {
                    MACAddres::removeMacAddress(SelectedItemForRemoval);
                    state = Event::select;
                    sc.Clear();
                    selectedItem = 0;
                    sc.DisplaySmartphones(getSmartphones(), selectedItem);
                    break;
                }
                sc.RemoveSmartphone(getSmartphones(), selectedItem, SelectedItemForRemoval);
            break;
        
        default:
            break;
        }


    }

    std::string getSmartphones()
    {   
        bool empty = false;
        
        std::string smartphoneList;
        auto err = NVS::Get(NVS_NAMESPACE, "smartphones", smartphoneList);

        return smartphoneList;
    }

    void up()
    {
        Panelstate(ButtonActions::up);
    }

    void press()
    {
        Panelstate(ButtonActions::press);
    }

    void down()
    {
        Panelstate(ButtonActions::down);
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

        
        Buttons::ButtonPressHandler::AddButton(GPIO_NUM_38, "Press", 0, press, nullptr);
        Buttons::ButtonPressHandler::AddButton(GPIO_NUM_39, "Down", 0, down, nullptr);
        Buttons::ButtonPressHandler::AddButton(GPIO_NUM_37, "up", 0, up, nullptr);

        state = Event::idle;

    }    
    
}
