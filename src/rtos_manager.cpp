#include "rtos_manager.h"

#ifdef USE_RTOS

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "queue_manager.h"
#include "mutex_manager.h"
#include "event_manager.h"

// ==========================================
// GLOBAL STATE
// ==========================================

RTOSState rtosState = RTOSState::UNINITIALIZED;
RTOSStatistics rtosStats = {0};

// ==========================================
// PRIVATE VARIABLES
// ==========================================

static unsigned long rtosInitTimeMs = 0;
static unsigned long lastStatsUpdateMs = 0;
static portMUX_TYPE _rtosManagerMux = portMUX_INITIALIZER_UNLOCKED;

// ==========================================
// CORE FUNCTIONS
// ==========================================

bool initializeRTOS() {
    if (rtosState != RTOSState::UNINITIALIZED) {
        Serial.println("[RTOS] Already initialized");
        return false;
    }
    
    Serial.println("[RTOS] Initializing FreeRTOS infrastructure...");
    rtosState = RTOSState::INITIALIZING;
    rtosInitTimeMs = millis();
    
    // Initialize queue manager
    Serial.println("[RTOS] Initializing queue manager...");
    if (!initializeQueueManager()) {
        rtosErrorHandler(1, "Queue manager initialization failed");
        rtosState = RTOSState::ERROR;
        return false;
    }
    
    // Initialize mutex manager
    Serial.println("[RTOS] Initializing mutex manager...");
    if (!initializeMutexManager()) {
        rtosErrorHandler(2, "Mutex manager initialization failed");
        rtosState = RTOSState::ERROR;
        return false;
    }
    
    // Initialize event manager
    Serial.println("[RTOS] Initializing event manager...");
    if (!initializeEventManager()) {
        rtosErrorHandler(3, "Event manager initialization failed");
        rtosState = RTOSState::ERROR;
        return false;
    }
    
    // Initialize statistics
    updateRTOSStatistics();
    
    rtosState = RTOSState::RUNNING;
    Serial.println("[RTOS] Initialization complete!");
    printRTOSInfo();
    
    return true;
}

bool shutdownRTOS() {
    if (rtosState == RTOSState::UNINITIALIZED) {
        Serial.println("[RTOS] Not initialized, nothing to shutdown");
        return true;
    }
    
    Serial.println("[RTOS] Shutting down FreeRTOS infrastructure...");
    rtosState = RTOSState::SHUTTING_DOWN;
    
    // Shutdown event manager
    shutdownEventManager();
    
    // Shutdown mutex manager
    shutdownMutexManager();
    
    // Shutdown queue manager
    shutdownQueueManager();
    
    rtosState = RTOSState::UNINITIALIZED;
    Serial.println("[RTOS] Shutdown complete");
    
    return true;
}

RTOSState getRTOSState() {
    return rtosState;
}

bool isRTOSRunning() {
    return rtosState == RTOSState::RUNNING;
}

// ==========================================
// STATISTICS & MONITORING
// ==========================================

void updateRTOSStatistics() {
    unsigned long now = millis();
    
    // Update heap statistics
    rtosStats.totalHeapSize = ESP.getHeapSize();
    rtosStats.freeHeapSize = ESP.getFreeHeap();
    rtosStats.minFreeHeapSize = ESP.getMinFreeHeap();
    
    // Update task count
    rtosStats.taskCount = uxTaskGetNumberOfTasks();
    
    // Update queue count (from queue manager)
    rtosStats.queueCount = getQueueCount();
    
    // Update mutex count (from mutex manager)
    rtosStats.mutexCount = getMutexCount();
    
    // Update uptime
    rtosStats.uptimeMs = now - rtosInitTimeMs;
    rtosStats.lastUpdateMs = now;
    
    lastStatsUpdateMs = now;
}

RTOSStatistics getRTOSStatistics() {
    // Update if enough time has passed
    unsigned long now = millis();
    if (now - lastStatsUpdateMs >= RTOS_STATS_UPDATE_INTERVAL_MS) {
        updateRTOSStatistics();
    }
    
    return rtosStats;
}

void printRTOSStatistics() {
    updateRTOSStatistics();
    
    Serial.println("\n=== RTOS Statistics ===");
    Serial.printf("State: ");
    switch (rtosState) {
        case RTOSState::UNINITIALIZED:
            Serial.println("UNINITIALIZED");
            break;
        case RTOSState::INITIALIZING:
            Serial.println("INITIALIZING");
            break;
        case RTOSState::RUNNING:
            Serial.println("RUNNING");
            break;
        case RTOSState::ERROR:
            Serial.println("ERROR");
            break;
        case RTOSState::SHUTTING_DOWN:
            Serial.println("SHUTTING_DOWN");
            break;
    }
    
    Serial.printf("Uptime: %lu ms (%.2f seconds)\n", 
                  rtosStats.uptimeMs, 
                  rtosStats.uptimeMs / 1000.0);
    
    Serial.println("\n--- Memory ---");
    Serial.printf("Total Heap: %u bytes (%.2f KB)\n", 
                  rtosStats.totalHeapSize, 
                  rtosStats.totalHeapSize / 1024.0);
    Serial.printf("Free Heap: %u bytes (%.2f KB, %.1f%%)\n", 
                  rtosStats.freeHeapSize, 
                  rtosStats.freeHeapSize / 1024.0,
                  (rtosStats.freeHeapSize * 100.0) / rtosStats.totalHeapSize);
    Serial.printf("Min Free Heap: %u bytes (%.2f KB)\n", 
                  rtosStats.minFreeHeapSize, 
                  rtosStats.minFreeHeapSize / 1024.0);
    
    Serial.println("\n--- Resources ---");
    Serial.printf("Tasks: %u\n", rtosStats.taskCount);
    Serial.printf("Queues: %u\n", rtosStats.queueCount);
    Serial.printf("Mutexes: %u\n", rtosStats.mutexCount);
    Serial.printf("Semaphores: %u\n", rtosStats.semaphoreCount);
    
    Serial.println("=====================\n");
}

