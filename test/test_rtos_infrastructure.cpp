#include <unity.h>
#include <Arduino.h>

#ifdef USE_RTOS
#include "rtos_manager.h"
#include "queue_manager.h"
#include "mutex_manager.h"
#include "event_manager.h"

// ==========================================
// RTOS MANAGER TESTS
// ==========================================

void test_rtos_initialization() {
    // RTOS should already be initialized in setup()
    TEST_ASSERT_TRUE(isRTOSRunning());
    TEST_ASSERT_EQUAL(RTOSState::RUNNING, getRTOSState());
}

void test_rtos_statistics() {
    RTOSStatistics stats = getRTOSStatistics();
    
    // Check memory stats
    TEST_ASSERT_GREATER_THAN(0, stats.totalHeapSize);
    TEST_ASSERT_GREATER_THAN(0, stats.freeHeapSize);
    TEST_ASSERT_LESS_THAN(stats.totalHeapSize, stats.freeHeapSize);
    
    // Check resource counts
    TEST_ASSERT_EQUAL(5, stats.queueCount);   // 5 queues
    TEST_ASSERT_EQUAL(4, stats.mutexCount);   // 4 mutexes
}

void test_rtos_health_check() {
    TEST_ASSERT_TRUE(checkRTOSHealth());
}

// ==========================================
// QUEUE MANAGER TESTS
// ==========================================

void test_queue_creation() {
    // All queues should be created
    TEST_ASSERT_EQUAL(5, getQueueCount());
    TEST_ASSERT_NOT_NULL(commandQueue);
    TEST_ASSERT_NOT_NULL(wifiEventQueue);
    TEST_ASSERT_NOT_NULL(analysisResultQueue);
    TEST_ASSERT_NOT_NULL(webRequestQueue);
    TEST_ASSERT_NOT_NULL(statusQueue);
}

void test_command_queue() {
    // Create a test command
    CommandRequest cmd;
    cmd.type = CommandRequest::CommandType::WIFI_SCAN;
    cmd.commandString = "scan";
    cmd.argument = "";
    cmd.requestId = 1;
    cmd.timestamp = millis();
    
    // Send command
    TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    
    // Check pending count
    TEST_ASSERT_EQUAL(1, getPendingCommandCount());
    
    // Receive command
    CommandRequest receivedCmd;
    TEST_ASSERT_TRUE(receiveCommand(receivedCmd, 100));
    
    // Verify command
    TEST_ASSERT_EQUAL(CommandRequest::CommandType::WIFI_SCAN, receivedCmd.type);
    TEST_ASSERT_EQUAL_STRING("scan", receivedCmd.commandString.c_str());
    TEST_ASSERT_EQUAL(1, receivedCmd.requestId);
    
    // Queue should be empty now
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());
}

void test_wifi_event_queue() {
    // Create a test event
    WiFiEvent event;
    event.type = WiFiEvent::EventType::SCAN_STARTED;
    event.timestamp = millis();
    
    // Send event
    TEST_ASSERT_TRUE(sendWiFiEvent(event, 100));
    
    // Check pending count
    TEST_ASSERT_EQUAL(1, getPendingWiFiEventCount());
    
    // Receive event
    WiFiEvent receivedEvent;
    TEST_ASSERT_TRUE(receiveWiFiEvent(receivedEvent, 100));
    
    // Verify event
    TEST_ASSERT_EQUAL(WiFiEvent::EventType::SCAN_STARTED, receivedEvent.type);
    
    // Queue should be empty now
    TEST_ASSERT_EQUAL(0, getPendingWiFiEventCount());
}

void test_status_queue() {
    // Create a test status
    StatusUpdate status;
    status.component = StatusUpdate::ComponentType::WIFI;
    status.message = "Test status";
    status.level = StatusUpdate::Level::INFO;
    status.timestamp = millis();
    
    // Send status
    TEST_ASSERT_TRUE(sendStatusUpdate(status, 100));
    
    // Check pending count
    TEST_ASSERT_EQUAL(1, getPendingStatusCount());
    
    // Receive status
    StatusUpdate receivedStatus;
    TEST_ASSERT_TRUE(receiveStatusUpdate(receivedStatus, 100));
    
    // Verify status
    TEST_ASSERT_EQUAL(StatusUpdate::ComponentType::WIFI, receivedStatus.component);
    TEST_ASSERT_EQUAL_STRING("Test status", receivedStatus.message.c_str());
    TEST_ASSERT_EQUAL(StatusUpdate::Level::INFO, receivedStatus.level);
    
    // Queue should be empty now
    TEST_ASSERT_EQUAL(0, getPendingStatusCount());
}

