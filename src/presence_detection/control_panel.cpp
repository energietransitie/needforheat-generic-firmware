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

enum class Menu {idle, read_onboarded, create_onboarded, delete_onboarded};
enum class ButtonActions {up, press, down};

namespace ControlPanel
{
    Menu menuState;
    Screen sc;

    static PresenceDetection::UseBluetooth *useBluetoothPtr = nullptr;

    void ExitControlPanel()
    {
        // Release bluetooth.
        if (useBluetoothPtr != nullptr)
            delete useBluetoothPtr;

        // Show information about what this device does on the screen.
        sc.DisplayInfoQR();

        menuState = Menu::idle;
    }

    static Timer::Timer s_timer("ExitControlPanel", ExitControlPanel, EXIT_TIMEOUT_S);
  
    void OnboardingMenuState(ButtonActions button)
    {
        static uint8_t selectedLine = 1, smartphoneIndex;

        // Reset timer, since this function ran because of a button press.
        s_timer.StartOrReset();

        switch (menuState)
        {
            case Menu::idle:
                // go out of idle the moment of interaction 
                if (button == ButtonActions::up || button == ButtonActions::down || button == ButtonActions::press) 
                {
                    menuState = Menu::read_onboarded;
                    selectedLine = 1;
                    sc.Clear();
                    sc.ReadOnboardedSmartphones(getSmartphones(), selectedLine);
                }
                break;
            case Menu::read_onboarded:
                if (button == ButtonActions::up)
                {
                    if (selectedLine) //TODO: why this guard?
                    {
                        selectedLine--;
                        //jump to last menu item if selectedLine <= 1
                        if (selectedLine < 1) {
                            selectedLine = getSmartphones().size() + 2;
                        }
                        sc.ReadOnboardedSmartphones(getSmartphones(), selectedLine);
                    }
                }
                else if (button == ButtonActions::down)
                {
                    selectedLine++;
                    //jump to first menu item if selectedLine >= last menu item
                    if (selectedLine > (getSmartphones().size() + 2)) {
                        selectedLine = 1;
                    }
                    sc.ReadOnboardedSmartphones(getSmartphones(), selectedLine);
                }
                if (button == ButtonActions::press && selectedLine == 1)
                {   
                    sc.Clear();
                    sc.CreateOnboardedSmartphone();
                    // Make device discoverable as Bluetooth Classic-only device with A2DP profile
                    PresenceDetection::InitializeOptions options{};
                    options.EnableA2DPSink = true;
                    options.EnableDiscoverable = true;
                    useBluetoothPtr = new PresenceDetection::UseBluetooth(options);

                    menuState = Menu::create_onboarded;
                    break;
                }
                else if (button == ButtonActions::press)
                {
                    if (selectedLine == (getSmartphones().size() + 2)) {
                        ExitControlPanel();
                    } else {
                        menuState = Menu::delete_onboarded;
                        sc.Clear();
                        smartphoneIndex = selectedLine - 2;// phone
                        selectedLine = 3; 
                        sc.DeleteOnboardedSmartphone(getSmartphones(), selectedLine, smartphoneIndex);
                    }
                    break;
                }
                break;
            case Menu::create_onboarded:
                if(button == ButtonActions::press)
                {
                    // Release bluetooth.
                    if (useBluetoothPtr != nullptr)
                        delete useBluetoothPtr;

                    menuState = Menu::read_onboarded;
                    sc.Clear();
                    selectedLine = 1;
                    sc.ReadOnboardedSmartphones(getSmartphones(), selectedLine);
                    break;
                }
            case Menu::delete_onboarded:
                if (button == ButtonActions::up)
                {
                    if(selectedLine) //TODO: why this guard?
                    {
                        selectedLine--;
                        //jump to last menu item if selectedLine <= 1
                        if (selectedLine <= 1) {
                            selectedLine = 4;
                        }
                        sc.DeleteOnboardedSmartphone(getSmartphones(), selectedLine, smartphoneIndex);
                        break;
                    }
                }
                else if (button == ButtonActions::down)
                {
                    selectedLine++;
                    //jump to first menu item if selectedLine >= last menu item
                    if (selectedLine > 4) {
                        selectedLine = 2;
                    }
                    sc.DeleteOnboardedSmartphone(getSmartphones(), selectedLine, smartphoneIndex);
                    break;
                }
                if((button == ButtonActions::press && selectedLine == 3) || selectedLine == 4)
                {   
                    menuState = Menu::read_onboarded;
                    sc.Clear();
                    selectedLine = 1;
                    sc.ReadOnboardedSmartphones(getSmartphones(), selectedLine);
                    break;
                }
                else if(button == ButtonActions::press && selectedLine == 2)
                {
                    MACAddres::deleteOnboardedSmartphoneFromNVS(smartphoneIndex);
                    menuState = Menu::read_onboarded;
                    sc.Clear();
                    selectedLine = 1;
                    sc.ReadOnboardedSmartphones(getSmartphones(), selectedLine);
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
