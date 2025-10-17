#pragma once

#include <Arduino.h>
#include "config.h"

#ifdef USE_RTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ==========================================
// TASK BASE CLASS
// ==========================================

/**
 * @file task_base.h
 * @brief Base class for FreeRTOS tasks
 * 
 * Provides a C++ wrapper around FreeRTOS tasks with common
 * functionality for task lifecycle, monitoring, and control.
 * 
 * @version 4.1.0
 * @date 2025-10-17
 */

// ==========================================
// TASK CONFIGURATION
// ==========================================

/**
 * @brief Task priorities
 * 
 * Higher numbers = higher priority
 * FreeRTOS supports priorities from 0 to configMAX_PRIORITIES-1
 */
enum class TaskPriority {
    IDLE = 0,           ///< Lowest priority (idle task level)
    PRIORITY_LOW = 1,   ///< Low priority (LED, background tasks)
    PRIORITY_MEDIUM = 2,///< Medium priority (WiFi, Web Server, Analysis)
    PRIORITY_HIGH = 3,  ///< High priority (Command Interface)
    REALTIME = 4        ///< Highest priority (critical/time-sensitive)
};

/**
 * @brief Task state enum
 */
enum class TaskState {
    NOT_CREATED,        ///< Task not yet created
    CREATED,            ///< Task created but not started
    RUNNING,            ///< Task is running
    SUSPENDED,          ///< Task is suspended
    STOPPING,           ///< Task is being stopped
    STOPPED,            ///< Task has stopped
    ERROR               ///< Task encountered error
};

/**
 * @brief Task statistics structure
 */
struct TaskStatistics {
    const char* name;              ///< Task name
    TaskState state;               ///< Current task state
    TaskPriority priority;         ///< Task priority
    uint32_t stackSize;            ///< Allocated stack size in bytes
    uint32_t stackHighWaterMark;   ///< Minimum free stack ever seen
    uint32_t loopCount;            ///< Number of loop iterations
    unsigned long totalRunTimeMs;  ///< Total run time in milliseconds
    unsigned long lastRunTimeMs;   ///< Last loop execution time
    unsigned long maxLoopTimeMs;   ///< Maximum loop execution time
    unsigned long avgLoopTimeMs;   ///< Average loop execution time
};

// ==========================================
// TASK BASE CLASS
// ==========================================

/**
 * @brief Base class for FreeRTOS tasks
 * 
 * Inherit from this class to create custom tasks with built-in
 * lifecycle management, statistics, and control functions.
 * 
 * @example
 * class MyTask : public TaskBase {
 * public:
 *     MyTask() : TaskBase("MyTask", 4096, TaskPriority::MEDIUM) {}
 * 
 * protected:
 *     void setup() override {
 *         // Task initialization
 *     }
 *     
 *     void loop() override {
 *         // Main task logic
 *     }
 *     
 *     void cleanup() override {
 *         // Task cleanup
 *     }
 * };
 */
class TaskBase {
public:
    /**
     * @brief Constructor
     * 
     * @param taskName Name of the task (max 16 characters)
     * @param stackSize Stack size in bytes
     * @param priority Task priority
     * @param coreId CPU core to pin task to (0, 1, or tskNO_AFFINITY)
     */
    TaskBase(const char* taskName, 
             uint32_t stackSize, 
             TaskPriority priority,
             BaseType_t coreId = tskNO_AFFINITY);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~TaskBase();
    
    /**
     * @brief Create and start the task
     * 
     * @return true if task created successfully, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop the task
     * 
     * Signals the task to stop and waits for it to terminate.
     * 
     * @param timeoutMs Maximum time to wait for task to stop
     * @return true if task stopped successfully, false if timeout
     */
    bool stop(uint32_t timeoutMs = 5000);
    
    /**
     * @brief Suspend the task
     * 
     * @return true if suspended successfully
     */
    bool suspend();
    
    /**
     * @brief Resume the task
     * 
     * @return true if resumed successfully
     */
    bool resume();
    
    /**
     * @brief Check if task is running
     * 
     * @return true if task is running, false otherwise
     */
    bool isRunning() const;
    
    /**
     * @brief Get task state
     * 
     * @return Current task state
     */
    TaskState getState() const;
    
    /**
     * @brief Get task handle
     * 
     * @return FreeRTOS task handle, or NULL if not created
     */
    TaskHandle_t getHandle() const;
    
    /**
     * @brief Get task name
     * 
     * @return Task name string
     */
    const char* getName() const;
    
    /**
     * @brief Get task priority
     * 
     * @return Task priority
     */
    TaskPriority getPriority() const;
    
    /**
     * @brief Set task priority
     * 
     * @param newPriority New priority level
     */
    void setPriority(TaskPriority newPriority);
    
    /**
     * @brief Get task statistics
     * 
     * @return Task statistics structure
     */
    TaskStatistics getStatistics();
    
    /**
     * @brief Print task statistics
     * 
     * Prints detailed task statistics to serial console.
     */
    void printStatistics();
    
    /**
     * @brief Reset task statistics
     * 
     * Resets statistics counters to zero.
     */
    void resetStatistics();
    
protected:
    /**
     * @brief Task setup (override in derived classes)
     * 
     * Called once when task starts, before entering main loop.
     * Use for initialization that must occur in the task context.
     */
    virtual void setup() {}
    
    /**
     * @brief Task main loop (must override in derived classes)
     * 
     * Called repeatedly. Implement main task logic here.
     * Should not block for long periods - use delays or waiting functions.
     */
    virtual void loop() = 0;
    
    /**
     * @brief Task cleanup (override in derived classes)
     * 
     * Called when task is being stopped. Use for cleanup and
     * resource deallocation.
     */
    virtual void cleanup() {}
    
    /**
     * @brief Task delay
     * 
     * Delays for specified milliseconds while yielding to other tasks.
     * Preferred over delay() in task context.
     * 
     * @param ms Delay time in milliseconds
     */
    void taskDelay(uint32_t ms);
    
    /**
     * @brief Check if stop requested
     * 
     * Tasks should check this periodically and exit loop() if true.
     * 
     * @return true if stop has been requested
     */
    bool isStopRequested() const;
    
    /**
     * @brief Yield to other tasks
     * 
     * Gives other tasks a chance to run.
     */
    void yield();
    
private:
    // Static task function wrapper (required by FreeRTOS)
    static void taskFunction(void* parameter);
    
    // Task control
    TaskHandle_t taskHandle;
    const char* name;
    uint32_t stackSizeBytes;
    TaskPriority taskPriority;
    BaseType_t cpuCore;
    TaskState currentState;
    bool stopRequested;
    
    // Statistics
    TaskStatistics stats;
    unsigned long loopStartTimeMs;
    unsigned long lastStatsUpdateMs;
    
    // Update internal statistics
    void updateStatistics();
    
    // Prevent copying
    TaskBase(const TaskBase&) = delete;
    TaskBase& operator=(const TaskBase&) = delete;
};

#endif // USE_RTOS
