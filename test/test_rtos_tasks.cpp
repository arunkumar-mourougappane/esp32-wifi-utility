/**
 * @file test_rtos_tasks.cpp
 * @brief Comprehensive RTOS Task Tests
 * 
 * Tests for FreeRTOS task operations including:
 * - Task creation and deletion
 * - Task lifecycle (start/stop/suspend/resume)
 * - Task priorities and scheduling
 * - Stack management
 * - Task states and monitoring
 * 
 * @version 4.1.0
 * @date 2025-10-18
 */

#include <unity.h>
#include <Arduino.h>

#ifdef USE_RTOS
#include "task_base.h"
#include "rtos_manager.h"
#include "command_task.h"
#include "wifi_task.h"
#include "led_task.h"

// ==========================================
// TEST TASK CLASS
// ==========================================

class TestTask : public TaskBase {
public:
    volatile bool setupCalled = false;
    volatile bool loopCalled = false;
    volatile bool cleanupCalled = false;
    volatile int loopCount = 0;
    volatile bool shouldExit = false;
    
    TestTask(const char* name, uint32_t stackSize, TaskPriority priority, int core = -1)
        : TaskBase(name, stackSize, priority, core) {}
    
protected:
    void setup() override {
        setupCalled = true;
    }
    
    void loop() override {
        loopCalled = true;
        loopCount++;
        
        if (shouldExit) {
            requestStop();
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    void cleanup() override {
        cleanupCalled = true;
    }
};

// ==========================================
// TASK CREATION TESTS
// ==========================================

void test_task_creation() {
    TEST_MESSAGE("Testing task creation");
    
    TestTask* task = new TestTask("TestTask1", 2048, TaskPriority::PRIORITY_LOW);
    TEST_ASSERT_NOT_NULL(task);
    
    // Initial state should be NOT_CREATED
    TEST_ASSERT_EQUAL(TaskState::NOT_CREATED, task->getState());
    TEST_ASSERT_FALSE(task->setupCalled);
    
    delete task;
}

void test_task_start() {
    TEST_MESSAGE("Testing task start");
    
    TestTask* task = new TestTask("TestTask2", 2048, TaskPriority::PRIORITY_LOW);
    
    // Start task
    TEST_ASSERT_TRUE(task->start());
    
    // Wait for setup to be called
    delay(100);
    
    // Verify task is running
    TEST_ASSERT_TRUE(task->setupCalled);
    TEST_ASSERT_TRUE(task->loopCalled);
    TEST_ASSERT_EQUAL(TaskState::RUNNING, task->getState());
    
    // Stop task
    task->shouldExit = true;
    delay(100);
    
    delete task;
}

void test_task_cannot_start_twice() {
    TEST_MESSAGE("Testing that task cannot be started twice");
    
    TestTask* task = new TestTask("TestTask3", 2048, TaskPriority::PRIORITY_LOW);
    
    TEST_ASSERT_TRUE(task->start());
    TEST_ASSERT_FALSE(task->start());  // Second start should fail
    
    task->shouldExit = true;
    delay(100);
    delete task;
}

// ==========================================
// TASK LIFECYCLE TESTS
// ==========================================

void test_task_suspend_resume() {
    TEST_MESSAGE("Testing task suspend and resume");
    
    TestTask* task = new TestTask("TestTask4", 2048, TaskPriority::PRIORITY_LOW);
    TEST_ASSERT_TRUE(task->start());
    delay(100);
    
    int countBefore = task->loopCount;
    
    // Suspend task
    task->suspend();
    TEST_ASSERT_EQUAL(TaskState::SUSPENDED, task->getState());
    delay(100);
    
    // Loop count should not increase while suspended
    int countDuringSuspend = task->loopCount;
    TEST_ASSERT_EQUAL(countBefore, countDuringSuspend);
    
    // Resume task
    task->resume();
    TEST_ASSERT_EQUAL(TaskState::RUNNING, task->getState());
    delay(100);
    
    // Loop count should increase after resume
    TEST_ASSERT_GREATER_THAN(countDuringSuspend, task->loopCount);
    
    task->shouldExit = true;
    delay(100);
    delete task;
}

void test_task_stop() {
    TEST_MESSAGE("Testing task stop");
    
    TestTask* task = new TestTask("TestTask5", 2048, TaskPriority::PRIORITY_LOW);
    TEST_ASSERT_TRUE(task->start());
    delay(100);
    
    TEST_ASSERT_TRUE(task->setupCalled);
    TEST_ASSERT_TRUE(task->loopCalled);
    
    // Request stop
    task->shouldExit = true;
    delay(200);
    
    // Cleanup should be called
    TEST_ASSERT_TRUE(task->cleanupCalled);
    
    delete task;
}

// ==========================================
// TASK PRIORITY TESTS
// ==========================================

void test_task_priority_settings() {
    TEST_MESSAGE("Testing task priority settings");
    
    TestTask* lowTask = new TestTask("LowPri", 2048, TaskPriority::PRIORITY_LOW);
    TestTask* medTask = new TestTask("MedPri", 2048, TaskPriority::PRIORITY_MEDIUM);
    TestTask* highTask = new TestTask("HighPri", 2048, TaskPriority::PRIORITY_HIGH);
    
    TEST_ASSERT_TRUE(lowTask->start());
    TEST_ASSERT_TRUE(medTask->start());
    TEST_ASSERT_TRUE(highTask->start());
    
    delay(100);
    
    // Higher priority tasks should generally execute more
    // (though this is not guaranteed in all scenarios)
    TEST_ASSERT_TRUE(lowTask->loopCalled);
    TEST_ASSERT_TRUE(medTask->loopCalled);
    TEST_ASSERT_TRUE(highTask->loopCalled);
    
    lowTask->shouldExit = true;
    medTask->shouldExit = true;
    highTask->shouldExit = true;
    delay(200);
    
    delete lowTask;
    delete medTask;
    delete highTask;
}

void test_task_priority_change() {
    TEST_MESSAGE("Testing task priority change");
    
    TestTask* task = new TestTask("PriChange", 2048, TaskPriority::PRIORITY_LOW);
    TEST_ASSERT_TRUE(task->start());
    delay(50);
    
    // Change priority
    task->setPriority(TaskPriority::PRIORITY_HIGH);
    delay(50);
    
    // Task should still be running
    TEST_ASSERT_EQUAL(TaskState::RUNNING, task->getState());
    TEST_ASSERT_TRUE(task->loopCalled);
    
    task->shouldExit = true;
    delay(100);
    delete task;
}

// ==========================================
// CORE AFFINITY TESTS
// ==========================================

void test_task_core_affinity() {
    TEST_MESSAGE("Testing task core affinity");
    
    // Core 0 task
    TestTask* core0Task = new TestTask("Core0Task", 2048, TaskPriority::PRIORITY_LOW, 0);
    TEST_ASSERT_TRUE(core0Task->start());
    
    // Core 1 task
    TestTask* core1Task = new TestTask("Core1Task", 2048, TaskPriority::PRIORITY_LOW, 1);
    TEST_ASSERT_TRUE(core1Task->start());
    
    // Any core task
    TestTask* anyTask = new TestTask("AnyTask", 2048, TaskPriority::PRIORITY_LOW, -1);
    TEST_ASSERT_TRUE(anyTask->start());
    
    delay(100);
    
    // All tasks should be running
    TEST_ASSERT_TRUE(core0Task->loopCalled);
    TEST_ASSERT_TRUE(core1Task->loopCalled);
    TEST_ASSERT_TRUE(anyTask->loopCalled);
    
    core0Task->shouldExit = true;
    core1Task->shouldExit = true;
    anyTask->shouldExit = true;
    delay(200);
    
    delete core0Task;
    delete core1Task;
    delete anyTask;
}

// ==========================================
// STACK TESTS
// ==========================================

void test_task_stack_monitoring() {
    TEST_MESSAGE("Testing task stack monitoring");
    
    TestTask* task = new TestTask("StackTest", 4096, TaskPriority::PRIORITY_LOW);
    TEST_ASSERT_TRUE(task->start());
    delay(100);
    
    // Get stack high water mark
    uint32_t stackRemaining = task->getStackHighWaterMark();
    TEST_ASSERT_GREATER_THAN(0, stackRemaining);
    TEST_ASSERT_LESS_THAN(4096, stackRemaining);
    
    // Stack usage percentage should be reasonable
    uint8_t stackUsage = task->getStackUsagePercent();
    TEST_ASSERT_LESS_THAN(50, stackUsage);  // Should use less than 50% for simple task
    
    task->shouldExit = true;
    delay(100);
    delete task;
}

void test_task_stack_sizes() {
    TEST_MESSAGE("Testing different stack sizes");
    
    // Small stack
    TestTask* smallTask = new TestTask("SmallStack", 1024, TaskPriority::PRIORITY_LOW);
    TEST_ASSERT_TRUE(smallTask->start());
    delay(50);
    TEST_ASSERT_TRUE(smallTask->loopCalled);
    
    // Medium stack
    TestTask* medTask = new TestTask("MedStack", 2048, TaskPriority::PRIORITY_LOW);
    TEST_ASSERT_TRUE(medTask->start());
    delay(50);
    TEST_ASSERT_TRUE(medTask->loopCalled);
    
    // Large stack
    TestTask* largeTask = new TestTask("LargeStack", 8192, TaskPriority::PRIORITY_LOW);
    TEST_ASSERT_TRUE(largeTask->start());
    delay(50);
    TEST_ASSERT_TRUE(largeTask->loopCalled);
    
    smallTask->shouldExit = true;
    medTask->shouldExit = true;
    largeTask->shouldExit = true;
    delay(200);
    
    delete smallTask;
    delete medTask;
    delete largeTask;
}

// ==========================================
// SYSTEM TASK TESTS
// ==========================================

void test_system_tasks_running() {
    TEST_MESSAGE("Testing that all system tasks are running");
    
    // Check that core system tasks exist and are operational
    // Note: These tests assume the tasks are initialized in main setup
    
    // System should be running
    TEST_ASSERT_TRUE(isRTOSRunning());
    TEST_ASSERT_EQUAL(RTOSState::RUNNING, getRTOSState());
    
    // Get statistics
    RTOSStatistics stats = getRTOSStatistics();
    TEST_ASSERT_GREATER_THAN(0, stats.totalHeapSize);
    TEST_ASSERT_GREATER_THAN(0, stats.freeHeapSize);
}

void test_task_count() {
    TEST_MESSAGE("Testing task count");
    
    RTOSStatistics stats = getRTOSStatistics();
    
    // Should have multiple tasks running (system tasks + test task)
    // At minimum: IDLE tasks (2), command task, wifi task, led task, test runner
    TEST_ASSERT_GREATER_THAN(4, stats.taskCount);
}

// ==========================================
// CONCURRENT TASK TESTS
// ==========================================

void test_multiple_tasks_concurrent() {
    TEST_MESSAGE("Testing multiple concurrent tasks");
    
    const int TASK_COUNT = 5;
    TestTask* tasks[TASK_COUNT];
    
    // Create and start multiple tasks
    for (int i = 0; i < TASK_COUNT; i++) {
        char name[32];
        snprintf(name, sizeof(name), "ConcTask%d", i);
        tasks[i] = new TestTask(name, 2048, TaskPriority::PRIORITY_LOW);
        TEST_ASSERT_TRUE(tasks[i]->start());
    }
    
    delay(200);
    
    // All tasks should be running
    for (int i = 0; i < TASK_COUNT; i++) {
        TEST_ASSERT_TRUE(tasks[i]->loopCalled);
        TEST_ASSERT_GREATER_THAN(0, tasks[i]->loopCount);
    }
    
    // Stop all tasks
    for (int i = 0; i < TASK_COUNT; i++) {
        tasks[i]->shouldExit = true;
    }
    delay(200);
    
    // Cleanup
    for (int i = 0; i < TASK_COUNT; i++) {
        delete tasks[i];
    }
}

void test_task_rapid_creation_deletion() {
    TEST_MESSAGE("Testing rapid task creation and deletion");
    
    for (int i = 0; i < 10; i++) {
        TestTask* task = new TestTask("RapidTask", 2048, TaskPriority::PRIORITY_LOW);
        TEST_ASSERT_TRUE(task->start());
        delay(50);
        task->shouldExit = true;
        delay(50);
        delete task;
    }
    
    // System should still be stable
    TEST_ASSERT_TRUE(isRTOSRunning());
}

// ==========================================
// TASK NAME TESTS
// ==========================================

void test_task_names() {
    TEST_MESSAGE("Testing task name retrieval");
    
    TestTask* task = new TestTask("NamedTask123", 2048, TaskPriority::PRIORITY_LOW);
    TEST_ASSERT_TRUE(task->start());
    delay(50);
    
    // Get task name
    String name = task->getName();
    TEST_ASSERT_EQUAL_STRING("NamedTask123", name.c_str());
    
    task->shouldExit = true;
    delay(100);
    delete task;
}

// ==========================================
// TEST RUNNER SETUP
// ==========================================

void setup() {
    delay(2000);  // Allow serial and RTOS to initialize
    
    UNITY_BEGIN();
    
    // Creation tests
    RUN_TEST(test_task_creation);
    RUN_TEST(test_task_start);
    RUN_TEST(test_task_cannot_start_twice);
    
    // Lifecycle tests
    RUN_TEST(test_task_suspend_resume);
    RUN_TEST(test_task_stop);
    
    // Priority tests
    RUN_TEST(test_task_priority_settings);
    RUN_TEST(test_task_priority_change);
    
    // Core affinity tests
    RUN_TEST(test_task_core_affinity);
    
    // Stack tests
    RUN_TEST(test_task_stack_monitoring);
    RUN_TEST(test_task_stack_sizes);
    
    // System tasks
    RUN_TEST(test_system_tasks_running);
    RUN_TEST(test_task_count);
    
    // Concurrent tests
    RUN_TEST(test_multiple_tasks_concurrent);
    RUN_TEST(test_task_rapid_creation_deletion);
    
    // Name tests
    RUN_TEST(test_task_names);
    
    UNITY_END();
}

void loop() {
    // Nothing to do
}

#else

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("RTOS task tests require USE_RTOS to be defined");
}

void loop() {
    delay(1000);
}

#endif // USE_RTOS
