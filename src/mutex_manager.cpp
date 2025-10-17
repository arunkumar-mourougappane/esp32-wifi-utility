#include "mutex_manager.h"

#ifdef USE_RTOS

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdarg.h>

// ==========================================
// MUTEX HANDLES
// ==========================================

SemaphoreHandle_t wifiMutex = NULL;
SemaphoreHandle_t webServerMutex = NULL;
SemaphoreHandle_t scanResultsMutex = NULL;
SemaphoreHandle_t serialMutex = NULL;

// ==========================================
// MUTEX STATISTICS
// ==========================================

static MutexStatistics wifiMutexStats = {"WiFi"};
static MutexStatistics webServerMutexStats = {"WebServer"};
static MutexStatistics scanResultsMutexStats = {"ScanResults"};
static MutexStatistics serialMutexStats = {"Serial"};

// ==========================================
// PRIVATE FUNCTIONS
// ==========================================

static MutexStatistics* getStatsForMutex(SemaphoreHandle_t mutex) {
    if (mutex == wifiMutex) return &wifiMutexStats;
    if (mutex == webServerMutex) return &webServerMutexStats;
    if (mutex == scanResultsMutex) return &scanResultsMutexStats;
    if (mutex == serialMutex) return &serialMutexStats;
    return nullptr;
}

static void updateMutexStats(SemaphoreHandle_t mutex, bool locked, uint32_t waitTimeMs) {
    MutexStatistics* stats = getStatsForMutex(mutex);
    if (stats == nullptr) return;
    
    if (locked) {
        stats->lockCount++;
        stats->totalWaitTimeMs += waitTimeMs;
        if (waitTimeMs > stats->maxWaitTimeMs) {
            stats->maxWaitTimeMs = waitTimeMs;
        }
        if (stats->lockCount > 0) {
            stats->avgWaitTimeMs = stats->totalWaitTimeMs / stats->lockCount;
        }
        stats->currentlyLocked = true;
        stats->ownerTask = xTaskGetCurrentTaskHandle();
        stats->lockTimeMs = millis();
    } else {
        stats->unlockCount++;
        stats->currentlyLocked = false;
        stats->ownerTask = NULL;
    }
}

// ==========================================
// RAII MUTEX LOCK GUARD IMPLEMENTATION
// ==========================================

MutexLock::MutexLock(SemaphoreHandle_t mutex, const char* name, uint32_t timeoutMs)
    : mutexHandle(mutex), operationName(name), locked(false), waitTimeMs(0) {
    
    if (mutexHandle == NULL) {
        return;
    }
    
    lockStartMs = millis();
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    
    if (xSemaphoreTake(mutexHandle, timeout) == pdTRUE) {
        locked = true;
        waitTimeMs = millis() - lockStartMs;
        updateMutexStats(mutexHandle, true, waitTimeMs);
        
        if (operationName != nullptr && waitTimeMs > 100) {
            Serial.printf("[Mutex] WARNING: '%s' waited %ums for mutex\n", 
                         operationName, waitTimeMs);
        }
    } else {
        MutexStatistics* stats = getStatsForMutex(mutexHandle);
        if (stats) {
            stats->lockFailures++;
            stats->timeoutCount++;
        }
        
        if (operationName != nullptr) {
            Serial.printf("[Mutex] ERROR: '%s' failed to acquire mutex (timeout: %ums)\n", 
                         operationName, timeoutMs);
        }
    }
}

MutexLock::~MutexLock() {
    if (locked && mutexHandle != NULL) {
        xSemaphoreGive(mutexHandle);
        updateMutexStats(mutexHandle, false, 0);
        
        uint32_t holdTimeMs = millis() - lockStartMs;
        if (operationName != nullptr && holdTimeMs > 1000) {
            Serial.printf("[Mutex] WARNING: '%s' held mutex for %ums\n", 
                         operationName, holdTimeMs);
        }
    }
}

// ==========================================
// CORE FUNCTIONS
// ==========================================

