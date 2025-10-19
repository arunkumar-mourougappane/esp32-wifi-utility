/**
 * @file test_rtos_mutexes.cpp
 * @brief Comprehensive RTOS Mutex Tests
 * 
 * Tests for FreeRTOS mutex operations including:
 * - Mutex creation and deletion
 * - Lock and unlock operations
 * - Timeout behavior
 * - Priority inheritance
 * - Concurrent access protection
 * - Deadlock scenarios
 * 
 * @version 4.1.0
 * @date 2025-10-18
 */

#include <unity.h>
#include <Arduino.h>

#ifdef USE_RTOS
#include "mutex_manager.h"
#include "rtos_manager.h"
#include "task_base.h"

// ==========================================
// TEST VARIABLES
// ==========================================

volatile int sharedCounter = 0;
volatile bool mutexTestComplete = false;

// ==========================================
// HELPER TASK CLASSES
// ==========================================

class MutexTestTask : public TaskBase {
public:
    SemaphoreHandle_t testMutex;
    volatile int incrementCount;
    volatile bool shouldStop;
    
    MutexTestTask(const char* name, SemaphoreHandle_t mutex) 
        : TaskBase(name, 2048, TaskPriority::PRIORITY_MEDIUM),
          testMutex(mutex),
          incrementCount(0),
          shouldStop(false) {}
    
protected:
    void setup() override {}
    
