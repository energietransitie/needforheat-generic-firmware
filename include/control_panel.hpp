#pragma once

#include <string>


enum class ButtonActions {up, press, down, longPress};
enum class Event {idle, select, info, remove};

namespace ControlPanel
{
     /**
     * InitilizeButtons.
     * 
     * This initilises the button togher with starting the state machine
     */
    void initialzeButtons();
    /**
     * getSmartphones.
     * 
     * This collects all the participating smartphones
     */
    std::string getSmartphones();
    /**
     * Panelstate
     * 
     * this is the statemachine 
     */
    void Panelstate(ButtonActions button);
}