bool initializeMutexManager() {
    Serial.println("[Mutex] Creating mutexes...");
    
    // Create WiFi mutex
    wifiMutex = xSemaphoreCreateMutex();
    if (wifiMutex == NULL) {
        Serial.println("[Mutex] ERROR: Failed to create WiFi mutex");
        return false;
    }
    Serial.println("[Mutex] WiFi mutex created");
    
    // Create web server mutex
    webServerMutex = xSemaphoreCreateMutex();
    if (webServerMutex == NULL) {
        Serial.println("[Mutex] ERROR: Failed to create web server mutex");
        return false;
    }
    Serial.println("[Mutex] Web server mutex created");
    
    // Create scan results mutex
    scanResultsMutex = xSemaphoreCreateMutex();
    if (scanResultsMutex == NULL) {
        Serial.println("[Mutex] ERROR: Failed to create scan results mutex");
        return false;
    }
    Serial.println("[Mutex] Scan results mutex created");
    
    // Create serial mutex
    serialMutex = xSemaphoreCreateMutex();
    if (serialMutex == NULL) {
        Serial.println("[Mutex] ERROR: Failed to create serial mutex");
        return false;
    }
    Serial.println("[Mutex] Serial mutex created");
    
    // Reset statistics
    resetMutexStatistics();
    
    Serial.println("[Mutex] All mutexes created successfully");
    return true;
}

bool shutdownMutexManager() {
    Serial.println("[Mutex] Deleting mutexes...");
    
    if (serialMutex != NULL) {
        vSemaphoreDelete(serialMutex);
        serialMutex = NULL;
    }
    
    if (scanResultsMutex != NULL) {
        vSemaphoreDelete(scanResultsMutex);
        scanResultsMutex = NULL;
    }
    
    if (webServerMutex != NULL) {
        vSemaphoreDelete(webServerMutex);
        webServerMutex = NULL;
    }
    
    if (wifiMutex != NULL) {
        vSemaphoreDelete(wifiMutex);
        wifiMutex = NULL;
    }
    
    Serial.println("[Mutex] All mutexes deleted");
    return true;
}

uint32_t getMutexCount() {
    uint32_t count = 0;
    if (wifiMutex != NULL) count++;
    if (webServerMutex != NULL) count++;
    if (scanResultsMutex != NULL) count++;
    if (serialMutex != NULL) count++;
    return count;
}

bool checkMutexDeadlocks() {
    bool deadlockDetected = false;
    unsigned long now = millis();
    
    // Check each mutex for potential deadlock
    MutexStatistics* mutexStats[] = {
        &wifiMutexStats,
        &webServerMutexStats,
        &scanResultsMutexStats,
        &serialMutexStats
    };
    
    for (int i = 0; i < 4; i++) {
        MutexStatistics* stats = mutexStats[i];
        if (stats->currentlyLocked) {
            uint32_t lockDuration = now - stats->lockTimeMs;
            if (lockDuration > MUTEX_DEADLOCK_THRESHOLD_MS) {
                Serial.printf("[Mutex] DEADLOCK WARNING: %s mutex locked for %ums\n",
                             stats->name, lockDuration);
                deadlockDetected = true;
            }
        }
    }
    
    return deadlockDetected;
}

void printMutexDeadlockInfo() {
    Serial.println("\n=== Mutex Deadlock Information ===");
    
    MutexStatistics* mutexStats[] = {
        &wifiMutexStats,
        &webServerMutexStats,
        &scanResultsMutexStats,
        &serialMutexStats
    };
    
    for (int i = 0; i < 4; i++) {
        MutexStatistics* stats = mutexStats[i];
        if (stats->currentlyLocked) {
            Serial.printf("\n%s Mutex:\n", stats->name);
            Serial.printf("  Status: LOCKED\n");
            Serial.printf("  Lock Duration: %lums\n", millis() - stats->lockTimeMs);
            if (stats->ownerTask != NULL) {
                Serial.printf("  Owner Task: 0x%08X\n", (uint32_t)stats->ownerTask);
            }
        }
    }
    
    Serial.println("=================================\n");
}

// ==========================================
// MUTEX LOCK/UNLOCK FUNCTIONS
// ==========================================

bool lockMutex(SemaphoreHandle_t mutex, uint32_t timeoutMs) {
    if (mutex == NULL) return false;
    
    unsigned long startMs = millis();
    TickType_t timeout = pdMS_TO_TICKS(timeoutMs);
    
    if (xSemaphoreTake(mutex, timeout) == pdTRUE) {
        uint32_t waitTimeMs = millis() - startMs;
        updateMutexStats(mutex, true, waitTimeMs);
        return true;
    }
    
    MutexStatistics* stats = getStatsForMutex(mutex);
    if (stats) {
        stats->lockFailures++;
        stats->timeoutCount++;
    }
    
    return false;
}

