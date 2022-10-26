#include "scheduler.hpp"

#include <vector>
#include <algorithm>

#include <esp_log.h>

#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <util/delay.hpp>

#ifdef ESP32DEV
#include "platform_esp32_dev.hpp"
#endif // ESP32DEV
#if M5STACK_COREINK
#include "platform_m5stack_coreink.hpp"
#endif // M5STACK_COREINK

constexpr const char *TAG = "Scheduler";

constexpr size_t MAX_TASK_AMOUNT = 23; // 23 because there are 24 bits, but one is used for a restart request.
constexpr EventBits_t EVENT_REQUEST_RESTART = 1 << 23;

namespace Scheduler
{
	namespace
	{
		static std::vector<Task> s_tasks;

		// An event group has a maximum of 24 bits that can be set.
		// We use 1 bit for restart request (MSB),
		// so the maximum amount of tasks that can be added to the scheduler is 23.
		static EventGroupHandle_t s_tasksEventGroup = xEventGroupCreate();
		static EventBits_t s_startedTaskBits = 0;

		static time_t s_schedulerStartTime = time(nullptr);

		/**
		 * Struct that holds information needed by the TaskWrapper.
		 */
		struct TaskWrapperInfo
		{
			TaskFunction_t function;
			std::string name;
			void *params;
			uint32_t id;
		};

		/**
		 * Wrapper for the real FreeRTOS task.
		 *
		 * When the real FreeRTOS task returns, this task will signal that it is done.
		 */
		void TaskWrapper(void *pvParams)
		{
			auto task = reinterpret_cast<TaskWrapperInfo *>(pvParams);

			// Start the actual task function and pass in the params.
			task->function(pvParams);

			// Signal that the task is done.
			xEventGroupSetBits(s_tasksEventGroup, 1 << task->id);

			// Delete the TaskWrapperInfo pointer.
			delete task;

			ESP_LOGD(task->name.c_str(), "task finished.");

			// Delete this FreeRTOS task.
			vTaskDelete(nullptr);
		}

		/**
		 * Calculates the next time a task has to run.
		 *
		 * @return the next task time.
		 */
		time_t CalculateNextTaskTime()
		{
			time_t currentTaskTime_s = GetCurrentTaskTime();

			std::vector<time_t> nextTaskTimes_s;

			for (const auto &task : s_tasks)
			{
				// Calculate the next time this this task has to run.
				time_t nextTaskTime_s = (currentTaskTime_s / (time_t)task.interval_s + 1) * (time_t)task.interval_s;
				nextTaskTimes_s.push_back(nextTaskTime_s);
			}

			// Sort the vector in ascending order.
			// Element 0 will be the earliest time.
			std::sort(nextTaskTimes_s.begin(), nextTaskTimes_s.end());

			if (!nextTaskTimes_s.empty())
			{
				return nextTaskTimes_s.front();
			}

			ESP_LOGE(TAG, "The scheduler was started without any tasks. Please add tasks or do not start the scheduler.");

			// Something went wrong. Set the time to 0
			// to make sure we do not sleep.
			return 0;
		}

		/**
		 * Wait for the next time tasks have to run.
		 *
		 * This waiting can be done in three different ways,
		 * also depending on the platform:
		 *   - Continue immediately if next task time already passed.
		 *   - Stay on and delay the scheduler until the next interval.
		 *   - Put the ESP32 in deep sleep.
		 *   - Turn the ESP32 off and wake it up using the external RTC (M5STACK_COREINK only).
		 */
		void WaitUntilNextTaskTime()
		{
			// Give other tasks the possibility to run.
			vTaskDelay(Delay::Seconds(1));

			// Calculate next measurement time.
			time_t nextTaskTime_s = CalculateNextTaskTime();

#ifdef ESP32DEV
			ESP32Dev::WaitUntilNextTaskTime(nextTaskTime_s);
#endif // ESP32DEV

#ifdef M5STACK_COREINK
			M5StackCoreInk::WaitUntilNextTaskTime(nextTaskTime_s);
#endif // M5STACK_COREINK
		}

		/**
		 * This task will start all of the other tasks, wait for them to finish
		 * and then call the platform-specific code to wait until the next task time.
		 */
		void SchedulerTask(void *pvParams)
		{
			auto runImmediately = static_cast<int>(reinterpret_cast<int>(pvParams));

			while (true)
			{
				ESP_LOGD(TAG, "Starting scheduler.");

				// Reset started task bits and count.
				s_startedTaskBits = 0;
				int startedTasks = 0;

				s_schedulerStartTime = time(nullptr);

				// Loop over every task.
				for (const auto &task : s_tasks)
				{
					// Check if the bootMinute is exactly a multiple of the interval
					// or tasks need to run immediately.
					if (GetCurrentTaskTime() % static_cast<time_t>(task.interval_s) == 0 || runImmediately)
					{
						ESP_LOGD(TAG, "Starting task \"%s\".", task.name.c_str());

						// Add the task we are about to start to the started task bits.
						s_startedTaskBits |= 1 << task.id;

						// Create information that the TaskWrapper needs.
						// This will be deleted by the TaskWrapper.
						auto info = new TaskWrapperInfo({task.function, task.name, task.params, task.id});

						// Start the FreeRTOS task inside the TaskWrapper.
						// We do not need the handle, so it is nullptr.
						xTaskCreatePinnedToCore(TaskWrapper,
												task.name.c_str(),
												task.stackDepth,
												info,
												task.priority,
												nullptr,
												APP_CPU_NUM);

						startedTasks++;
					}
				}

				if (startedTasks == 0)
				{
					ESP_LOGD(TAG, "No tasks were started at this time.");
					WaitUntilNextTaskTime();
					continue;
				}

				// Wait for all the bits to be set,
				// which means all tasks are done.
				auto bits = xEventGroupWaitBits(s_tasksEventGroup, s_startedTaskBits, pdTRUE, pdTRUE, portMAX_DELAY);

				ESP_LOGD(TAG, "All tasks have finished running.");

				if (bits & EVENT_REQUEST_RESTART)
				{
					ESP_LOGI(TAG, "Restart requested. Restarting now.");
					esp_restart();
				}

				WaitUntilNextTaskTime();
			}
		}
	} // namespace

