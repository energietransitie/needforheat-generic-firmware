#pragma once

#include <string>

#include <freertos/FreeRTOS.h>

namespace Scheduler
{
	/**
	 * Intervals that are supported by the scheduler.
	 * The scheduler will make sure that the task
	 * will run according to this interval_s.
	 *
	 * The underlying value of this enum is a time_t in seconds.
	 */
	enum class Interval : time_t
	{
		MINUTES_1 = 60, // seconds
		MINUTES_2 = 2 * MINUTES_1,
		MINUTES_5 = 5 * MINUTES_1,
		MINUTES_10 = 10 * MINUTES_1,
		MINUTES_15 = 15 * MINUTES_1,
		MINUTES_30 = 30 * MINUTES_1,
		HOURS_1 = 60 * MINUTES_1,
		HOURS_2 = 2 * HOURS_1,
		HOURS_6 = 6 * HOURS_1,
		HOURS_12 = 12 * HOURS_1,
		HOURS_24 = 24 * HOURS_1,
		HOURS_48 = 48 * HOURS_1,
		WEEK_1 = 7 * HOURS_24,
	};

	struct Task
	{
		TaskFunction_t function;
		std::string name;
		uint32_t stackDepth;
		void *params;
		uint32_t priority;
		Interval interval_s;
		uint32_t id;

		// // Constructor that allows to move name instead
		// Task(TaskFunction_t function,
		// 	 std::string &&name,
		// 	 uint32_t stackDepth,
		// 	 void *params,
		// 	 uint32_t priority,
		// 	 Interval interval_s,
		// 	 uint32_t id)
		// 	: function(function),
		// 	  name(name),
		// 	  stackDepth(stackDepth),
		// 	  params(params),
		// 	  priority(priority),
		// 	  interval_s(interval_s),
		// 	  id(id) {}
	};

	/**
	 * Helper class with method Needed() to see if init is needed.
	 */
	class Init
	{
	public:
		/**
		 * Check if initialization needs to be done or not.
		 *
		 * @returns True if initialization is needed. False if already initialized.
		 */
		bool Needed()
		{
			if (needInit)
			{
				needInit = false;
				return true;
			}
			return false;
		}

	private:
		bool needInit = true;
	};

	/**
	 * Add a task to the scheduler.
	 *
	 * Call Scheduler::Start() to start the added tasks.
	 *
	 * @param function The task function. This is a FreeRTOS task that MUST return.
	 * @param name The name of the FreeRTOS task.
	 * @param stackDepth The size of the FreeRTOS task stack.
	 * @param params Parameters passed to the FreeRTOS task.
	 * @param priority The priority of the FreeRTOS task.
	 * @param interval_s The interval with which the task should run.
	 */
	void AddTask(TaskFunction_t function,
				 std::string &&name,
				 uint32_t stackDepth,
				 void *params,
				 uint32_t priority,
				 Interval interval_s);

	/**
	 * Get a task that was added to the scheduler.
	 * This can be used to edit a task.
	 * 
	 * DO NOT CALL THIS OR USE THE POINTER AFTER CALLING Scheduler::Start()!
	 * 
	 * @param name The task name.
	 * 
	 * @returns A pointer to the added task. nullptr if the task name was not found.
	 */
	Task *GetTask(const std::string &name);

	/**
	 * Start the scheduler.
	 *
	 * This will start the tasks according to their scheduled interval.
	 */
	void Start();

	/**
	 * Immediately run all the tasks that were added to the scheduler once
	 * and wait for them to finish.
	 */
	void RunAll();

	/**
	 * Get the current task time.
	 *
	 * This is the time when this task interval was started.
	 *
	 * @returns the currentTaskTime.
	 */
	time_t GetCurrentTaskTime();

	/**
	 * Get the name of the current task.
	 *
	 * @param taskInfo Parameters that were injected by the TaskWrapper.
	 */
	std::string GetName(void *&taskInfo);

	/**
	 * Get the task parameters for the current task.
	 *
	 * @param taskInfo Parameters that were injected by the TaskWrapper.
	 */
	void *GetParams(void *&taskInfo);

	/**
	 * Get the task parameters for the current task and immediately cast it to a specific type.
	 *
	 * @param taskInfo Parameters that were injected by the TaskWrapper.
	 * @tparam T type to cast to.
	 */
	template <typename T>
	T GetParams(void *&taskInfo)
	{
		return reinterpret_cast<T>(GetParams(taskInfo));
	}

	/**
	 * Get the ID of the current task.
	 *
	 * @param taskInfo Parameters that were injected by the TaskWrapper.
	 */
	uint32_t GetID(void *&taskInfo);

	/**
	 * Block until all the other tasks that were started by the scheduler are finished.
	 *
	 * @param id ID of the calling tasks.
	 */
	void WaitForOtherTasks(uint32_t id);

	/**
	 * Block until all the other tasks that were started by the scheduler are finished.
	 *
	 * @param taskInfo taskInfo of the calling task
	 */
	void WaitForOtherTasks(void *&taskInfo);

	/**
	 * Request a restart.
	 *
	 * This will immediately restart the ESP32 when all tasks are done.
	 */
	void RequestRestart();
} // namespace Scheduler
