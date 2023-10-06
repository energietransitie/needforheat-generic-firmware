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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <presence_detection.hpp>
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
#include <util/timer.hpp>
#include <driver/gpio.h>


#define LEFT_INPUT_PIN 37
#define PRESS_INPUT_PIN 38
#define RIGHT_INPUT_PIN 39

constexpr const char *NVS_NAMESPACE = "twomes_storage";
constexpr const char *TAG = "MAC Address";

constexpr const uint64_t EXIT_TIMEOUT_S = Timer::Timeout::MINUTE * 2;
constexpr const char *ONBOARDING_PAIR_NAME = "NeedForHeat_OK"; // change also in presence_detection.cpp

// mutex to protect access to the shared resource useBluetoothPtr 
// static portMUX_TYPE useBluetoothPtrMutex = portMUX_INITIALIZER_UNLOCKED;

namespace ControlPanel
{
    Screen sc;
    Menu menuState;

    static PresenceDetection::UseBluetooth *useBluetoothPtr = nullptr;

    void ReadOnboardedSmartphones(std::vector<std::string> smartphoneList, uint8_t position)
    {		
        // Create a list of text to display
        std::vector<std::string> onboardedLines = {
            "SMARTPHONES:", 
            "+ toevoegen"
        };

        // Extend the OnboardedLines vector with the splitted strings
        onboardedLines.insert(onboardedLines.end(), smartphoneList.begin(), smartphoneList.end());

        // Add "terug" to the end of the OnboardedLines vector
        onboardedLines.push_back("terug");
        sc.DrawMenu(onboardedLines, position);
    }

    void CreateOnboardedSmartphone()
    {
        // Create a list of text to display
        std::vector<std::string> createOnboardedSmartphoneScreenLines = {
            "TOEVOEGEN?", 
            "Ga op je mobiel",
            "naar Bluetooth",
            "en koppel met:",
        };

        // Add Bluetooth device name to the end of the OnboardedLines vector
        createOnboardedSmartphoneScreenLines.push_back(ONBOARDING_PAIR_NAME);

        // Add "terug" to the end of the OnboardedLines vector

        createOnboardedSmartphoneScreenLines.push_back("terug");
        sc.DrawMenu(createOnboardedSmartphoneScreenLines, 5);
    }


    void DeleteOnboardedSmartphone(std::vector<std::string> smartphoneList, uint8_t highlightedLine, uint8_t phoneID)
    {		
        std::vector<std::string> infoScreenLines = {
            smartphoneList[phoneID].c_str(),
            "VERWIJDEREN?", 
            "Ja",
            "Nee",
            "terug"
        };
        sc.DrawMenu(infoScreenLines, highlightedLine);
    }


    void ExitControlPanel()
    {
        // Release bluetooth.
        // portENTER_CRITICAL(&useBluetoothPtrMutex);
        if (useBluetoothPtr != nullptr)
            delete useBluetoothPtr;
        // portEXIT_CRITICAL(&useBluetoothPtrMutex);

        // Show information about what this device does on the screen.
        sc.DisplayInfoQR();

        menuState = Menu::idle;
    }

    static Timer::Timer s_timer("ExitControlPanel", ExitControlPanel, EXIT_TIMEOUT_S);
  