	void AddTask(TaskFunction_t function,
				 std::string &&name,
				 uint32_t stackDepth,
				 void *params,
				 uint32_t priority,
				 Interval interval_s)
	{
		static uint32_t id = 0;

		if (s_tasks.size() >= MAX_TASK_AMOUNT)
		{
			ESP_LOGE(TAG,
					 "Not adding task \"%s\" because the maximum task amount was already reached.",
					 name.c_str());
			return;
		}

		if (GetTask(name) != nullptr)
		{
			// Task is already added. We can skip this.
			ESP_LOGD(TAG, "Task with name \"%s\" was already started. Skipping now.", name.c_str());
			return;
		}

		Task task = {function, name, stackDepth, params, priority, interval_s, id};

		s_tasks.push_back(std::move(task));

		id++;
	}

	Task *GetTask(const std::string &name)
	{
		for (auto &task : s_tasks)
		{
			if (task.name == name)
				return &task;
		}

		return nullptr;
	}

	void ClearTasks()
	{
		s_tasks.clear();
	}

	void Start()
	{
		xTaskCreatePinnedToCore(SchedulerTask,
								"Scheduler task",
								4096,
								reinterpret_cast<void *>(false),
								configMAX_PRIORITIES - 1,
								nullptr,
								APP_CPU_NUM);
	}

	void RunAll()
	{
		s_schedulerStartTime = time(nullptr);

		// Reset started task bits and count.
		s_startedTaskBits = 0;
		int startedTasks = 0;

		// Loop over every task.
		for (const auto &task : s_tasks)
		{
			ESP_LOGD(TAG, "Starting task \"%s\".", task.name.c_str());

			// Add the task we are about to start to the started task bits.
			s_startedTaskBits |= 1 << task.id;

			// Create information that the TaskWrapper needs.
			// This will be deleted by the TaskWrapper.
			auto info = new TaskWrapperInfo({task.function, task.name, task.params, task.id});

			// Start the FreeRTOS task inside the TaskWrapper.
			// We do not need the handle, so it is nullptr.
			xTaskCreatePinnedToCore(TaskWrapper,
									task.name.c_str(),
									task.stackDepth,
									info,
									task.priority,
									nullptr,
									APP_CPU_NUM);

			startedTasks++;
		}

		if (startedTasks == 0)
		{
			ESP_LOGD(TAG, "No tasks were added to the scheduler.");
			return;
		}

		// Wait for all the bits to be set,
		// which means all tasks are done.
		xEventGroupWaitBits(s_tasksEventGroup, s_startedTaskBits, pdTRUE, pdTRUE, portMAX_DELAY);

		ESP_LOGD(TAG, "All tasks have finished running.");
	}

	time_t GetCurrentTaskTime()
	{
		// Round down the time to full minutes,
		// since the intervals will never be smaller than 1 minute.
		auto currentTime = localtime(&s_schedulerStartTime);

		currentTime->tm_sec = 0;

		return mktime(currentTime);
	}

	std::string GetName(void *&taskInfo)
	{
		if (taskInfo == nullptr)
			return "";

		auto info = reinterpret_cast<TaskWrapperInfo *>(taskInfo);
		return info->name;
	}

	void *GetParams(void *&taskInfo)
	{
		if (taskInfo == nullptr)
			return nullptr;

		auto info = reinterpret_cast<TaskWrapperInfo *>(taskInfo);
		return info->params;
	}

	uint32_t GetID(void *&taskInfo)
	{
		if (taskInfo == nullptr)
			return -1;

		auto info = reinterpret_cast<TaskWrapperInfo *>(taskInfo);
		return info->id;
	}

	void WaitForOtherTasks(uint32_t id)
	{
		// Remove the calling task's ID from the waitbits.
		EventBits_t waitBits = s_startedTaskBits & ~(1 << id);

		// There are no other tasks, so we can return.
		if (waitBits == 0)
			return;

		// Wait for all the other bits to be set,
		// which means all other tasks are done.
		// Do not clear the bits on exit!
		xEventGroupWaitBits(s_tasksEventGroup, waitBits, pdFALSE, pdTRUE, portMAX_DELAY);
	}

	void WaitForOtherTasks(void *&taskInfo)
	{
		if (taskInfo == nullptr)
			return;

		WaitForOtherTasks(GetID(taskInfo));
	}

	void RequestRestart()
	{
		xEventGroupSetBits(s_tasksEventGroup, EVENT_REQUEST_RESTART);
	}
} // namespace Scheduler
