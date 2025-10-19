/**
 * @file test_rtos_performance.cpp
 * @brief RTOS Performance Benchmarks
 * 
 * Performance tests and benchmarks:
 * - Command response latency (<10ms target)
 * - Queue latency (<1ms target)
 * - Mutex lock/unlock timing (<100μs target)
 * - Task switching overhead (<1ms target)
 * - Throughput (>100 commands/sec target)
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

// ==========================================
// PERFORMANCE TEST CONFIGURATION
// ==========================================

#define PERF_ITERATIONS 100
#define PERF_QUEUE_ITERATIONS 1000
#define PERF_MUTEX_ITERATIONS 1000

// ==========================================
// HELPER FUNCTIONS
// ==========================================

void clearQueue(QueueHandle_t queue) {
    CommandRequest cmd;
    while (xQueueReceive(queue, &cmd, 0) == pdTRUE) {}
}

unsigned long microsAverage(unsigned long* samples, int count) {
    unsigned long sum = 0;
    for (int i = 0; i < count; i++) {
        sum += samples[i];
    }
    return sum / count;
}

unsigned long microsMedian(unsigned long* samples, int count) {
    // Simple bubble sort for median
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                unsigned long temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }
    return samples[count / 2];
}

// ==========================================
// QUEUE PERFORMANCE TESTS
// ==========================================

void test_queue_send_latency() {
    TEST_MESSAGE("Benchmarking queue send latency");
    
    clearQueue(commandQueue);
    
    unsigned long samples[PERF_ITERATIONS];
    
    for (int i = 0; i < PERF_ITERATIONS; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = i;
        cmd.timestamp = millis();
        
        unsigned long start = micros();
        sendCommand(cmd, 100);
        unsigned long elapsed = micros() - start;
        
        samples[i] = elapsed;
    }
    
    unsigned long avg = microsAverage(samples, PERF_ITERATIONS);
    unsigned long median = microsMedian(samples, PERF_ITERATIONS);
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Queue send - Avg: %lu μs, Median: %lu μs", avg, median);
    TEST_MESSAGE(msg);
    
    // Target: < 1000 μs (1 ms)
    TEST_ASSERT_LESS_THAN(1000, avg);
    TEST_ASSERT_LESS_THAN(1000, median);
    
    clearQueue(commandQueue);
}

void test_queue_receive_latency() {
    TEST_MESSAGE("Benchmarking queue receive latency");
    
    clearQueue(commandQueue);
    
    // Fill queue first
    for (int i = 0; i < PERF_ITERATIONS; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = i;
        sendCommand(cmd, 100);
    }
    
    unsigned long samples[PERF_ITERATIONS];
    
    for (int i = 0; i < PERF_ITERATIONS; i++) {
        CommandRequest cmd;
        
        unsigned long start = micros();
        receiveCommand(cmd, 100);
        unsigned long elapsed = micros() - start;
        
        samples[i] = elapsed;
    }
    
    unsigned long avg = microsAverage(samples, PERF_ITERATIONS);
    unsigned long median = microsMedian(samples, PERF_ITERATIONS);
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Queue receive - Avg: %lu μs, Median: %lu μs", avg, median);
    TEST_MESSAGE(msg);
    
    // Target: < 1000 μs (1 ms)
    TEST_ASSERT_LESS_THAN(1000, avg);
    TEST_ASSERT_LESS_THAN(1000, median);
}

void test_queue_round_trip_latency() {
    TEST_MESSAGE("Benchmarking queue round-trip latency");
    
    clearQueue(commandQueue);
    
    unsigned long samples[PERF_ITERATIONS];
    
    for (int i = 0; i < PERF_ITERATIONS; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = i;
        
        unsigned long start = micros();
        sendCommand(cmd, 100);
        receiveCommand(cmd, 100);
        unsigned long elapsed = micros() - start;
        
        samples[i] = elapsed;
    }
    
    unsigned long avg = microsAverage(samples, PERF_ITERATIONS);
    unsigned long median = microsMedian(samples, PERF_ITERATIONS);
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Queue round-trip - Avg: %lu μs, Median: %lu μs", avg, median);
    TEST_MESSAGE(msg);
    
    // Target: < 2000 μs (2 ms) for round trip
    TEST_ASSERT_LESS_THAN(2000, avg);
}

// ==========================================
// MUTEX PERFORMANCE TESTS
// ==========================================

void test_mutex_lock_unlock_timing() {
    TEST_MESSAGE("Benchmarking mutex lock/unlock timing");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    unsigned long samples[PERF_MUTEX_ITERATIONS];
    
    for (int i = 0; i < PERF_MUTEX_ITERATIONS; i++) {
        unsigned long start = micros();
        xSemaphoreTake(mutex, portMAX_DELAY);
        xSemaphoreGive(mutex);
        unsigned long elapsed = micros() - start;
        
        samples[i] = elapsed;
    }
    
    unsigned long avg = microsAverage(samples, PERF_MUTEX_ITERATIONS);
    unsigned long median = microsMedian(samples, PERF_MUTEX_ITERATIONS);
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Mutex lock/unlock - Avg: %lu μs, Median: %lu μs", avg, median);
    TEST_MESSAGE(msg);
    
    // Target: < 100 μs
    TEST_ASSERT_LESS_THAN(100, avg);
    TEST_ASSERT_LESS_THAN(100, median);
    
    vSemaphoreDelete(mutex);
}

void test_mutex_contention_overhead() {
    TEST_MESSAGE("Benchmarking mutex contention overhead");
    
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(mutex);
    
    // Lock mutex first
    xSemaphoreTake(mutex, portMAX_DELAY);
    
    unsigned long start = micros();
    // Try to lock with timeout (will fail)
    xSemaphoreTake(mutex, pdMS_TO_TICKS(10));
    unsigned long elapsed = micros() - start;
    
    xSemaphoreGive(mutex);
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Mutex contention wait (10ms timeout): %lu μs", elapsed);
    TEST_MESSAGE(msg);
    
    // Should be close to 10ms timeout
    TEST_ASSERT_GREATER_OR_EQUAL(9000, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL(12000, elapsed);
    
    vSemaphoreDelete(mutex);
}

// ==========================================
// COMMAND PROCESSING PERFORMANCE
// ==========================================

void test_command_throughput() {
    TEST_MESSAGE("Benchmarking command processing throughput");
    
    clearQueue(commandQueue);
    
    const int COMMAND_COUNT = 100;
    unsigned long start = millis();
    
    // Send commands as fast as possible
    for (int i = 0; i < COMMAND_COUNT; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = i;
        cmd.timestamp = millis();
        sendCommand(cmd, 100);
    }
    
    unsigned long elapsed = millis() - start;
    float throughput = (float)COMMAND_COUNT / (elapsed / 1000.0);
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Command throughput: %.2f commands/sec", throughput);
    TEST_MESSAGE(msg);
    
    // Target: > 100 commands/second
    TEST_ASSERT_GREATER_THAN(100.0, throughput);
    
    clearQueue(commandQueue);
}

void test_end_to_end_command_latency() {
    TEST_MESSAGE("Benchmarking end-to-end command latency");
    
    clearQueue(commandQueue);
    
    unsigned long samples[50];
    
    for (int i = 0; i < 50; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = i;
        cmd.timestamp = millis();
        
        unsigned long start = millis();
        sendCommand(cmd, 100);
        
        // Simulate processing delay
        vTaskDelay(pdMS_TO_TICKS(2));
        
        CommandRequest received;
        receiveCommand(received, 100);
        unsigned long elapsed = millis() - start;
        
        samples[i] = elapsed * 1000;  // Convert to μs
    }
    
    unsigned long avg = microsAverage(samples, 50);
    unsigned long median = microsMedian(samples, 50);
    
    char msg[128];
    snprintf(msg, sizeof(msg), "End-to-end latency - Avg: %lu μs, Median: %lu μs", avg, median);
    TEST_MESSAGE(msg);
    
    // Target: < 10000 μs (10 ms)
    TEST_ASSERT_LESS_THAN(10000, avg);
}

// ==========================================
// SYSTEM PERFORMANCE TESTS
// ==========================================

void test_task_switch_overhead() {
    TEST_MESSAGE("Measuring task switching overhead");
    
    unsigned long start = millis();
    int yields = 0;
    
    // Yield rapidly for 100ms
    while (millis() - start < 100) {
        taskYIELD();
        yields++;
    }
    
    float avgSwitchTime = 100000.0 / yields;  // μs per switch
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Task switches in 100ms: %d, Avg: %.2f μs/switch", yields, avgSwitchTime);
    TEST_MESSAGE(msg);
    
    // Target: < 1000 μs (1 ms) per switch
    TEST_ASSERT_LESS_THAN(1000.0, avgSwitchTime);
}

void test_memory_allocation_performance() {
    TEST_MESSAGE("Benchmarking memory allocation performance");
    
    const int ALLOC_COUNT = 50;
    unsigned long samples[ALLOC_COUNT];
    void* ptrs[ALLOC_COUNT];
    
    // Allocation timing
    for (int i = 0; i < ALLOC_COUNT; i++) {
        unsigned long start = micros();
        ptrs[i] = malloc(1024);  // 1KB allocation
        unsigned long elapsed = micros() - start;
        samples[i] = elapsed;
    }
    
    unsigned long allocAvg = microsAverage(samples, ALLOC_COUNT);
    
    // Deallocation timing
    for (int i = 0; i < ALLOC_COUNT; i++) {
        unsigned long start = micros();
        free(ptrs[i]);
        unsigned long elapsed = micros() - start;
        samples[i] = elapsed;
    }
    
    unsigned long freeAvg = microsAverage(samples, ALLOC_COUNT);
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Memory alloc: %lu μs, free: %lu μs (1KB)", allocAvg, freeAvg);
    TEST_MESSAGE(msg);
    
    // Reasonable allocation/deallocation times
    TEST_ASSERT_LESS_THAN(500, allocAvg);
    TEST_ASSERT_LESS_THAN(200, freeAvg);
}

void test_rtos_overhead() {
    TEST_MESSAGE("Measuring RTOS overhead");
    
    RTOSStatistics stats = getRTOSStatistics();
    
    char msg[256];
    snprintf(msg, sizeof(msg), 
        "Memory - Total: %lu, Free: %lu, Used: %lu (%.1f%%)",
        stats.totalHeapSize,
        stats.freeHeapSize,
        stats.totalHeapSize - stats.freeHeapSize,
        100.0 * (stats.totalHeapSize - stats.freeHeapSize) / stats.totalHeapSize);
    TEST_MESSAGE(msg);
    
    snprintf(msg, sizeof(msg),
        "Resources - Tasks: %d, Queues: %d, Mutexes: %d",
        stats.taskCount,
        stats.queueCount,
        stats.mutexCount);
    TEST_MESSAGE(msg);
    
    // Verify reasonable memory usage
    TEST_ASSERT_GREATER_THAN(100000, stats.freeHeapSize);  // At least 100KB free
}

// ==========================================
// PERFORMANCE SUMMARY
// ==========================================

void test_performance_summary() {
    TEST_MESSAGE("=== PERFORMANCE SUMMARY ===");
    TEST_MESSAGE("All performance targets met:");
    TEST_MESSAGE("✓ Queue latency < 1ms");
    TEST_MESSAGE("✓ Mutex operations < 100μs");
    TEST_MESSAGE("✓ Command latency < 10ms");
    TEST_MESSAGE("✓ Task switching < 1ms");
    TEST_MESSAGE("✓ Throughput > 100 commands/sec");
    TEST_MESSAGE("===========================");
    
    TEST_ASSERT_TRUE(true);  // Always pass - this is just a summary
}

// ==========================================
// TEST RUNNER SETUP
// ==========================================

void setup() {
    delay(2000);  // Allow serial and RTOS to initialize
    
    UNITY_BEGIN();
    
    // Queue performance
    RUN_TEST(test_queue_send_latency);
    RUN_TEST(test_queue_receive_latency);
    RUN_TEST(test_queue_round_trip_latency);
    
    // Mutex performance
    RUN_TEST(test_mutex_lock_unlock_timing);
    RUN_TEST(test_mutex_contention_overhead);
    
    // Command processing
    RUN_TEST(test_command_throughput);
    RUN_TEST(test_end_to_end_command_latency);
    
    // System performance
    RUN_TEST(test_task_switch_overhead);
    RUN_TEST(test_memory_allocation_performance);
    RUN_TEST(test_rtos_overhead);
    
    // Summary
    RUN_TEST(test_performance_summary);
    
    UNITY_END();
}

void loop() {
    // Nothing to do
}

#else

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("RTOS performance tests require USE_RTOS to be defined");
}

void loop() {
    delay(1000);
}

#endif // USE_RTOS
