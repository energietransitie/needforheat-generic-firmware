#pragma once

namespace PresenceDetection
{
    /**
     * Presence detection task.
     * 
     * This task will scan known bluetooth MAC-addresses
     * and report which ones responded.
     */
    void PresenceDetectionTask(void *taskInfo);
} // namespace PresenceDetection
