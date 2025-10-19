/**
 * @file test_rtos_integration.cpp
 * @brief RTOS Integration Tests
 * 
 * Tests for inter-task communication and workflow integration:
 * - Command -> WiFi -> LED workflow
 * - Command -> Analysis workflow
 * - Web -> WiFi -> LED workflow
 * - Concurrent multi-task operations
 * - Queue chaining
 * 
 * @version 4.1.0
 * @date 2025-10-18
 */

#include <unity.h>
#include <Arduino.h>

#ifdef USE_RTOS
#include "rtos_manager.h"
#include "queue_manager.h"
#include "command_task.h"
#include "wifi_task.h"
#include "led_task.h"

// ==========================================
// INTEGRATION TEST HELPERS
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
// BASIC INTEGRATION TESTS
// ==========================================

void test_command_to_wifi_flow() {
    TEST_MESSAGE("Testing Command -> WiFi event flow");
    
    clearAllQueues();
    
    // Send a command
    CommandRequest cmd;
    cmd.type = CommandRequest::CommandType::WIFI_SCAN;
    cmd.commandString = "scan";
    cmd.requestId = 1001;
    cmd.timestamp = millis();
    
    TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    
    // Give system time to process
    delay(200);
    
    // Should generate WiFi events
    // Note: Actual event generation depends on WiFi task implementation
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());  // Command should be consumed
}

void test_wifi_event_to_led_flow() {
    TEST_MESSAGE("Testing WiFi event -> LED state flow");
    
    clearAllQueues();
    
    // Send WiFi event
    WiFiEvent event;
    event.type = WiFiEvent::EventType::SCAN_STARTED;
    event.timestamp = millis();
    
    TEST_ASSERT_TRUE(sendWiFiEvent(event, 100));
    
    // LED task should react to WiFi events
    delay(100);
    
    // Event should be consumed
    TEST_ASSERT_EQUAL(0, getPendingWiFiEventCount());
}

void test_status_queue_integration() {
    TEST_MESSAGE("Testing system status queue integration");
    
    clearAllQueues();
    
    // Create and send status
    SystemStatus status;
    status.wifiConnected = true;
    status.apActive = false;
    status.scanningEnabled = true;
    status.timestamp = millis();
    
    TEST_ASSERT_TRUE(sendSystemStatus(status, 100));
    
    // Status should be available
    TEST_ASSERT_GREATER_THAN(0, uxQueueMessagesWaiting(statusQueue));
    
    // Consume status
    SystemStatus received;
    TEST_ASSERT_TRUE(receiveSystemStatus(received, 100));
    TEST_ASSERT_TRUE(received.wifiConnected);
    TEST_ASSERT_TRUE(received.scanningEnabled);
}

// ==========================================
// MULTI-QUEUE TESTS
// ==========================================

void test_queue_chaining() {
    TEST_MESSAGE("Testing queue chaining (command -> event -> status)");
    
    clearAllQueues();
    
    // Step 1: Send command
    CommandRequest cmd;
    cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
    cmd.requestId = 2001;
    cmd.timestamp = millis();
    TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    
    // Step 2: Generate WiFi event
    delay(50);
    WiFiEvent event;
    event.type = WiFiEvent::EventType::SCAN_STARTED;
    event.timestamp = millis();
    TEST_ASSERT_TRUE(sendWiFiEvent(event, 100));
    
    // Step 3: Generate status update
    delay(50);
    SystemStatus status;
    status.wifiConnected = false;
    status.scanningEnabled = true;
    status.timestamp = millis();
    TEST_ASSERT_TRUE(sendSystemStatus(status, 100));
    
    // Verify all queues have items or were processed
    delay(100);
    
    // System should be stable
    TEST_ASSERT_TRUE(isRTOSRunning());
}

void test_concurrent_queue_operations() {
    TEST_MESSAGE("Testing concurrent operations on multiple queues");
    
    clearAllQueues();
    
    // Send to all queues simultaneously
    CommandRequest cmd;
    cmd.type = CommandRequest::CommandType::HELP_REQUEST;
    cmd.requestId = 3001;
    TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    
    WiFiEvent event;
    event.type = WiFiEvent::EventType::CONNECTED;
    TEST_ASSERT_TRUE(sendWiFiEvent(event, 100));
    
    SystemStatus status;
    status.wifiConnected = true;
    TEST_ASSERT_TRUE(sendSystemStatus(status, 100));
    
    // All queues should have messages
    TEST_ASSERT_GREATER_THAN(0, getPendingCommandCount());
    TEST_ASSERT_GREATER_THAN(0, getPendingWiFiEventCount());
    TEST_ASSERT_GREATER_THAN(0, uxQueueMessagesWaiting(statusQueue));
    
    clearAllQueues();
}

// ==========================================
// TASK INTERACTION TESTS
// ==========================================

void test_system_tasks_running_together() {
    TEST_MESSAGE("Testing all system tasks running concurrently");
    
    // Get RTOS statistics
    RTOSStatistics stats = getRTOSStatistics();
    
    // Multiple tasks should be running
    TEST_ASSERT_GREATER_THAN(4, stats.taskCount);
    
    // System should be healthy
    TEST_ASSERT_TRUE(checkRTOSHealth());
    
    // All queues should be operational
    TEST_ASSERT_EQUAL(5, getQueueCount());
    TEST_ASSERT_EQUAL(4, getMutexCount());
}

