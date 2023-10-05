#pragma once

#include <string>
#include <vector>

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
    std::vector<std::string> getSmartphones();
}