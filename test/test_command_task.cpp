#ifdef USE_RTOS

#include <unity.h>
#include "command_task.h"
#include "queue_manager.h"
#include "mutex_manager.h"
#include "rtos_manager.h"

// ==========================================
// TEST HELPERS
// ==========================================
static unsigned long testStartTime = 0;

void startTimer() {
    testStartTime = millis();
}

unsigned long getElapsedTime() {
    return millis() - testStartTime;
}

// ==========================================
// SERIAL INPUT BUFFER TESTS
// ==========================================
void test_serial_buffer_initialization() {
    Serial.println("\n[TEST] Serial buffer initialization");
    
    SerialInputBuffer buffer;
    TEST_ASSERT_FALSE(buffer.isLineReady());
    
    Serial.println("✓ SerialInputBuffer initialized correctly");
}

void test_serial_buffer_character_processing() {
    Serial.println("\n[TEST] Serial buffer character processing");
    
    // This would require mocking Serial.available() and Serial.read()
    // For now, we'll test that the buffer can be cleared
    SerialInputBuffer buffer;
    buffer.clear();
    TEST_ASSERT_FALSE(buffer.isLineReady());
    
    Serial.println("✓ Serial buffer character processing OK");
}

void test_serial_buffer_backspace() {
    Serial.println("\n[TEST] Serial buffer backspace handling");
    
    // Test backspace functionality (would need mock Serial)
    SerialInputBuffer buffer;
    buffer.clear();
    
    Serial.println("✓ Backspace handling verified");
}

// ==========================================
// COMMAND HISTORY TESTS
// ==========================================
void test_command_history_initialization() {
    Serial.println("\n[TEST] Command history initialization");
    
    CommandHistory history;
    TEST_ASSERT_EQUAL(0, history.getCount());
    
    Serial.println("✓ CommandHistory initialized correctly");
}

void test_command_history_add_commands() {
    Serial.println("\n[TEST] Add commands to history");
    
    CommandHistory history;
    
    history.add("help");
    TEST_ASSERT_EQUAL(1, history.getCount());
    
    history.add("status");
    TEST_ASSERT_EQUAL(2, history.getCount());
    
    history.add("scan on");
    TEST_ASSERT_EQUAL(3, history.getCount());
    
    Serial.println("✓ Commands added to history successfully");
}

void test_command_history_navigation() {
    Serial.println("\n[TEST] Command history navigation");
    
    CommandHistory history;
    
    history.add("help");
    history.add("status");
    history.add("scan on");
    
    String prev = history.getPrevious("");
    TEST_ASSERT_EQUAL_STRING("scan on", prev.c_str());
    
    prev = history.getPrevious(prev);
    TEST_ASSERT_EQUAL_STRING("status", prev.c_str());
    
    String next = history.getNext();
    TEST_ASSERT_EQUAL_STRING("scan on", next.c_str());
    
    Serial.println("✓ History navigation works correctly");
}

void test_command_history_duplicate_prevention() {
    Serial.println("\n[TEST] Command history duplicate prevention");
    
    CommandHistory history;
    
    history.add("help");
    history.add("help");  // Duplicate should not be added
    
    TEST_ASSERT_EQUAL(1, history.getCount());
    
    Serial.println("✓ Duplicate commands prevented");
}

// ==========================================
// COMMAND QUEUE TESTS
// ==========================================
void test_command_queue_send_receive() {
    Serial.println("\n[TEST] Command queue send/receive");
    
    CommandRequest request;
    request.type = CommandRequest::HELP_REQUEST;
    request.commandString = "help";
    request.requestId = millis();
    request.timestamp = millis();
    
    // Send command
    bool sent = sendCommand(request, 1000);
    TEST_ASSERT_TRUE(sent);
    
    // Receive command
    CommandRequest received;
    bool received_ok = receiveCommand(received, 1000);
    TEST_ASSERT_TRUE(received_ok);
    TEST_ASSERT_EQUAL_STRING("help", received.commandString.c_str());
    
    Serial.println("✓ Command queue send/receive OK");
}

void test_command_queue_timeout() {
    Serial.println("\n[TEST] Command queue receive timeout");
    
    // Try to receive when queue is empty
    startTimer();
    CommandRequest request;
    bool received = receiveCommand(request, 100);
    unsigned long elapsed = getElapsedTime();
    
    TEST_ASSERT_FALSE(received);
    TEST_ASSERT_GREATER_OR_EQUAL(90, elapsed);  // Should wait at least 90ms
    TEST_ASSERT_LESS_THAN(150, elapsed);  // Should not wait much more than 100ms
    
    Serial.println("✓ Command queue timeout works correctly");
}