void test_command_processing_workflow() {
    TEST_MESSAGE("Testing complete command processing workflow");
    
    clearAllQueues();
    
    // Send multiple different commands
    const char* commands[] = {"status", "help", "scan"};
    const CommandRequest::CommandType types[] = {
        CommandRequest::CommandType::STATUS_REQUEST,
        CommandRequest::CommandType::HELP_REQUEST,
        CommandRequest::CommandType::WIFI_SCAN
    };
    
    for (int i = 0; i < 3; i++) {
        CommandRequest cmd;
        cmd.type = types[i];
        cmd.commandString = commands[i];
        cmd.requestId = 4000 + i;
        cmd.timestamp = millis();
        
        TEST_ASSERT_TRUE(sendCommand(cmd, 100));
        delay(50);  // Give time to process
    }
    
    // All commands should be processed
    delay(200);
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());
}

// ==========================================
// MEMORY AND RESOURCE TESTS
// ==========================================

void test_memory_stability_during_operations() {
    TEST_MESSAGE("Testing memory stability during queue operations");
    
    RTOSStatistics statsBefore = getRTOSStatistics();
    
    // Perform many queue operations
    for (int i = 0; i < 50; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = 5000 + i;
        sendCommand(cmd, 10);
        
        CommandRequest received;
        receiveCommand(received, 10);
    }
    
    RTOSStatistics statsAfter = getRTOSStatistics();
    
    // Memory should be relatively stable (some variation is ok)
    int memoryChange = statsBefore.freeHeapSize - statsAfter.freeHeapSize;
    TEST_ASSERT_LESS_THAN(1000, abs(memoryChange));  // Less than 1KB change
}

void test_no_queue_leaks() {
    TEST_MESSAGE("Testing for queue memory leaks");
    
    clearAllQueues();
    
    // Fill and drain queues multiple times
    for (int cycle = 0; cycle < 10; cycle++) {
        // Fill command queue
        for (int i = 0; i < 5; i++) {
            CommandRequest cmd;
            cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
            cmd.requestId = 6000 + cycle * 10 + i;
            sendCommand(cmd, 100);
        }
        
        // Drain command queue
        for (int i = 0; i < 5; i++) {
            CommandRequest cmd;
            receiveCommand(cmd, 100);
        }
    }
    
    // Queue should be empty and functional
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());
    
    // Should still be able to send/receive
    CommandRequest testCmd;
    testCmd.type = CommandRequest::CommandType::STATUS_REQUEST;
    testCmd.requestId = 9999;
    TEST_ASSERT_TRUE(sendCommand(testCmd, 100));
    
    CommandRequest received;
    TEST_ASSERT_TRUE(receiveCommand(received, 100));
    TEST_ASSERT_EQUAL(9999, received.requestId);
}

// ==========================================
// ERROR HANDLING TESTS
// ==========================================

void test_graceful_queue_overflow_handling() {
    TEST_MESSAGE("Testing graceful handling of queue overflow scenarios");
    
    clearAllQueues();
    
    // Fill command queue to capacity
    for (int i = 0; i < COMMAND_QUEUE_LENGTH; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = 7000 + i;
        sendCommand(cmd, 100);
    }
    
    // Try to send more (should handle gracefully)
    CommandRequest extraCmd;
    extraCmd.type = CommandRequest::CommandType::STATUS_REQUEST;
    extraCmd.requestId = 7999;
    bool result = sendCommand(extraCmd, 0);  // No timeout
    
    // Should fail gracefully without crashing
    TEST_ASSERT_FALSE(result);
    
    // System should still be stable
    TEST_ASSERT_TRUE(isRTOSRunning());
    TEST_ASSERT_TRUE(checkRTOSHealth());
    
    clearAllQueues();
}

void test_system_recovery_after_errors() {
    TEST_MESSAGE("Testing system recovery after error conditions");
    
    clearAllQueues();
    
    // Cause some error conditions
    for (int i = 0; i < 3; i++) {
        // Try to overflow queue
        for (int j = 0; j < COMMAND_QUEUE_LENGTH + 5; j++) {
            CommandRequest cmd;
            cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
            sendCommand(cmd, 0);
        }
        
        // Clear and try again
        clearAllQueues();
        delay(50);
    }
    
    // System should recover and be functional
    TEST_ASSERT_TRUE(isRTOSRunning());
    
    // Should be able to send/receive normally
    CommandRequest cmd;
    cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
    cmd.requestId = 8888;
    TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    
    CommandRequest received;
    TEST_ASSERT_TRUE(receiveCommand(received, 100));
    TEST_ASSERT_EQUAL(8888, received.requestId);
}

// ==========================================
// TEST RUNNER SETUP
// ==========================================

void setup() {
    delay(2000);  // Allow serial and RTOS to initialize
    
    UNITY_BEGIN();
    
    // Basic integration
    RUN_TEST(test_command_to_wifi_flow);
    RUN_TEST(test_wifi_event_to_led_flow);
    RUN_TEST(test_status_queue_integration);
    
    // Multi-queue tests
    RUN_TEST(test_queue_chaining);
    RUN_TEST(test_concurrent_queue_operations);
    
    // Task interaction
    RUN_TEST(test_system_tasks_running_together);
    RUN_TEST(test_command_processing_workflow);
    
    // Memory and resources
    RUN_TEST(test_memory_stability_during_operations);
    RUN_TEST(test_no_queue_leaks);
    
    // Error handling
    RUN_TEST(test_graceful_queue_overflow_handling);
    RUN_TEST(test_system_recovery_after_errors);
    
    UNITY_END();
}

void loop() {
    // Nothing to do
}

#else

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("RTOS integration tests require USE_RTOS to be defined");
}

void loop() {
    delay(1000);
}

#endif // USE_RTOS