    void OnboardingMenuState(ButtonActions button)
    {
        static uint8_t highlightedLine = 1, smartphoneIndex;

        // Reset timer, since this function ran because of a button press.
        s_timer.StartOrReset();

        switch (menuState)
        {
            case Menu::idle:
                // go out of idle the moment of interaction 
                if (button == ButtonActions::up || button == ButtonActions::down || button == ButtonActions::press) 
                {
                    menuState = Menu::read_onboarded;
                    highlightedLine = 1;
                    ReadOnboardedSmartphones(getSmartphones(), highlightedLine);
                }
                break;
            case Menu::read_onboarded:
                if (button == ButtonActions::up)
                {
                    if (highlightedLine) //TODO: why this guard?
                    {
                        highlightedLine--;
                        //jump to last menu item if highlightedLine <= 1
                        if (highlightedLine < 1) {
                            highlightedLine = getSmartphones().size() + 2;
                        }
                        ReadOnboardedSmartphones(getSmartphones(), highlightedLine);
                    }
                }
                else if (button == ButtonActions::down)
                {
                    highlightedLine++;
                    //jump to first menu item if highlightedLine >= last menu item
                    if (highlightedLine > (getSmartphones().size() + 2)) {
                        highlightedLine = 1;
                    }
                    ReadOnboardedSmartphones(getSmartphones(), highlightedLine);
                }
                if (button == ButtonActions::press && highlightedLine == 1)
                {   
                    CreateOnboardedSmartphone();
                    // Make device discoverable as Bluetooth Classic-only device with A2DP profile
                    PresenceDetection::InitializeOptions options{};
                    options.EnableA2DPSink = true;
                    options.EnableDiscoverable = true;

                    // portENTER_CRITICAL(&useBluetoothPtrMutex);
                    useBluetoothPtr = new PresenceDetection::UseBluetooth(options);
                    // portEXIT_CRITICAL(&useBluetoothPtrMutex);

                    menuState = Menu::create_onboarded;
                    break;
                }
                else if (button == ButtonActions::press)
                {
                    if (highlightedLine == (getSmartphones().size() + 2)) {
                        ExitControlPanel();
                    } else {
                        menuState = Menu::delete_onboarded;
                        smartphoneIndex = highlightedLine - 2;// phone
                        highlightedLine = 3; 
                        DeleteOnboardedSmartphone(getSmartphones(), highlightedLine, smartphoneIndex);
                    }
                    break;
                }
                break;
            case Menu::create_onboarded:
                if(button == ButtonActions::press)
                {
                    // Release bluetooth.
                    // portENTER_CRITICAL(&useBluetoothPtrMutex);
                    if (useBluetoothPtr != nullptr)
                        delete useBluetoothPtr;
                    // portEXIT_CRITICAL(&useBluetoothPtrMutex);

                    menuState = Menu::read_onboarded;
                    highlightedLine = 1;
                    ReadOnboardedSmartphones(getSmartphones(), highlightedLine);
                    break;
                }
            case Menu::delete_onboarded:
                if (button == ButtonActions::up)
                {
                    if(highlightedLine) //TODO: why this guard?
                    {
                        highlightedLine--;
                        //jump to last menu item if highlightedLine <= 1
                        if (highlightedLine <= 1) {
                            highlightedLine = 4;
                        }
                        DeleteOnboardedSmartphone(getSmartphones(), highlightedLine, smartphoneIndex);
                        break;
                    }
                }
                else if (button == ButtonActions::down)
                {
                    highlightedLine++;
                    //jump to first menu item if highlightedLine >= last menu item
                    if (highlightedLine > 4) {
                        highlightedLine = 2;
                    }
                    DeleteOnboardedSmartphone(getSmartphones(), highlightedLine, smartphoneIndex);
                    break;
                }
                if((button == ButtonActions::press && highlightedLine == 3) || highlightedLine == 4)
                {   
                    menuState = Menu::read_onboarded;
                    highlightedLine = 1;
                    ReadOnboardedSmartphones(getSmartphones(), highlightedLine);
                    break;
                }
                else if(button == ButtonActions::press && highlightedLine == 2)
                {
                    MACAddres::deleteOnboardedSmartphoneFromNVS(smartphoneIndex);
                    menuState = Menu::read_onboarded;
                    highlightedLine = 1;
                    ReadOnboardedSmartphones(getSmartphones(), highlightedLine);
                    break;
                }
        }
    }

    std::vector<std::string> getSmartphones()
    {   
        bool empty = false;
        char delimiter = ';'; 
        std::string smartphones;
        auto err = NVS::Get(NVS_NAMESPACE, NVS_ONBOARDED_BT_NAMES_KEY, smartphones);
    	std::vector<std::string> smartphoneList = Strings::Split(smartphones, delimiter);

        return smartphoneList;
    }

    void up()
    {
        OnboardingMenuState(ButtonActions::up);
    }

    void press()
    {
        OnboardingMenuState(ButtonActions::press);
    }

    void down()
    {
        OnboardingMenuState(ButtonActions::down);
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
        Error::CheckAppendName(err, "ControlPanel", "An error occured when configuring GPIO for rocker button 'up'");

        gpio_config_t pressButton = {};
        pressButton.intr_type = GPIO_INTR_NEGEDGE;
        pressButton.mode = GPIO_MODE_INPUT;
        pressButton.pin_bit_mask = GPIO_SEL_38;
        pressButton.pull_down_en = GPIO_PULLDOWN_DISABLE;
        pressButton.pull_up_en = GPIO_PULLUP_ENABLE;
        err = gpio_config(&pressButton);
        Error::CheckAppendName(err, "ControlPanel", "An error occured when configuring GPIO for rocker button 'press'");

        gpio_config_t rightButton = {};
        rightButton.intr_type = GPIO_INTR_NEGEDGE;
        rightButton.mode = GPIO_MODE_INPUT;
        rightButton.pin_bit_mask = GPIO_SEL_39;
        rightButton.pull_down_en = GPIO_PULLDOWN_DISABLE;
        rightButton.pull_up_en = GPIO_PULLUP_ENABLE;
        err = gpio_config(&rightButton);
        Error::CheckAppendName(err, "ControlPanel", "An error occured when configuring GPIO for rocker button 'down'");

        
        Buttons::ButtonPressHandler::AddButton(GPIO_NUM_38, "Press", 0, press, nullptr);
        Buttons::ButtonPressHandler::AddButton(GPIO_NUM_39, "Down", 0, down, nullptr);
        Buttons::ButtonPressHandler::AddButton(GPIO_NUM_37, "Up", 0, up, nullptr);

        menuState = Menu::idle;

    }    
    
}
