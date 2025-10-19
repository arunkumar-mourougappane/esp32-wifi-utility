/**
 * @file test_rtos_stress.cpp
 * @brief RTOS Stress Tests
 * 
 * Stress tests for system stability:
 * - High load scenarios
 * - Queue flooding
 * - Rapid task creation/deletion
 * - Memory pressure
 * - Long-running stability tests
 * - Concurrent operations at scale
 * 
 * @version 4.1.0
 * @date 2025-10-18
 */

#include <unity.h>
#include <Arduino.h>

#ifdef USE_RTOS
#include "rtos_manager.h"
#include "queue_manager.h"
#include "mutex_manager.h"
#include "task_base.h"

// ==========================================
// STRESS TEST CONFIGURATION
// ==========================================

#define STRESS_DURATION_MS 5000
#define STRESS_SHORT_DURATION_MS 2000
#define STRESS_VERY_SHORT_DURATION_MS 500

// ==========================================
// HELPER FUNCTIONS
// ==========================================

void clearAllQueues() {
    CommandRequest cmd;
    while (xQueueReceive(commandQueue, &cmd, 0) == pdTRUE) {}
    
    WiFiEvent event;
    while (xQueueReceive(wifiEventQueue, &event, 0) == pdTRUE) {}
    
    SystemStatus status;
    while (xQueueReceive(statusQueue, &status, 0) == pdTRUE) {}
}

// ==========================================
// QUEUE STRESS TESTS
// ==========================================

void test_queue_flooding() {
    TEST_MESSAGE("Stress test: Queue flooding");
    
    clearAllQueues();
    
    RTOSStatistics statsBefore = getRTOSStatistics();
    
    // Flood queue with commands
    unsigned long start = millis();
    int sentCount = 0;
    int failedCount = 0;
    
    while (millis() - start < STRESS_DURATION_MS) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = sentCount;
        cmd.timestamp = millis();
        
        if (sendCommand(cmd, 0)) {  // No timeout
            sentCount++;
        } else {
            failedCount++;
        }
        
        // Also drain some to simulate processing
        if (sentCount % 10 == 0) {
            CommandRequest received;
            receiveCommand(received, 0);
        }
    }
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Sent: %d, Failed: %d in %dms", sentCount, failedCount, STRESS_DURATION_MS);
    TEST_MESSAGE(msg);
    
    // System should remain stable
    TEST_ASSERT_TRUE(isRTOSRunning());
    TEST_ASSERT_TRUE(checkRTOSHealth());
    
    // Memory should be relatively stable
    RTOSStatistics statsAfter = getRTOSStatistics();
    int memoryChange = statsBefore.freeHeapSize - statsAfter.freeHeapSize;
    TEST_ASSERT_LESS_THAN(5000, abs(memoryChange));  // Less than 5KB change
    
    clearAllQueues();
}

void test_multi_queue_stress() {
    TEST_MESSAGE("Stress test: Multiple queue flooding");
    
    clearAllQueues();
    
    unsigned long start = millis();
    int cmdCount = 0, eventCount = 0, statusCount = 0;
    
    while (millis() - start < STRESS_SHORT_DURATION_MS) {
        // Send to all queues
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = cmdCount;
        if (sendCommand(cmd, 0)) cmdCount++;
        
        WiFiEvent event;
        event.type = WiFiEvent::EventType::SCAN_STARTED;
        event.timestamp = millis();
        if (sendWiFiEvent(event, 0)) eventCount++;
        
        SystemStatus status;
        status.wifiConnected = (cmdCount % 2 == 0);
        status.timestamp = millis();
        if (sendSystemStatus(status, 0)) statusCount++;
        
        // Occasionally drain
        if (cmdCount % 5 == 0) {
            CommandRequest rcmd;
            receiveCommand(rcmd, 0);
            WiFiEvent revent;
            receiveWiFiEvent(revent, 0);
            SystemStatus rstatus;
            receiveSystemStatus(rstatus, 0);
        }
    }
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Sent - Cmd: %d, Event: %d, Status: %d", 
             cmdCount, eventCount, statusCount);
    TEST_MESSAGE(msg);
    
    // All queues should have received items
    TEST_ASSERT_GREATER_THAN(100, cmdCount);
    TEST_ASSERT_GREATER_THAN(100, eventCount);
    TEST_ASSERT_GREATER_THAN(100, statusCount);
    
    // System should be stable
    TEST_ASSERT_TRUE(isRTOSRunning());
    
    clearAllQueues();
}