void printTaskList() {
    Serial.println("\n=== Active Tasks ===");
    
    #if (configUSE_TRACE_FACILITY == 1)
    char taskListBuffer[512];
    vTaskList(taskListBuffer);
    
    Serial.println("Name          State Priority Stack  #");
    Serial.println("------------------------------------------");
    Serial.print(taskListBuffer);
    #else
    Serial.println("Task list unavailable (configUSE_TRACE_FACILITY not enabled)");
    Serial.printf("Total Tasks: %u\n", uxTaskGetNumberOfTasks());
    #endif
    
    Serial.println("==================\n");
    
    // Task states:
    // X = Running, B = Blocked, R = Ready, D = Deleted, S = Suspended
}

uint32_t getTaskCount() {
    return uxTaskGetNumberOfTasks();
}

uint32_t getFreeHeap() {
    return ESP.getFreeHeap();
}

uint32_t getMinFreeHeap() {
    return ESP.getMinFreeHeap();
}

// ==========================================
// DEADLOCK DETECTION
// ==========================================

#if RTOS_ENABLE_DEADLOCK_DETECTION

bool checkForDeadlocks() {
    // Get mutex wait times from mutex manager
    // This is a simple implementation - checks if any mutex
    // has been waiting longer than the timeout threshold
    
    return checkMutexDeadlocks();
}

void reportDeadlockInfo() {
    Serial.println("\n!!! DEADLOCK DETECTED !!!");
    Serial.println("=========================");
    
    printMutexDeadlockInfo();
    printTaskList();
    
    Serial.println("=========================\n");
}

#endif // RTOS_ENABLE_DEADLOCK_DETECTION

// ==========================================
// ERROR HANDLING
// ==========================================

void rtosErrorHandler(int errorCode, const char* errorMessage) {
    Serial.printf("\n!!! RTOS ERROR %d !!!\n", errorCode);
    Serial.println(errorMessage);
    Serial.println("===================\n");
    
    // Print diagnostic information
    printRTOSStatistics();
    printTaskList();
    
    // Set error state
    rtosState = RTOSState::ERROR;
}

bool checkRTOSHealth() {
    if (rtosState != RTOSState::RUNNING) {
        return false;
    }
    
    // Check heap health (at least 50KB should be free)
    if (getFreeHeap() < 50000) {
        Serial.println("[RTOS] WARNING: Low heap memory!");
        return false;
    }
    
    // Check for deadlocks
    #if RTOS_ENABLE_DEADLOCK_DETECTION
    if (checkForDeadlocks()) {
        reportDeadlockInfo();
        return false;
    }
    #endif
    
    // Check queue health
    if (!checkQueueHealth()) {
        Serial.println("[RTOS] WARNING: Queue health check failed!");
        return false;
    }
    
    return true;
}

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

TaskHandle_t getCurrentTask() {
    return xTaskGetCurrentTaskHandle();
}

TaskHandle_t getTaskByName(const char* taskName) {
    return xTaskGetHandle(taskName);
}

void rtosDelay(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void rtosEnterCritical() {
    portENTER_CRITICAL(&_rtosManagerMux);
}

void rtosExitCritical() {
    portEXIT_CRITICAL(&_rtosManagerMux);
}

// ==========================================
// VERSION INFORMATION
// ==========================================

const char* getRTOSVersion() {
    return tskKERNEL_VERSION_NUMBER;
}

void printRTOSInfo() {
    Serial.println("\n=== RTOS Information ===");
    Serial.printf("FreeRTOS Version: %s\n", getRTOSVersion());
    Serial.printf("Tick Rate: %d Hz\n", configTICK_RATE_HZ);
    Serial.printf("Max Priorities: %d\n", configMAX_PRIORITIES);
    Serial.printf("Minimal Stack Size: %d words\n", configMINIMAL_STACK_SIZE);
    Serial.printf("Total Heap Size: %u bytes\n", ESP.getHeapSize());
    
    #if RTOS_ENABLE_STATISTICS
    Serial.println("Statistics: ENABLED");
    #else
    Serial.println("Statistics: DISABLED");
    #endif
    
    #if RTOS_ENABLE_DEADLOCK_DETECTION
    Serial.println("Deadlock Detection: ENABLED");
    #else
    Serial.println("Deadlock Detection: DISABLED");
    #endif
    
    Serial.println("=======================\n");
}

#endif // USE_RTOS