void test_queue_overflow() {
    // Fill command queue to capacity
    for (int i = 0; i < COMMAND_QUEUE_LENGTH; i++) {
        CommandRequest cmd;
        cmd.type = CommandRequest::CommandType::STATUS_REQUEST;
        cmd.requestId = i;
        TEST_ASSERT_TRUE(sendCommand(cmd, 100));
    }
    
    // Queue should be full now
    TEST_ASSERT_EQUAL(COMMAND_QUEUE_LENGTH, getPendingCommandCount());
    
    // Try to send one more (should fail)
    CommandRequest extraCmd;
    extraCmd.type = CommandRequest::CommandType::HELP_REQUEST;
    TEST_ASSERT_FALSE(sendCommand(extraCmd, 10));  // Short timeout
    
    // Clear queue
    flushAllQueues();
    TEST_ASSERT_EQUAL(0, getPendingCommandCount());
}

// ==========================================
// MUTEX MANAGER TESTS
// ==========================================

void test_mutex_creation() {
    // All mutexes should be created
    TEST_ASSERT_EQUAL(4, getMutexCount());
    TEST_ASSERT_NOT_NULL(wifiMutex);
    TEST_ASSERT_NOT_NULL(webServerMutex);
    TEST_ASSERT_NOT_NULL(scanResultsMutex);
    TEST_ASSERT_NOT_NULL(serialMutex);
}

void test_mutex_lock_unlock() {
    // Test WiFi mutex
    TEST_ASSERT_TRUE(lockWiFiMutex(100));
    TEST_ASSERT_TRUE(unlockWiFiMutex());
    
    // Test Web Server mutex
    TEST_ASSERT_TRUE(lockWebServerMutex(100));
    TEST_ASSERT_TRUE(unlockWebServerMutex());
    
    // Test Scan Results mutex
    TEST_ASSERT_TRUE(lockScanResultsMutex(100));
    TEST_ASSERT_TRUE(unlockScanResultsMutex());
    
    // Test Serial mutex
    TEST_ASSERT_TRUE(lockSerialMutex(100));
    TEST_ASSERT_TRUE(unlockSerialMutex());
}

void test_mutex_raii() {
    // Test RAII lock guard
    {
        MutexLock lock(wifiMutex, "Test RAII");
        TEST_ASSERT_TRUE(lock.isLocked());
        // Mutex should be locked here
    }
    // Mutex should be automatically unlocked after scope exit
    
    // Verify we can lock it again
    TEST_ASSERT_TRUE(lockWiFiMutex(100));
    TEST_ASSERT_TRUE(unlockWiFiMutex());
}

void test_mutex_timeout() {
    // Lock WiFi mutex
    TEST_ASSERT_TRUE(lockWiFiMutex(100));
    
    // Try to lock again with short timeout (should fail)
    TEST_ASSERT_FALSE(lockMutex(wifiMutex, 10));
    
    // Unlock
    TEST_ASSERT_TRUE(unlockWiFiMutex());
    
    // Should be able to lock again
    TEST_ASSERT_TRUE(lockWiFiMutex(100));
    TEST_ASSERT_TRUE(unlockWiFiMutex());
}

// ==========================================
// EVENT MANAGER TESTS
// ==========================================

void test_event_manager_initialization() {
    TEST_ASSERT_TRUE(isEventManagerInitialized());
}

void test_event_bits_set_clear() {
    // Clear all bits first
    clearAllEventBits();
    TEST_ASSERT_EQUAL(0, getEventBits());
    
    // Set WiFi connected bit
    setEventBits(WIFI_CONNECTED_BIT);
    TEST_ASSERT_TRUE(isEventBitSet(WIFI_CONNECTED_BIT));
    TEST_ASSERT_TRUE(isWiFiConnectedEvent());
    
    // Clear WiFi connected bit
    clearEventBits(WIFI_CONNECTED_BIT);
    TEST_ASSERT_FALSE(isEventBitSet(WIFI_CONNECTED_BIT));
    TEST_ASSERT_FALSE(isWiFiConnectedEvent());
}