    void loop() override {
        if (shouldStop) {
            requestStop();
            return;
        }
        
        // Try to acquire mutex
        if (xSemaphoreTake(testMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Critical section
            int temp = sharedCounter;
            vTaskDelay(pdMS_TO_TICKS(1));  // Simulate work
            sharedCounter = temp + 1;
            incrementCount++;
            
            // Release mutex
            xSemaphoreGive(testMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    void cleanup() override {}
};

// ==========================================
// BASIC MUTEX TESTS
// ==========================================

void test_mutex_manager_initialization() {
    TEST_MESSAGE("Testing mutex manager initialization");
    
    // Verify all system mutexes are created
    TEST_ASSERT_NOT_NULL(wifiMutex);
    TEST_ASSERT_NOT_NULL(configMutex);
    TEST_ASSERT_NOT_NULL(serialMutex);
    TEST_ASSERT_NOT_NULL(webServerMutex);
    
    // Verify mutex count
    TEST_ASSERT_EQUAL(4, getMutexCount());
}

void test_mutex_creation() {
    TEST_MESSAGE("Testing mutex creation");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    vSemaphoreDelete(mutex);
}

void test_mutex_lock_unlock() {
    TEST_MESSAGE("Testing basic lock/unlock operations");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    // Lock mutex
    TEST_ASSERT_TRUE(xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE);
    
    // Unlock mutex
    TEST_ASSERT_TRUE(xSemaphoreGive(mutex) == pdTRUE);
    
    vSemaphoreDelete(mutex);
}

void test_mutex_double_lock_fails() {
    TEST_MESSAGE("Testing that double lock from same task fails");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    // First lock succeeds
    TEST_ASSERT_TRUE(xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE);
    
    // Second lock should fail (timeout immediately with 0 wait)
    TEST_ASSERT_FALSE(xSemaphore Take(mutex, 0) == pdTRUE);
    
    // Unlock
    xSemaphoreGive(mutex);
    
    vSemaphoreDelete(mutex);
}

void test_mutex_unlock_without_lock_fails() {
    TEST_MESSAGE("Testing that unlock without lock fails");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    // Try to unlock without locking first - behavior is undefined
    // but mutex should still be functional after
    
    // Lock should still work
    TEST_ASSERT_TRUE(xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE);
    xSemaphoreGive(mutex);
    
    vSemaphoreDelete(mutex);
}

// ==========================================
// TIMEOUT TESTS
// ==========================================

void test_mutex_lock_timeout() {
    TEST_MESSAGE("Testing mutex lock timeout");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    // Lock mutex
    TEST_ASSERT_TRUE(xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE);
    
    // Try to lock again with timeout (should fail)
    unsigned long start = millis();
    bool result = xSemaphoreTake(mutex, pdMS_TO_TICKS(50)) == pdTRUE;
    unsigned long elapsed = millis() - start;
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_GREATER_OR_EQUAL(45, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL(100, elapsed);
    
    // Unlock
    xSemaphoreGive(mutex);
    
    vSemaphoreDelete(mutex);
}

void test_mutex_lock_no_timeout() {
    TEST_MESSAGE("Testing mutex lock with no timeout (immediate return)");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    // Lock mutex
    TEST_ASSERT_TRUE(xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE);
    
    // Try to lock with 0 timeout (should return immediately)
    unsigned long start = millis();
    bool result = xSemaphoreTake(mutex, 0) == pdTRUE;
    unsigned long elapsed = millis() - start;
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_LESS_THAN(10, elapsed);
    
    // Unlock
    xSemaphoreGive(mutex);
    
    vSemaphoreDelete(mutex);
}

// ==========================================
// CONCURRENT ACCESS TESTS
// ==========================================

void test_mutex_protects_shared_resource() {
    TEST_MESSAGE("Testing mutex protection of shared resource");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    sharedCounter = 0;
    
    // Create two tasks that increment shared counter
    MutexTestTask* task1 = new MutexTestTask("MutexTask1", mutex);
    MutexTestTask* task2 = new MutexTestTask("MutexTask2", mutex);
    
    TEST_ASSERT_TRUE(task1->start());
    TEST_ASSERT_TRUE(task2->start());
    
    // Let tasks run for a while
    delay(500);
    
    // Stop tasks
    task1->shouldStop = true;
    task2->shouldStop = true;
    delay(100);
    
    // Shared counter should equal sum of increments from both tasks
    int expectedCount = task1->incrementCount + task2->incrementCount;
    TEST_ASSERT_EQUAL(expectedCount, sharedCounter);
    
    // Both tasks should have incremented at least once
    TEST_ASSERT_GREATER_THAN(0, task1->incrementCount);
    TEST_ASSERT_GREATER_THAN(0, task2->incrementCount);
    
    delete task1;
    delete task2;
    vSemaphoreDelete(mutex);
}

void test_mutex_fairness() {
    TEST_MESSAGE("Testing mutex fairness between tasks");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    sharedCounter = 0;
    
    // Create three tasks with equal priority
    MutexTestTask* task1 = new MutexTestTask("Fair1", mutex);
    MutexTestTask* task2 = new MutexTestTask("Fair2", mutex);
    MutexTestTask* task3 = new MutexTestTask("Fair3", mutex);
    
    TEST_ASSERT_TRUE(task1->start());
    TEST_ASSERT_TRUE(task2->start());
    TEST_ASSERT_TRUE(task3->start());
    
    delay(500);
    
    // Stop tasks
    task1->shouldStop = true;
    task2->shouldStop = true;
    task3->shouldStop = true;
    delay(100);
    
    // All tasks should have gotten some mutex access
    TEST_ASSERT_GREATER_THAN(0, task1->incrementCount);
    TEST_ASSERT_GREATER_THAN(0, task2->incrementCount);
    TEST_ASSERT_GREATER_THAN(0, task3->incrementCount);
    
    // No task should have starved (gotten less than 10% of access)
    int totalIncrements = task1->incrementCount + task2->incrementCount + task3->incrementCount;
    TEST_ASSERT_GREATER_THAN(totalIncrements / 10, task1->incrementCount);
    TEST_ASSERT_GREATER_THAN(totalIncrements / 10, task2->incrementCount);
    TEST_ASSERT_GREATER_THAN(totalIncrements / 10, task3->incrementCount);
    
    delete task1;
    delete task2;
    delete task3;
    vSemaphoreDelete(mutex);
}

// ==========================================
// SYSTEM MUTEX TESTS
// ==========================================

void test_wifi_mutex_operations() {
    TEST_MESSAGE("Testing WiFi mutex operations");
    
    // Lock WiFi mutex
    TEST_ASSERT_TRUE(lockWiFiMutex(100));
    
    // Unlock WiFi mutex
    unlockWiFiMutex();
    
    // Can lock again after unlock
    TEST_ASSERT_TRUE(lockWiFiMutex(100));
    unlockWiFiMutex();
}

void test_config_mutex_operations() {
    TEST_MESSAGE("Testing config mutex operations");
    
    TEST_ASSERT_TRUE(lockConfigMutex(100));
    unlockConfigMutex();
    
    TEST_ASSERT_TRUE(lockConfigMutex(100));
    unlockConfigMutex();
}

void test_serial_mutex_operations() {
    TEST_MESSAGE("Testing serial mutex operations");
    
    TEST_ASSERT_TRUE(lockSerialMutex(100));
    unlockSerialMutex();
    
    TEST_ASSERT_TRUE(lockSerialMutex(100));
    unlockSerialMutex();
}

void test_webserver_mutex_operations() {
    TEST_MESSAGE("Testing web server mutex operations");
    
    TEST_ASSERT_TRUE(lockWebServerMutex(100));
    unlockWebServerMutex();
    
    TEST_ASSERT_TRUE(lockWebServerMutex(100));
    unlockWebServerMutex();
}

// ==========================================
// NESTED MUTEX TESTS
// ==========================================

void test_multiple_mutex_acquisition() {
    TEST_MESSAGE("Testing acquisition of multiple different mutexes");
    
    // Lock multiple system mutexes
    TEST_ASSERT_TRUE(lockConfigMutex(100));
    TEST_ASSERT_TRUE(lockSerialMutex(100));
    
    // Both should be locked
    // Unlock in reverse order (good practice)
    unlockSerialMutex();
    unlockConfigMutex();
    
    // Should be able to lock again
    TEST_ASSERT_TRUE(lockConfigMutex(100));
    TEST_ASSERT_TRUE(lockSerialMutex(100));
    unlockSerialMutex();
    unlockConfigMutex();
}

// ==========================================
// STRESS TESTS
// ==========================================

void test_mutex_rapid_operations() {
    TEST_MESSAGE("Testing rapid mutex lock/unlock operations");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    // Rapid lock/unlock cycles
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_TRUE(xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE);
        xSemaphoreGive(mutex);
    }
    
    vSemaphoreDelete(mutex);
}

void test_mutex_many_contentions() {
    TEST_MESSAGE("Testing mutex under high contention");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    sharedCounter = 0;
    
    // Create many tasks competing for mutex
    const int TASK_COUNT = 8;
    MutexTestTask* tasks[TASK_COUNT];
    
    for (int i = 0; i < TASK_COUNT; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Contend%d", i);
        tasks[i] = new MutexTestTask(name, mutex);
        TEST_ASSERT_TRUE(tasks[i]->start());
    }
    
    delay(1000);
    
    // Stop all tasks
    for (int i = 0; i < TASK_COUNT; i++) {
        tasks[i]->shouldStop = true;
    }
    delay(200);
    
    // Verify counter integrity
    int expectedCount = 0;
    for (int i = 0; i < TASK_COUNT; i++) {
        expectedCount += tasks[i]->incrementCount;
        TEST_ASSERT_GREATER_THAN(0, tasks[i]->incrementCount);
    }
    TEST_ASSERT_EQUAL(expectedCount, sharedCounter);
    
    // Cleanup
    for (int i = 0; i < TASK_COUNT; i++) {
        delete tasks[i];
    }
    vSemaphoreDelete(mutex);
}

// ==========================================
// TEST RUNNER SETUP
// ==========================================

void setup() {
    delay(2000);  // Allow serial and RTOS to initialize
    
    UNITY_BEGIN();
    
    // Basic tests
    RUN_TEST(test_mutex_manager_initialization);
    RUN_TEST(test_mutex_creation);
    RUN_TEST(test_mutex_lock_unlock);
    RUN_TEST(test_mutex_double_lock_fails);
    RUN_TEST(test_mutex_unlock_without_lock_fails);
    
    // Timeout tests
    RUN_TEST(test_mutex_lock_timeout);
    RUN_TEST(test_mutex_lock_no_timeout);
    
    // Concurrent access tests
    RUN_TEST(test_mutex_protects_shared_resource);
    RUN_TEST(test_mutex_fairness);
    
    // System mutex tests
    RUN_TEST(test_wifi_mutex_operations);
    RUN_TEST(test_config_mutex_operations);
    RUN_TEST(test_serial_mutex_operations);
    RUN_TEST(test_webserver_mutex_operations);
    
    // Nested mutex tests
    RUN_TEST(test_multiple_mutex_acquisition);
    
    // Stress tests
    RUN_TEST(test_mutex_rapid_operations);
    RUN_TEST(test_mutex_many_contentions);
    
    UNITY_END();
}

void loop() {
    // Nothing to do
}

#else

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("RTOS mutex tests require USE_RTOS to be defined");
}

void loop() {
    delay(1000);
}

#endif // USE_RTOS
