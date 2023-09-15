#pragma once

#include <string>

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
}