/**
 * @file test_rtos_queues.cpp
 * @brief Comprehensive RTOS Queue Tests
 * 
 * Tests for FreeRTOS queue operations including:
 * - Queue creation and deletion
 * - Send and receive operations
 * - Overflow handling
 * - Timeout behavior
 * - Full/empty conditions
 * - Stress testing
 * 
 * @version 4.1.0
 * @date 2025-10-18
 */

#include <unity.h>
#include <Arduino.h>

#ifdef USE_RTOS
#include "queue_manager.h"
#include "rtos_manager.h"

// ==========================================
// HELPER FUNCTIONS
// ==========================================

/**
 * @brief Clear all items from a queue
 */
void clearQueue(QueueHandle_t queue) {
    if (queue == nullptr) return;
    
    CommandRequest cmd;
    while (xQueueReceive(queue, &cmd, 0) == pdTRUE) {
        // Drain queue
    }
}

// ==========================================
// BASIC QUEUE TESTS
// ==========================================

void test_queue_creation_all() {
    TEST_MESSAGE("Testing queue creation for all system queues");
    
    // Verify all queues are created
    TEST_ASSERT_NOT_NULL(commandQueue);
    TEST_ASSERT_NOT_NULL(wifiEventQueue);
    TEST_ASSERT_NOT_NULL(analysisResultQueue);
    TEST_ASSERT_NOT_NULL(webRequestQueue);
    TEST_ASSERT_NOT_NULL(statusQueue);
    
    // Verify queue count
    TEST_ASSERT_EQUAL(5, getQueueCount());
}

void test_queue_capacity() {
    TEST_MESSAGE("Testing queue capacity limits");
    
    // Command queue should have capacity
    TEST_ASSERT_GREATER_THAN(0, uxQueueSpacesAvailable(commandQueue));
    
    // All queues should start empty
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());
    TEST_ASSERT_EQUAL(0, getPendingWiFiEventCount());
}

void test_queue_send_receive_basic() {
    TEST_MESSAGE("Testing basic send/receive operations");
    
    // Clear queue first
    clearQueue(commandQueue);
    
    // Create test command
    CommandRequest cmd;
    cmd.type = CommandRequest::CommandType::WIFI_SCAN;
    cmd.commandString = "scan";
    cmd.argument = "";
    cmd.requestId = 12345;
    cmd.timestamp = millis();
    
    // Send command
    TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    
    // Verify queue has item
    TEST_ASSERT_EQUAL(1, getPendingCommandCount());
    
    // Receive command
    CommandRequest received;
    TEST_ASSERT_TRUE(receiveCommand(received, 100));
    
    // Verify command data
    TEST_ASSERT_EQUAL(CommandRequest::CommandType::WIFI_SCAN, received.type);
    TEST_ASSERT_EQUAL_STRING("scan", received.commandString.c_str());
    TEST_ASSERT_EQUAL(12345, received.requestId);
    
    // Queue should be empty
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());
}

void test_queue_fifo_order() {
    TEST_MESSAGE("Testing FIFO ordering of queue items");
    
    clearQueue(commandQueue);
    
    // Send multiple commands
    for (int i = 0; i < 5; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = 100 + i;
        cmd.timestamp = millis();
        TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    }
    
    // Receive in order
    for (int i = 0; i < 5; i++) {
        CommandRequest received;
        TEST_ASSERT_TRUE(receiveCommand(received, 100));
        TEST_ASSERT_EQUAL(100 + i, received.requestId);
    }
    
    // Queue should be empty
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());
}

// ==========================================
// OVERFLOW TESTS
// ==========================================

void test_queue_overflow_handling() {
    TEST_MESSAGE("Testing queue overflow handling");
    
    clearQueue(commandQueue);
    
    // Fill queue to capacity (COMMAND_QUEUE_LENGTH = 10)
    int filled = 0;
    for (int i = 0; i < COMMAND_QUEUE_LENGTH; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = i;
        cmd.timestamp = millis();
        if (sendCommand(cmd, 0)) {  // No timeout
            filled++;
        }
    }
    
    TEST_ASSERT_EQUAL(COMMAND_QUEUE_LENGTH, filled);
    TEST_ASSERT_EQUAL(COMMAND_QUEUE_LENGTH, getPendingCommandCount());
    
    // Try to send one more (should fail immediately with 0 timeout)
    CommandRequest extraCmd;
    extraCmd.type = CommandRequest::CommandType::STATUS_REQUEST;
    extraCmd.requestId = 999;
    extraCmd.timestamp = millis();
    TEST_ASSERT_FALSE(sendCommand(extraCmd, 0));
    
    // Drain queue
    clearQueue(commandQueue);
}