// ==========================================
// MUTEX STRESS TESTS
// ==========================================

class MutexStressTask : public TaskBase {
public:
    SemaphoreHandle_t mutex;
    volatile int lockCount;
    volatile bool shouldStop;
    
    MutexStressTask(const char* name, SemaphoreHandle_t m) 
        : TaskBase(name, 2048, TaskPriority::PRIORITY_MEDIUM),
          mutex(m), lockCount(0), shouldStop(false) {}
    
protected:
    void setup() override {}
    
    void loop() override {
        if (shouldStop) {
            requestStop();
            return;
        }
        
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            lockCount++;
            vTaskDelay(pdMS_TO_TICKS(1));
            xSemaphoreGive(mutex);
        }
        
        taskYIELD();
    }
    
    void cleanup() override {}
};

void test_mutex_high_contention() {
    TEST_MESSAGE("Stress test: Mutex high contention");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    // Create many tasks competing for mutex
    const int TASK_COUNT = 10;
    MutexStressTask* tasks[TASK_COUNT];
    
    for (int i = 0; i < TASK_COUNT; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Stress%d", i);
        tasks[i] = new MutexStressTask(name, mutex);
        TEST_ASSERT_TRUE(tasks[i]->start());
    }
    
    delay(STRESS_DURATION_MS);
    
    // Stop all tasks
    for (int i = 0; i < TASK_COUNT; i++) {
        tasks[i]->shouldStop = true;
    }
    delay(200);
    
    // Verify lock counts
    int totalLocks = 0;
    for (int i = 0; i < TASK_COUNT; i++) {
        TEST_ASSERT_GREATER_THAN(0, tasks[i]->lockCount);
        totalLocks += tasks[i]->lockCount;
    }
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Total mutex locks: %d over %dms", totalLocks, STRESS_DURATION_MS);
    TEST_MESSAGE(msg);
    
    // System should be stable
    TEST_ASSERT_TRUE(isRTOSRunning());
    
    // Cleanup
    for (int i = 0; i < TASK_COUNT; i++) {
        delete tasks[i];
    }
    vSemaphoreDelete(mutex);
}

// ==========================================
// TASK STRESS TESTS
// ==========================================

class SimpleStressTask : public TaskBase {
public:
    volatile int iterations;
    volatile bool shouldStop;
    
    SimpleStressTask(const char* name) 
        : TaskBase(name, 2048, TaskPriority::PRIORITY_LOW),
          iterations(0), shouldStop(false) {}
    
protected:
    void setup() override {}
    