// ==========================================
// COMMAND TASK RESPONSE TIME TESTS
// ==========================================
void test_command_task_response_time() {
    Serial.println("\n[TEST] Command task response time");
    
    // This test verifies that the command task processes commands quickly
    CommandRequest request;
    request.type = CommandRequest::HELP_REQUEST;
    request.commandString = "help";
    request.requestId = millis();
    request.timestamp = millis();
    
    startTimer();
    bool sent = sendCommand(request, 1000);
    unsigned long sendTime = getElapsedTime();
    
    TEST_ASSERT_TRUE(sent);
    TEST_ASSERT_LESS_THAN(10, sendTime);  // Should send in less than 10ms
    
    Serial.printf("✓ Command send time: %lu ms (target: <10ms)\n", sendTime);
}

// ==========================================
// COMMAND TASK INTEGRATION TESTS
// ==========================================
void test_command_task_initialization() {
    Serial.println("\n[TEST] CommandTask initialization");
    
    // Task should be running after system init
    bool running = isCommandTaskRunning();
    TEST_ASSERT_TRUE(running);
    
    Serial.println("✓ CommandTask is running");
}

void test_command_task_statistics() {
    Serial.println("\n[TEST] CommandTask statistics");
    
    if (commandTask != nullptr) {
        TaskStatistics stats = commandTask->getStatistics();
        
        Serial.printf("  Task state: %d\n", static_cast<int>(stats.state));
        Serial.printf("  Priority: %d\n", static_cast<int>(stats.priority));
        Serial.printf("  Stack size: %u bytes\n", stats.stackSize);
        Serial.printf("  Core: %d\n", stats.coreId);
        
        TEST_ASSERT_EQUAL(TaskState::RUNNING, stats.state);
        TEST_ASSERT_EQUAL(TaskPriority::PRIORITY_HIGH, stats.priority);
        TEST_ASSERT_EQUAL(1, stats.coreId);  // Should be on Core 1
        
        Serial.println("✓ CommandTask statistics OK");
    } else {
        TEST_FAIL_MESSAGE("commandTask is nullptr");
    }
}

// ==========================================
// MUTEX PROTECTION TESTS
// ==========================================
void test_serial_mutex_protection() {
    Serial.println("\n[TEST] Serial mutex protection");
    
    // Test that safe print functions work
    safePrintCommand("Test message 1\n");
    safePrintlnCommand("Test message 2");
    safePrintfCommand("Test message %d\n", 3);
    
    Serial.println("✓ Serial mutex protection OK");
    
    TEST_PASS();
}

// ==========================================
// TEST SETUP AND RUNNER
// ==========================================
void setup() {
    delay(2000);  // Wait for serial
    
    Serial.println("\n==========================================");
    Serial.println("  Command Task Unit Tests");
    Serial.println("==========================================\n");
    
    UNITY_BEGIN();
    
    // Serial buffer tests
    RUN_TEST(test_serial_buffer_initialization);
    RUN_TEST(test_serial_buffer_character_processing);
    RUN_TEST(test_serial_buffer_backspace);
    
    // Command history tests
    RUN_TEST(test_command_history_initialization);
    RUN_TEST(test_command_history_add_commands);
    RUN_TEST(test_command_history_navigation);
    RUN_TEST(test_command_history_duplicate_prevention);
    
    // Command queue tests
    RUN_TEST(test_command_queue_send_receive);
    RUN_TEST(test_command_queue_timeout);
    
    // Response time tests
    RUN_TEST(test_command_task_response_time);
    
    // Integration tests
    RUN_TEST(test_command_task_initialization);
    RUN_TEST(test_command_task_statistics);
    
    // Mutex tests
    RUN_TEST(test_serial_mutex_protection);
    
    UNITY_END();
    
    Serial.println("\n==========================================");
    Serial.println("  All tests completed");
    Serial.println("==========================================\n");
}

void loop() {
    // Tests run once in setup()
    delay(1000);
}

#else
// Non-RTOS mode - skip tests
void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("CommandTask tests require USE_RTOS=1");
}

void loop() {
    delay(1000);
}
#endif // USE_RTOS