void test_multiple_event_bits() {
    // Clear all bits
    clearAllEventBits();
    
    // Set multiple bits
    setEventBits(WIFI_CONNECTED_BIT | WEB_SERVER_RUNNING_BIT);
    
    // Check both bits are set
    TEST_ASSERT_TRUE(isEventBitSet(WIFI_CONNECTED_BIT));
    TEST_ASSERT_TRUE(isEventBitSet(WEB_SERVER_RUNNING_BIT));
    TEST_ASSERT_FALSE(isEventBitSet(ANALYSIS_RUNNING_BIT));
    
    // Check any bits
    TEST_ASSERT_TRUE(isAnyEventBitSet(WIFI_CONNECTED_BIT | ANALYSIS_RUNNING_BIT));
    
    // Clear all
    clearAllEventBits();
    TEST_ASSERT_EQUAL(0, getEventBits());
}

void test_wifi_events() {
    clearAllEventBits();
    
    // Test WiFi connected
    signalWiFiConnected();
    TEST_ASSERT_TRUE(isWiFiConnectedEvent());
    
    signalWiFiDisconnected();
    TEST_ASSERT_FALSE(isWiFiConnectedEvent());
    
    // Test WiFi scanning
    signalWiFiScanStarted();
    TEST_ASSERT_TRUE(isWiFiScanningEvent());
    
    signalWiFiScanCompleted();
    TEST_ASSERT_FALSE(isWiFiScanningEvent());
}

void test_analysis_events() {
    clearAllEventBits();
    
    signalAnalysisStarted();
    TEST_ASSERT_TRUE(isAnalysisRunningEvent());
    
    signalAnalysisCompleted();
    TEST_ASSERT_FALSE(isAnalysisRunningEvent());
}

void test_ap_mode_events() {
    clearAllEventBits();
    
    signalAPModeStarted();
    TEST_ASSERT_TRUE(isAPModeActiveEvent());
    
    signalAPModeStopped();
    TEST_ASSERT_FALSE(isAPModeActiveEvent());
}

// ==========================================
// TEST RUNNER
// ==========================================

void setup() {
    delay(2000);  // Wait for serial
    
    UNITY_BEGIN();
    
    Serial.println("\n=== RTOS Infrastructure Tests ===\n");
    
    // Initialize RTOS before running tests
    if (!initializeRTOS()) {
        Serial.println("FATAL: RTOS initialization failed!");
        UNITY_END();
        return;
    }
    
    // RTOS Manager Tests
    Serial.println("--- RTOS Manager Tests ---");
    RUN_TEST(test_rtos_initialization);
    RUN_TEST(test_rtos_statistics);
    RUN_TEST(test_rtos_health_check);
    
    // Queue Manager Tests
    Serial.println("\n--- Queue Manager Tests ---");
    RUN_TEST(test_queue_creation);
    RUN_TEST(test_command_queue);
    RUN_TEST(test_wifi_event_queue);
    RUN_TEST(test_status_queue);
    RUN_TEST(test_queue_overflow);
    
    // Mutex Manager Tests
    Serial.println("\n--- Mutex Manager Tests ---");
    RUN_TEST(test_mutex_creation);
    RUN_TEST(test_mutex_lock_unlock);
    RUN_TEST(test_mutex_raii);
    RUN_TEST(test_mutex_timeout);
    
    // Event Manager Tests
    Serial.println("\n--- Event Manager Tests ---");
    RUN_TEST(test_event_manager_initialization);
    RUN_TEST(test_event_bits_set_clear);
    RUN_TEST(test_multiple_event_bits);
    RUN_TEST(test_wifi_events);
    RUN_TEST(test_analysis_events);
    RUN_TEST(test_ap_mode_events);
    
    Serial.println("\n=== Test Summary ===");
    UNITY_END();
    
    // Print final statistics
    Serial.println("\n=== Final RTOS Statistics ===");
    printRTOSStatistics();
    printQueueStatistics();
    printMutexStatistics();
    printEventBits();
}

void loop() {
    // Tests run once in setup()
}

#else // !USE_RTOS

void setup() {
    delay(2000);
    UNITY_BEGIN();
    Serial.println("RTOS tests skipped - USE_RTOS not defined");
    UNITY_END();
}

void loop() {}

#endif // USE_RTOS