    void loop() override {
        if (shouldStop) {
            requestStop();
            return;
        }
        iterations++;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    void cleanup() override {}
};

void test_many_concurrent_tasks() {
    TEST_MESSAGE("Stress test: Many concurrent tasks");
    
    RTOSStatistics statsBefore = getRTOSStatistics();
    
    const int TASK_COUNT = 15;
    SimpleStressTask* tasks[TASK_COUNT];
    
    // Create many tasks
    for (int i = 0; i < TASK_COUNT; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Concurrent%d", i);
        tasks[i] = new SimpleStressTask(name);
        TEST_ASSERT_TRUE(tasks[i]->start());
    }
    
    delay(STRESS_SHORT_DURATION_MS);
    
    // Stop all tasks
    for (int i = 0; i < TASK_COUNT; i++) {
        tasks[i]->shouldStop = true;
    }
    delay(200);
    
    // Verify all tasks ran
    for (int i = 0; i < TASK_COUNT; i++) {
        TEST_ASSERT_GREATER_THAN(0, tasks[i]->iterations);
    }
    
    // System should be stable
    TEST_ASSERT_TRUE(isRTOSRunning());
    
    // Memory should be relatively stable
    RTOSStatistics statsAfter = getRTOSStatistics();
    int memoryChange = statsBefore.freeHeapSize - statsAfter.freeHeapSize;
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Memory change with %d tasks: %d bytes", TASK_COUNT, memoryChange);
    TEST_MESSAGE(msg);
    
    // Cleanup
    for (int i = 0; i < TASK_COUNT; i++) {
        delete tasks[i];
    }
    
    // Allow cleanup to complete
    delay(100);
    
    // Final memory check
    RTOSStatistics statsCleanup = getRTOSStatistics();
    TEST_MESSAGE("After cleanup:");
    snprintf(msg, sizeof(msg), "Free heap: %lu bytes", statsCleanup.freeHeapSize);
    TEST_MESSAGE(msg);
}

void test_rapid_task_cycling() {
    TEST_MESSAGE("Stress test: Rapid task creation/deletion");
    
    RTOSStatistics statsBefore = getRTOSStatistics();
    
    unsigned long start = millis();
    int cycles = 0;
    
    while (millis() - start < STRESS_SHORT_DURATION_MS) {
        SimpleStressTask* task = new SimpleStressTask("CycleTask");
        TEST_ASSERT_TRUE(task->start());
        delay(20);
        task->shouldStop = true;
        delay(20);
        delete task;
        cycles++;
    }
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Created/deleted %d tasks in %dms", cycles, STRESS_SHORT_DURATION_MS);
    TEST_MESSAGE(msg);
    
    // System should be stable
    TEST_ASSERT_TRUE(isRTOSRunning());
    TEST_ASSERT_TRUE(checkRTOSHealth());
    
    // Memory should be relatively stable
    RTOSStatistics statsAfter = getRTOSStatistics();
    int memoryChange = statsBefore.freeHeapSize - statsAfter.freeHeapSize;
    TEST_ASSERT_LESS_THAN(2000, abs(memoryChange));  // Less than 2KB change
}

// ==========================================
// MEMORY STRESS TESTS
// ==========================================

void test_memory_pressure() {
    TEST_MESSAGE("Stress test: Memory pressure");
    
    RTOSStatistics statsBefore = getRTOSStatistics();
    
    const int ALLOC_COUNT = 50;
    void* allocations[ALLOC_COUNT];
    int successfulAllocs = 0;
    
    // Allocate large chunks
    for (int i = 0; i < ALLOC_COUNT; i++) {
        allocations[i] = malloc(4096);  // 4KB each
        if (allocations[i] != nullptr) {
            successfulAllocs++;
            memset(allocations[i], 0xAA, 4096);  // Use the memory
        }
    }
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Allocated %d x 4KB blocks (%d KB total)", 
             successfulAllocs, successfulAllocs * 4);
    TEST_MESSAGE(msg);
    
    // System should still be stable
    TEST_ASSERT_TRUE(isRTOSRunning());
    
    // Free all allocations
    for (int i = 0; i < ALLOC_COUNT; i++) {
        if (allocations[i] != nullptr) {
            free(allocations[i]);
        }
    }
    
    delay(100);  // Allow cleanup
    
    // Memory should be recovered
    RTOSStatistics statsAfter = getRTOSStatistics();
    int memoryRecovered = statsAfter.freeHeapSize - (statsBefore.freeHeapSize - successfulAllocs * 4096);
    
    snprintf(msg, sizeof(msg), "Memory recovered: %d bytes", memoryRecovered);
    TEST_MESSAGE(msg);
    
    // Should recover most memory (some fragmentation ok)
    TEST_ASSERT_GREATER_THAN(successfulAllocs * 4096 * 0.8, memoryRecovered);
}

// ==========================================
// COMBINED STRESS TESTS
// ==========================================