bool unlockMutex(SemaphoreHandle_t mutex) {
    if (mutex == NULL) return false;
    
    if (xSemaphoreGive(mutex) == pdTRUE) {
        updateMutexStats(mutex, false, 0);
        return true;
    }
    
    return false;
}

bool tryLockMutex(SemaphoreHandle_t mutex) {
    if (mutex == NULL) return false;
    
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
        updateMutexStats(mutex, true, 0);
        return true;
    }
    
    return false;
}

// ==========================================
// SPECIFIC MUTEX FUNCTIONS
// ==========================================

bool lockWiFiMutex(uint32_t timeoutMs) {
    return lockMutex(wifiMutex, timeoutMs);
}

bool unlockWiFiMutex() {
    return unlockMutex(wifiMutex);
}

bool lockWebServerMutex(uint32_t timeoutMs) {
    return lockMutex(webServerMutex, timeoutMs);
}

bool unlockWebServerMutex() {
    return unlockMutex(webServerMutex);
}

bool lockScanResultsMutex(uint32_t timeoutMs) {
    return lockMutex(scanResultsMutex, timeoutMs);
}

bool unlockScanResultsMutex() {
    return unlockMutex(scanResultsMutex);
}

bool lockSerialMutex(uint32_t timeoutMs) {
    return lockMutex(serialMutex, timeoutMs);
}

bool unlockSerialMutex() {
    return unlockMutex(serialMutex);
}

// ==========================================
// STATISTICS & MONITORING
// ==========================================

MutexStatistics getMutexStatistics(SemaphoreHandle_t mutex) {
    MutexStatistics* stats = getStatsForMutex(mutex);
    if (stats) {
        return *stats;
    }
    return MutexStatistics{"Unknown"};
}

void printMutexStatistics() {
    Serial.println("\n=== Mutex Statistics ===");
    
    MutexStatistics* mutexStats[] = {
        &wifiMutexStats,
        &webServerMutexStats,
        &scanResultsMutexStats,
        &serialMutexStats
    };
    
    for (int i = 0; i < 4; i++) {
        MutexStatistics* stats = mutexStats[i];
        Serial.printf("\n--- %s Mutex ---\n", stats->name);
        Serial.printf("Locks: %u | Unlocks: %u | Failures: %u | Timeouts: %u\n",
                     stats->lockCount, stats->unlockCount, 
                     stats->lockFailures, stats->timeoutCount);
        Serial.printf("Wait Times: Avg=%ums | Max=%ums | Total=%ums\n",
                     stats->avgWaitTimeMs, stats->maxWaitTimeMs, stats->totalWaitTimeMs);
        Serial.printf("Currently Locked: %s\n", stats->currentlyLocked ? "YES" : "NO");
        if (stats->currentlyLocked && stats->ownerTask != NULL) {
            Serial.printf("Owner Task: 0x%08X (held for %lums)\n",
                         (uint32_t)stats->ownerTask, millis() - stats->lockTimeMs);
        }
    }
    
    Serial.println("=======================\n");
}

void resetMutexStatistics() {
    wifiMutexStats = {"WiFi", 0};
    webServerMutexStats = {"WebServer", 0};
    scanResultsMutexStats = {"ScanResults", 0};
    serialMutexStats = {"Serial", 0};
}

bool isMutexLocked(SemaphoreHandle_t mutex) {
    if (mutex == NULL) return false;
    
    // Try to take mutex with zero timeout
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
        // We got it, so it wasn't locked - give it back
        xSemaphoreGive(mutex);
        return false;
    }
    
    // Couldn't get it, so it's locked
    return true;
}

TaskHandle_t getMutexOwner(SemaphoreHandle_t mutex) {
    MutexStatistics* stats = getStatsForMutex(mutex);
    if (stats && stats->currentlyLocked) {
        return stats->ownerTask;
    }
    return NULL;
}

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

void safePrint(const char* message) {
    if (lockSerialMutex(100)) {
        Serial.print(message);
        unlockSerialMutex();
    }
}

void safePrintln(const char* message) {
    if (lockSerialMutex(100)) {
        Serial.println(message);
        unlockSerialMutex();
    }
}

void safePrintf(const char* format, ...) {
    if (lockSerialMutex(100)) {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.print(buffer);
        unlockSerialMutex();
    }
}

#endif // USE_RTOS