void test_queue_overflow_with_timeout() {
    TEST_MESSAGE("Testing queue overflow with timeout");
    
    clearQueue(commandQueue);
    
    // Fill queue
    for (int i = 0; i < COMMAND_QUEUE_LENGTH; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = i;
        TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    }
    
    // Try to send with timeout (should fail after timeout)
    unsigned long start = millis();
    CommandRequest extraCmd;
    extraCmd.type = CommandRequest::CommandType::STATUS_REQUEST;
    TEST_ASSERT_FALSE(sendCommand(extraCmd, 50));  // 50ms timeout
    unsigned long elapsed = millis() - start;
    
    // Should have waited approximately 50ms
    TEST_ASSERT_GREATER_OR_EQUAL(45, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL(100, elapsed);
    
    clearQueue(commandQueue);
}

// ==========================================
// TIMEOUT TESTS
// ==========================================

void test_queue_receive_timeout_empty() {
    TEST_MESSAGE("Testing receive timeout on empty queue");
    
    clearQueue(commandQueue);
    
    // Try to receive from empty queue with timeout
    unsigned long start = millis();
    CommandRequest cmd;
    TEST_ASSERT_FALSE(receiveCommand(cmd, 50));  // 50ms timeout
    unsigned long elapsed = millis() - start;
    
    // Should have waited approximately 50ms
    TEST_ASSERT_GREATER_OR_EQUAL(45, elapsed);
    TEST_ASSERT_LESS_OR_EQUAL(100, elapsed);
}

void test_queue_receive_no_timeout() {
    TEST_MESSAGE("Testing receive with no timeout (immediate return)");
    
    clearQueue(commandQueue);
    
    // Try to receive with 0 timeout (should return immediately)
    unsigned long start = millis();
    CommandRequest cmd;
    TEST_ASSERT_FALSE(receiveCommand(cmd, 0));
    unsigned long elapsed = millis() - start;
    
    // Should return almost immediately
    TEST_ASSERT_LESS_THAN(10, elapsed);
}

// ==========================================
// WIFI EVENT QUEUE TESTS
// ==========================================

void test_wifi_event_queue_operations() {
    TEST_MESSAGE("Testing WiFi event queue operations");
    
    // Create test events
    WiFiEvent event1, event2;
    event1.type = WiFiEvent::EventType::SCAN_STARTED;
    event1.timestamp = millis();
    
    event2.type = WiFiEvent::EventType::SCAN_COMPLETE;
    event2.scanResult.networkCount = 5;
    event2.scanResult.scanDuration = 2500;
    event2.timestamp = millis();
    
    // Send events
    TEST_ASSERT_TRUE(sendWiFiEvent(event1, 100));
    TEST_ASSERT_TRUE(sendWiFiEvent(event2, 100));
    
    // Verify count
    TEST_ASSERT_EQUAL(2, getPendingWiFiEventCount());
    
    // Receive and verify
    WiFiEvent received1, received2;
    TEST_ASSERT_TRUE(receiveWiFiEvent(received1, 100));
    TEST_ASSERT_TRUE(receiveWiFiEvent(received2, 100));
    
    TEST_ASSERT_EQUAL(WiFiEvent::EventType::SCAN_STARTED, received1.type);
    TEST_ASSERT_EQUAL(WiFiEvent::EventType::SCAN_COMPLETE, received2.type);
    TEST_ASSERT_EQUAL(5, received2.scanResult.networkCount);
    TEST_ASSERT_EQUAL(2500, received2.scanResult.scanDuration);
    
    // Queue should be empty
    TEST_ASSERT_EQUAL(0, getPendingWiFiEventCount());
}

// ==========================================
// STATUS QUEUE TESTS
// ==========================================

void test_status_queue_operations() {
    TEST_MESSAGE("Testing status queue operations");
    
    // Create test status
    SystemStatus status;
    status.wifiConnected = true;
    status.apActive = false;
    status.scanningEnabled = false;
    status.timestamp = millis();
    
    // Send status
    TEST_ASSERT_TRUE(sendSystemStatus(status, 100));
    
    // Receive status
    SystemStatus received;
    TEST_ASSERT_TRUE(receiveSystemStatus(received, 100));
    
    // Verify status
    TEST_ASSERT_TRUE(received.wifiConnected);
    TEST_ASSERT_FALSE(received.apActive);
    TEST_ASSERT_FALSE(received.scanningEnabled);
}

// ==========================================
// STRESS TESTS
// ==========================================

void test_queue_rapid_operations() {
    TEST_MESSAGE("Testing rapid queue operations");
    
    clearQueue(commandQueue);
    
    // Rapid send/receive cycles
    for (int cycle = 0; cycle < 10; cycle++) {
        // Fill queue
        for (int i = 0; i < 5; i++) {
            CommandRequest cmd;
            cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
            cmd.requestId = cycle * 100 + i;
            TEST_ASSERT_TRUE(sendCommand(cmd, 100));
        }
        
        // Empty queue
        for (int i = 0; i < 5; i++) {
            CommandRequest received;
            TEST_ASSERT_TRUE(receiveCommand(received, 100));
            TEST_ASSERT_EQUAL(cycle * 100 + i, received.requestId);
        }
    }
    
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());
}

