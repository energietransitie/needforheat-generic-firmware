#pragma once

#include <esp_err.h>

namespace M5StackCoreInk
{
    /**
     * Wait for the next measurement interval for platform M5STACK_COREINK.
     */
    void WaitUntilNextTaskTime(time_t nextTaskTime_s);
} // namespace M5StackCoreInk
