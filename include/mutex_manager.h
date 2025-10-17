#pragma once

#include <Arduino.h>
#include "config.h"

#ifdef USE_RTOS
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ==========================================
// MUTEX MANAGER
// ==========================================

/**
 * @file mutex_manager.h
 * @brief RTOS Mutex Management System
 * 
 * Provides centralized creation, management, and monitoring of all
 * FreeRTOS mutexes used for resource protection with RAII wrapper.
 * 
 * @version 4.1.0
 * @date 2025-10-17
 */

// ==========================================
// MUTEX CONFIGURATION
// ==========================================

#define MUTEX_WAIT_TIMEOUT_MS 1000
#define MUTEX_DEADLOCK_THRESHOLD_MS 5000

// ==========================================
// MUTEX HANDLES
// ==========================================

extern SemaphoreHandle_t wifiMutex;
extern SemaphoreHandle_t webServerMutex;
extern SemaphoreHandle_t scanResultsMutex;
extern SemaphoreHandle_t serialMutex;

// ==========================================
// MUTEX STATISTICS
// ==========================================

/**
 * @brief Mutex statistics structure
 */
struct MutexStatistics {
    const char* name;          ///< Mutex name
    uint32_t lockCount;        ///< Number of successful locks
    uint32_t unlockCount;      ///< Number of unlocks
    uint32_t lockFailures;     ///< Number of failed lock attempts
    uint32_t timeoutCount;     ///< Number of timeout events
    uint32_t maxWaitTimeMs;    ///< Maximum wait time in milliseconds
    uint32_t totalWaitTimeMs;  ///< Total wait time in milliseconds
    uint32_t avgWaitTimeMs;    ///< Average wait time in milliseconds
    bool currentlyLocked;      ///< Current lock status
    TaskHandle_t ownerTask;    ///< Task currently holding the mutex
    unsigned long lockTimeMs;  ///< Time when mutex was locked
};

// ==========================================
// RAII MUTEX LOCK GUARD
// ==========================================

/**
 * @brief RAII mutex lock guard
 * 
 * Automatically locks mutex on construction and unlocks on destruction.
 * Provides exception-safe mutex management.
 * 
 * @example
 * {
 *     MutexLock lock(wifiMutex, "WiFi Operation");
 *     if (lock.isLocked()) {
 *         // Protected code here
 *         // WiFi API calls
 *     }
 * } // Mutex automatically unlocked here
 */
class MutexLock {
public:
    /**
     * @brief Constructor - attempts to lock mutex
     * 
     * @param mutex Mutex handle to lock
     * @param name Operation name for debugging
     * @param timeoutMs Maximum time to wait for lock
     */
    MutexLock(SemaphoreHandle_t mutex, const char* name = nullptr, 
              uint32_t timeoutMs = MUTEX_WAIT_TIMEOUT_MS);
    
    /**
     * @brief Destructor - unlocks mutex if locked
     */
    ~MutexLock();
    
    /**
     * @brief Check if mutex was successfully locked
     * 
     * @return true if locked, false if failed or timed out
     */
    bool isLocked() const { return locked; }
    
    /**
     * @brief Get wait time for lock acquisition
     * 
     * @return Wait time in milliseconds
     */
    uint32_t getWaitTimeMs() const { return waitTimeMs; }
    
    // Prevent copying
    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;
    
private:
    SemaphoreHandle_t mutexHandle;
    const char* operationName;
    bool locked;
    uint32_t waitTimeMs;
    unsigned long lockStartMs;
};

// ==========================================
// CORE FUNCTIONS
// ==========================================

/**
 * @brief Initialize all RTOS mutexes
 * 
 * Creates all FreeRTOS mutexes used for resource protection.
 * 
 * @return true if all mutexes created successfully, false otherwise
 */
bool initializeMutexManager();

/**
 * @brief Shutdown mutex manager
 * 
 * Deletes all mutexes and frees resources.
 * 
 * @return true if shutdown successful, false otherwise
 */
bool shutdownMutexManager();

/**
 * @brief Get mutex count
 * 
 * @return Number of created mutexes
 */
uint32_t getMutexCount();

/**
 * @brief Check for mutex deadlocks
 * 
 * Analyzes mutex wait times to detect potential deadlocks.
 * 
 * @return true if potential deadlock detected, false otherwise
 */
bool checkMutexDeadlocks();

/**
 * @brief Print mutex deadlock information
 * 
 * Prints detailed information about detected deadlocks.
 */
void printMutexDeadlockInfo();

// ==========================================
// MUTEX LOCK/UNLOCK FUNCTIONS
// ==========================================

