#pragma once

namespace GenericTasks
{
    /**
     * Task that uploads everything on the upload queue.
     */
    void UploadTask(void *taskInfo);

    /**
     * Task to create a heartbeat and put in on the upload queue.
     */
    void HeartbeatTask(void *taskInfo);

    /**
     * Task to sync the current time using NTP.
     */
    void TimeSyncTask(void *taskInfo);

    /**
     * Start the generic tasks by adding them to the scheduler.
     */
    void AddTasksToScheduler();
} // namespace GenericTasks
