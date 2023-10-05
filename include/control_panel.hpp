#pragma once

#include <string>
#include <vector>

enum class ButtonActions {up, press, down};
enum class Menu {idle, read_onboarded, create_onboarded, delete_onboarded};

namespace ControlPanel
{
    extern Menu menuState;

     /**
     * InitilizeButtons.
     * 
     * This initilises the button togher with starting the state machine
     */
    void initialzeButtons();

    /**
     * OnboardingMenuState
     * 
     * This is the statemachine of the Onboarding menu 
     */
    void OnboardingMenuState(ButtonActions button);

    /**
     * getSmartphones.
     * 
     * This collects all the participating smartphones
     */
    std::vector<std::string> getSmartphones();
}