/**
 * @brief Lock mutex with timeout
 * 
 * @param mutex Mutex to lock
 * @param timeoutMs Maximum wait time in milliseconds
 * @return true if locked successfully, false if timeout
 */
bool lockMutex(SemaphoreHandle_t mutex, uint32_t timeoutMs = MUTEX_WAIT_TIMEOUT_MS);

/**
 * @brief Unlock mutex
 * 
 * @param mutex Mutex to unlock
 * @return true if unlocked successfully, false otherwise
 */
bool unlockMutex(SemaphoreHandle_t mutex);

/**
 * @brief Try to lock mutex without blocking
 * 
 * @param mutex Mutex to try locking
 * @return true if locked immediately, false if already locked
 */
bool tryLockMutex(SemaphoreHandle_t mutex);

// ==========================================
// WIFI MUTEX FUNCTIONS
// ==========================================

/**
 * @brief Lock WiFi mutex
 * 
 * Use before accessing WiFi APIs to ensure thread safety.
 * 
 * @param timeoutMs Maximum wait time in milliseconds
 * @return true if locked successfully, false if timeout
 */
bool lockWiFiMutex(uint32_t timeoutMs = MUTEX_WAIT_TIMEOUT_MS);

/**
 * @brief Unlock WiFi mutex
 * 
 * @return true if unlocked successfully
 */
bool unlockWiFiMutex();

// ==========================================
// WEB SERVER MUTEX FUNCTIONS
// ==========================================

/**
 * @brief Lock web server mutex
 * 
 * Use before accessing web server state or configuration.
 * 
 * @param timeoutMs Maximum wait time in milliseconds
 * @return true if locked successfully, false if timeout
 */
bool lockWebServerMutex(uint32_t timeoutMs = MUTEX_WAIT_TIMEOUT_MS);

/**
 * @brief Unlock web server mutex
 * 
 * @return true if unlocked successfully
 */
bool unlockWebServerMutex();

// ==========================================
// SCAN RESULTS MUTEX FUNCTIONS
// ==========================================

/**
 * @brief Lock scan results mutex
 * 
 * Use before reading or writing scan result cache.
 * 
 * @param timeoutMs Maximum wait time in milliseconds
 * @return true if locked successfully, false if timeout
 */
bool lockScanResultsMutex(uint32_t timeoutMs = MUTEX_WAIT_TIMEOUT_MS);

/**
 * @brief Unlock scan results mutex
 * 
 * @return true if unlocked successfully
 */
bool unlockScanResultsMutex();

// ==========================================
// SERIAL MUTEX FUNCTIONS
// ==========================================

/**
 * @brief Lock serial mutex
 * 
 * Use before writing to Serial to prevent interleaved output.
 * 
 * @param timeoutMs Maximum wait time in milliseconds
 * @return true if locked successfully, false if timeout
 */
bool lockSerialMutex(uint32_t timeoutMs = MUTEX_WAIT_TIMEOUT_MS);

/**
 * @brief Unlock serial mutex
 * 
 * @return true if unlocked successfully
 */
bool unlockSerialMutex();

// ==========================================
// STATISTICS & MONITORING
// ==========================================

/**
 * @brief Get mutex statistics
 * 
 * @param mutex Mutex handle to get statistics for
 * @return Mutex statistics structure
 */
MutexStatistics getMutexStatistics(SemaphoreHandle_t mutex);

/**
 * @brief Print all mutex statistics
 * 
 * Prints statistics for all mutexes to serial console.
 */
void printMutexStatistics();

/**
 * @brief Reset mutex statistics
 * 
 * Resets all statistics counters to zero.
 */
void resetMutexStatistics();

/**
 * @brief Check if mutex is locked
 * 
 * @param mutex Mutex to check
 * @return true if mutex is currently locked
 */
bool isMutexLocked(SemaphoreHandle_t mutex);

/**
 * @brief Get mutex owner task
 * 
 * @param mutex Mutex to check
 * @return Task handle of mutex owner, or NULL if not locked
 */
TaskHandle_t getMutexOwner(SemaphoreHandle_t mutex);

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

/**
 * @brief Print safe (mutex-protected serial output)
 * 
 * Thread-safe version of Serial.print() that uses serial mutex.
 * 
 * @param message Message to print
 */
void safePrint(const char* message);

/**
 * @brief Print line safe (mutex-protected serial output)
 * 
 * Thread-safe version of Serial.println() that uses serial mutex.
 * 
 * @param message Message to print
 */
void safePrintln(const char* message);

/**
 * @brief Printf safe (mutex-protected serial output)
 * 
 * Thread-safe version of Serial.printf() that uses serial mutex.
 * 
 * @param format Printf format string
 * @param ... Variable arguments
 */
void safePrintf(const char* format, ...);

#endif // USE_RTOS