void test_queue_interleaved_operations() {
    TEST_MESSAGE("Testing interleaved send/receive operations");
    
    clearQueue(commandQueue);
    
    // Interleave sends and receives
    for (int i = 0; i < 20; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = i;
        TEST_ASSERT_TRUE(sendCommand(cmd, 100));
        
        if (i % 2 == 1) {  // Receive every other iteration
            CommandRequest r1, r2;
            TEST_ASSERT_TRUE(receiveCommand(r1, 100));
            TEST_ASSERT_TRUE(receiveCommand(r2, 100));
        }
    }
    
    clearQueue(commandQueue);
}

// ==========================================
// MULTI-QUEUE TESTS
// ==========================================

void test_multiple_queues_concurrent() {
    TEST_MESSAGE("Testing multiple queues concurrently");
    
    // Clear all queues
    clearQueue(commandQueue);
    
    // Send to multiple queues simultaneously
    CommandRequest cmd;
    cmd.type = CommandRequest::CommandType::WIFI_SCAN;
    cmd.requestId = 1;
    TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    
    WiFiEvent event;
    event.type = WiFiEvent::EventType::SCAN_STARTED;
    event.timestamp = millis();
    TEST_ASSERT_TRUE(sendWiFiEvent(event, 100));
    
    SystemStatus status;
    status.wifiConnected = false;
    status.timestamp = millis();
    TEST_ASSERT_TRUE(sendSystemStatus(status, 100));
    
    // Verify all queues have items
    TEST_ASSERT_EQUAL(1, getPendingCommandCount());
    TEST_ASSERT_EQUAL(1, getPendingWiFiEventCount());
    
    // Receive from all queues
    CommandRequest rcmd;
    TEST_ASSERT_TRUE(receiveCommand(rcmd, 100));
    
    WiFiEvent revent;
    TEST_ASSERT_TRUE(receiveWiFiEvent(revent, 100));
    
    SystemStatus rstatus;
    TEST_ASSERT_TRUE(receiveSystemStatus(rstatus, 100));
    
    // All queues should be empty
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());
    TEST_ASSERT_EQUAL(0, getPendingWiFiEventCount());
}

// ==========================================
// TEST RUNNER SETUP
// ==========================================

void setup() {
    delay(2000);  // Allow serial to initialize
    
    UNITY_BEGIN();
    
    // Basic tests
    RUN_TEST(test_queue_creation_all);
    RUN_TEST(test_queue_capacity);
    RUN_TEST(test_queue_send_receive_basic);
    RUN_TEST(test_queue_fifo_order);
    
    // Overflow tests
    RUN_TEST(test_queue_overflow_handling);
    RUN_TEST(test_queue_overflow_with_timeout);
    
    // Timeout tests
    RUN_TEST(test_queue_receive_timeout_empty);
    RUN_TEST(test_queue_receive_no_timeout);
    
    // Specific queue tests
    RUN_TEST(test_wifi_event_queue_operations);
    RUN_TEST(test_status_queue_operations);
    
    // Stress tests
    RUN_TEST(test_queue_rapid_operations);
    RUN_TEST(test_queue_interleaved_operations);
    RUN_TEST(test_multiple_queues_concurrent);
    
    UNITY_END();
}

void loop() {
    // Nothing to do
}

#else

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("RTOS queue tests require USE_RTOS to be defined");
}

void loop() {
    delay(1000);
}

#endif // USE_RTOS
