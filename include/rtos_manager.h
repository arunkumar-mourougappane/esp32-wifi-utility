#pragma once

#include <Arduino.h>
#include "config.h"

#ifdef USE_RTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ==========================================
// RTOS MANAGER
// ==========================================

/**
 * @file rtos_manager.h
 * @brief Core RTOS Infrastructure Manager
 * 
 * Provides centralized initialization, monitoring, and statistics
 * for the FreeRTOS-based task and queue architecture.
 * 
 * @version 4.1.0
 * @date 2025-10-17
 */

// ==========================================
// RTOS CONFIGURATION
// ==========================================

/**
 * @brief Enable/disable RTOS statistics collection
 * Set to 1 to enable runtime statistics monitoring
 */
#define RTOS_ENABLE_STATISTICS 1

/**
 * @brief Enable/disable deadlock detection
 * Set to 1 to enable mutex deadlock detection
 */
#define RTOS_ENABLE_DEADLOCK_DETECTION 1

/**
 * @brief Statistics update interval (milliseconds)
 */
#define RTOS_STATS_UPDATE_INTERVAL_MS 5000

/**
 * @brief Maximum time to wait for mutex (milliseconds)
 */
#define RTOS_MUTEX_TIMEOUT_MS 1000

// ==========================================
// RTOS STATE
// ==========================================

/**
 * @brief RTOS initialization state
 */
enum class RTOSState {
    UNINITIALIZED,    ///< RTOS not yet initialized
    INITIALIZING,     ///< RTOS initialization in progress
    RUNNING,          ///< RTOS running normally
    ERROR,            ///< RTOS encountered error
    SHUTTING_DOWN     ///< RTOS shutting down
};

/**
 * @brief RTOS statistics structure
 */
struct RTOSStatistics {
    uint32_t totalHeapSize;           ///< Total heap size in bytes
    uint32_t freeHeapSize;            ///< Free heap size in bytes
    uint32_t minFreeHeapSize;         ///< Minimum free heap ever seen
    uint32_t taskCount;               ///< Number of active tasks
    uint32_t queueCount;              ///< Number of created queues
    uint32_t mutexCount;              ///< Number of created mutexes
    uint32_t semaphoreCount;          ///< Number of created semaphores
    unsigned long uptimeMs;           ///< System uptime in milliseconds
    unsigned long lastUpdateMs;       ///< Last statistics update time
};

// ==========================================
// GLOBAL STATE
// ==========================================

extern RTOSState rtosState;
extern RTOSStatistics rtosStats;

// ==========================================
// CORE FUNCTIONS
// ==========================================

/**
 * @brief Initialize RTOS infrastructure
 * 
 * Initializes all RTOS components including queues, mutexes,
 * semaphores, event groups, and base tasks.
 * 
 * @return true if initialization successful, false otherwise
 */
bool initializeRTOS();

/**
 * @brief Shutdown RTOS infrastructure
 * 
 * Cleanly shuts down all tasks, deletes queues and synchronization
 * primitives, and frees all allocated resources.
 * 
 * @return true if shutdown successful, false otherwise
 */
bool shutdownRTOS();

/**
 * @brief Get current RTOS state
 * 
 * @return Current RTOSState
 */
RTOSState getRTOSState();

/**
 * @brief Check if RTOS is initialized and running
 * 
 * @return true if RTOS is running, false otherwise
 */
bool isRTOSRunning();

// ==========================================
// STATISTICS & MONITORING
// ==========================================

/**
 * @brief Update RTOS statistics
 * 
 * Collects current statistics from FreeRTOS and updates
 * the global statistics structure.
 */
void updateRTOSStatistics();

/**
 * @brief Get RTOS statistics
 * 
 * @return Copy of current RTOSStatistics structure
 */
RTOSStatistics getRTOSStatistics();

/**
 * @brief Print RTOS statistics to serial
 * 
 * Formats and prints current RTOS statistics including
 * memory usage, task count, and uptime.
 */
void printRTOSStatistics();

/**
 * @brief Print all task information
 * 
 * Prints detailed information about all FreeRTOS tasks including
 * name, priority, stack usage, and state.
 */
void printTaskList();

/**
 * @brief Get task count
 * 
 * @return Number of active tasks
 */
uint32_t getTaskCount();

/**
 * @brief Get free heap size
 * 
 * @return Free heap size in bytes
 */
uint32_t getFreeHeap();

/**
 * @brief Get minimum free heap size ever seen
 * 
 * @return Minimum free heap in bytes
 */
uint32_t getMinFreeHeap();

// ==========================================
// DEADLOCK DETECTION
// ==========================================

#if RTOS_ENABLE_DEADLOCK_DETECTION

/**
 * @brief Check for potential deadlocks
 * 
 * Analyzes mutex wait times and task states to detect
 * potential deadlock conditions.
 * 
 * @return true if potential deadlock detected, false otherwise
 */
bool checkForDeadlocks();

/**
 * @brief Report deadlock information
 * 
 * Prints detailed information about detected deadlocks
 * to help with debugging.
 */
void reportDeadlockInfo();

#endif // RTOS_ENABLE_DEADLOCK_DETECTION

// ==========================================
// ERROR HANDLING
// ==========================================

/**
 * @brief RTOS error callback
 * 
 * Called when RTOS encounters a fatal error. This function
 * will log the error and attempt recovery or shutdown.
 * 
 * @param errorCode Error code identifier
 * @param errorMessage Human-readable error message
 */
void rtosErrorHandler(int errorCode, const char* errorMessage);

/**
 * @brief Check RTOS health
 * 
 * Performs health checks on RTOS subsystems including
 * memory, tasks, and queues.
 * 
 * @return true if RTOS is healthy, false if issues detected
 */
bool checkRTOSHealth();

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

/**
 * @brief Get current task handle
 * 
 * @return Handle to currently executing task, or NULL if not in a task
 */
TaskHandle_t getCurrentTask();

/**
 * @brief Get task by name
 * 
 * @param taskName Name of task to find
 * @return Task handle, or NULL if not found
 */
TaskHandle_t getTaskByName(const char* taskName);

/**
 * @brief Delay with task yield
 * 
 * Delays for specified milliseconds while yielding to other tasks.
 * Preferred over delay() in RTOS environment.
 * 
 * @param ms Delay time in milliseconds
 */
void rtosDelay(uint32_t ms);

/**
 * @brief Critical section enter
 * 
 * Disables interrupts for critical section protection.
 * Must be paired with rtosExitCritical().
 * 
 * @warning Keep critical sections as short as possible!
 */
void rtosEnterCritical();

/**
 * @brief Critical section exit
 * 
 * Re-enables interrupts after critical section.
 */
void rtosExitCritical();

// ==========================================
// VERSION INFORMATION
// ==========================================

/**
 * @brief Get RTOS version string
 * 
 * @return Version string (e.g., "FreeRTOS V10.4.3")
 */
const char* getRTOSVersion();

/**
 * @brief Print RTOS version and configuration
 * 
 * Prints FreeRTOS version and compile-time configuration
 * to serial console.
 */
void printRTOSInfo();

#endif // USE_RTOS