void test_combined_stress() {
    TEST_MESSAGE("Stress test: Combined operations");
    
    RTOSStatistics statsBefore = getRTOSStatistics();
    clearAllQueues();
    
    // Create some background tasks
    SimpleStressTask* task1 = new SimpleStressTask("CombTask1");
    SimpleStressTask* task2 = new SimpleStressTask("CombTask2");
    TEST_ASSERT_TRUE(task1->start());
    TEST_ASSERT_TRUE(task2->start());
    
    // Stress queues, mutexes, and memory simultaneously
    unsigned long start = millis();
    int operations = 0;
    
    while (millis() - start < STRESS_VERY_SHORT_DURATION_MS) {
        // Queue operations
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = operations;
        sendCommand(cmd, 0);
        
        if (operations % 5 == 0) {
            CommandRequest rcmd;
            receiveCommand(rcmd, 0);
        }
        
        // Mutex operations
        if (lockConfigMutex(10)) {
            unlockConfigMutex();
        }
        
        // Small allocations
        if (operations % 10 == 0) {
            void* ptr = malloc(512);
            if (ptr) free(ptr);
        }
        
        operations++;
    }
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Combined stress: %d operations in %dms", 
             operations, STRESS_VERY_SHORT_DURATION_MS);
    TEST_MESSAGE(msg);
    
    // Stop tasks
    task1->shouldStop = true;
    task2->shouldStop = true;
    delay(100);
    
    // System should be stable
    TEST_ASSERT_TRUE(isRTOSRunning());
    TEST_ASSERT_TRUE(checkRTOSHealth());
    
    // Cleanup
    delete task1;
    delete task2;
    clearAllQueues();
    
    // Memory should be relatively stable
    RTOSStatistics statsAfter = getRTOSStatistics();
    int memoryChange = statsBefore.freeHeapSize - statsAfter.freeHeapSize;
    TEST_ASSERT_LESS_THAN(5000, abs(memoryChange));
}

// ==========================================
// LONG-RUNNING STABILITY TEST
// ==========================================

void test_long_running_stability() {
    TEST_MESSAGE("Stress test: Long-running stability (reduced for testing)");
    
    RTOSStatistics statsBefore = getRTOSStatistics();
    clearAllQueues();
    
    // Run for extended period (reduced from 24 hours for practical testing)
    const unsigned long RUN_DURATION = 10000;  // 10 seconds (normally would be longer)
    unsigned long start = millis();
    int cycles = 0;
    
    while (millis() - start < RUN_DURATION) {
        // Continuous queue operations
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = cycles;
        sendCommand(cmd, 100);
        
        CommandRequest received;
        receiveCommand(received, 100);
        
        // Periodic health check
        if (cycles % 100 == 0) {
            TEST_ASSERT_TRUE(isRTOSRunning());
            TEST_ASSERT_TRUE(checkRTOSHealth());
        }
        
        cycles++;
        delay(10);
    }
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Completed %d cycles over %lu ms", cycles, RUN_DURATION);
    TEST_MESSAGE(msg);
    
    // Final health check
    TEST_ASSERT_TRUE(isRTOSRunning());
    TEST_ASSERT_TRUE(checkRTOSHealth());
    
    RTOSStatistics statsAfter = getRTOSStatistics();
    snprintf(msg, sizeof(msg), "Memory - Before: %lu, After: %lu, Change: %d",
             statsBefore.freeHeapSize, statsAfter.freeHeapSize,
             statsBefore.freeHeapSize - statsAfter.freeHeapSize);
    TEST_MESSAGE(msg);
    
    // No significant memory leak
    int memoryChange = statsBefore.freeHeapSize - statsAfter.freeHeapSize;
    TEST_ASSERT_LESS_THAN(5000, abs(memoryChange));
}

// ==========================================
// TEST RUNNER SETUP
// ==========================================

void setup() {
    delay(2000);  // Allow serial and RTOS to initialize
    
    UNITY_BEGIN();
    
    TEST_MESSAGE("=== STARTING RTOS STRESS TESTS ===");
    TEST_MESSAGE("These tests push the system to its limits");
    TEST_MESSAGE("=====================================");
    
    // Queue stress
    RUN_TEST(test_queue_flooding);
    RUN_TEST(test_multi_queue_stress);
    
    // Mutex stress
    RUN_TEST(test_mutex_high_contention);
    
    // Task stress
    RUN_TEST(test_many_concurrent_tasks);
    RUN_TEST(test_rapid_task_cycling);
    
    // Memory stress
    RUN_TEST(test_memory_pressure);
    
    // Combined stress
    RUN_TEST(test_combined_stress);
    
    // Long-running
    RUN_TEST(test_long_running_stability);
    
    TEST_MESSAGE("=== STRESS TESTS COMPLETE ===");
    
    UNITY_END();
}

void loop() {
    // Nothing to do
}

#else

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("RTOS stress tests require USE_RTOS to be defined");
}

void loop() {
    delay(1000);
}

#endif // USE_RTOS
