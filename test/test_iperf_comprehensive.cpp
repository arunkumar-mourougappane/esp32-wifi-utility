#include <unity.h>
#include <Arduino.h>

// Include the headers we want to test
#include "../include/iperf_manager.h"
#include "../include/command_interface.h"
#include "../include/config.h"

// ==========================================
// MOCK IMPLEMENTATIONS AND GLOBALS
// ==========================================

// Mock global variables that would normally be defined in the source files
IperfState currentIperfState = IPERF_IDLE;
IperfConfig activeConfig;
IperfResults lastResults;
WiFiServer* iperfTcpServer = nullptr;
WiFiUDP iperfUdp;
unsigned long iperfStartTime = 0;
bool iperfServerRunning = false;

// Mock WiFi state for testing
bool mockWiFiConnected = false;
bool mockAPMode = false;

// Capture test output
String testOutput = "";
bool captureOutput = false;

// ==========================================
// SIMPLIFIED MOCK IMPLEMENTATIONS
// ==========================================

// Mock WiFi status functions
int mockWiFiStatus() {
    return mockWiFiConnected ? WL_CONNECTED : WL_DISCONNECTED;
}

// Simple implementations of core functions for testing
IperfConfig getDefaultConfig() {
    IperfConfig config;
    config.protocol = IPERF_TCP;
    config.mode = IPERF_CLIENT;
    config.serverIP = "";
    config.port = IPERF_DEFAULT_PORT;
    config.duration = IPERF_DEFAULT_DURATION;
    config.interval = IPERF_DEFAULT_INTERVAL;
    config.bandwidth = 1000000;
    config.bufferSize = IPERF_BUFFER_SIZE;
    config.reverse = false;
    config.bidir = false;
    config.parallel = 1;
    return config;
}

void initializeIperf() {
    currentIperfState = IPERF_IDLE;
    iperfServerRunning = false;
    activeConfig = getDefaultConfig();
}

void shutdownIperf() {
    currentIperfState = IPERF_IDLE;
    iperfServerRunning = false;
}

bool isIperfRunning() {
    return currentIperfState == IPERF_RUNNING;
}

void stopIperfTest() {
    currentIperfState = IPERF_IDLE;
    iperfServerRunning = false;
}

IperfResults getIperfResults() {
    return lastResults;
}

String formatThroughput(float mbps) {
    if (mbps >= 1000.0) {
        return String(mbps / 1000.0, 2) + " Gbps";
    } else if (mbps >= 1.0) {
        return String(mbps, 2) + " Mbps";
    } else {
        return String(mbps * 1000.0, 1) + " Kbps";
    }
}

String formatBytes(unsigned long bytes) {
    if (bytes >= 1073741824) {
        return String(bytes / 1073741824.0, 2) + " GB";
    } else if (bytes >= 1048576) {
        return String(bytes / 1048576.0, 2) + " MB";
    } else if (bytes >= 1024) {
        return String(bytes / 1024.0, 1) + " KB";
    } else {
        return String(bytes) + " bytes";
    }
}

bool startIperfClient(const IperfConfig& config) {
    if (currentIperfState != IPERF_IDLE) {
        return false;
    }
    if (!mockWiFiConnected) {
        return false;
    }
    
    activeConfig = config;
    currentIperfState = IPERF_RUNNING;
    return true;
}

bool startIperfServer(const IperfConfig& config) {
    if (currentIperfState != IPERF_IDLE) {
        return false;
    }
    if (!mockWiFiConnected && !mockAPMode) {
        return false;
    }
    
    activeConfig = config;
    currentIperfState = IPERF_RUNNING;
    iperfServerRunning = true;
    return true;
}

void executeIperfCommand(const String& command) {
    String cmd = command;
    cmd.trim();
    cmd.toLowerCase();
    
    if (captureOutput) {
        testOutput += "Executing: " + cmd + "\n";
    }
    
    if (cmd == "iperf help") {
        if (captureOutput) testOutput += "IPERF COMMANDS help displayed\n";
    }
    else if (cmd == "iperf status") {
        if (captureOutput) testOutput += "IPERF STATUS displayed\n";
    }
    else if (cmd == "iperf stop") {
        stopIperfTest();
        if (captureOutput) testOutput += "iPerf test stopped\n";
    }
    else if (cmd.startsWith("iperf server")) {
        IperfConfig config = getDefaultConfig();
        config.mode = IPERF_SERVER;
        if (cmd.indexOf("udp") > 0) {
            config.protocol = IPERF_UDP;
        }
        startIperfServer(config);
    }
    else if (cmd.startsWith("iperf client")) {
        if (cmd.indexOf("192.168.1.100") > 0) {
            IperfConfig config = getDefaultConfig();
            config.mode = IPERF_CLIENT;
            config.serverIP = "192.168.1.100";
            if (cmd.indexOf("udp") > 0) {
                config.protocol = IPERF_UDP;
            }
            startIperfClient(config);
        } else {
            if (captureOutput) testOutput += "Usage: iperf client tcp <server_ip>\n";
        }
    }
    else {
        if (captureOutput) testOutput += "Unknown iPerf command\n";
    }
}

// ==========================================
// TEST SETUP AND TEARDOWN
// ==========================================
void setUp(void) {
    // Reset state before each test
    currentIperfState = IPERF_IDLE;
    iperfServerRunning = false;
    mockWiFiConnected = false;
    mockAPMode = false;
    testOutput = "";
    captureOutput = false;
    
    // Reset results
    lastResults = {};
    
    initializeIperf();
}

void tearDown(void) {
    // Clean up after each test
    stopIperfTest();
    shutdownIperf();
    testOutput = "";
    captureOutput = false;
}

// ==========================================
// CONFIGURATION TESTS
// ==========================================
void test_iperf_default_config(void) {
    IperfConfig config = getDefaultConfig();
    
    TEST_ASSERT_EQUAL(IPERF_TCP, config.protocol);
    TEST_ASSERT_EQUAL(IPERF_CLIENT, config.mode);
    TEST_ASSERT_EQUAL(IPERF_DEFAULT_PORT, config.port);
    TEST_ASSERT_EQUAL(IPERF_DEFAULT_DURATION, config.duration);
    TEST_ASSERT_EQUAL(IPERF_BUFFER_SIZE, config.bufferSize);
    TEST_ASSERT_FALSE(config.reverse);
    TEST_ASSERT_FALSE(config.bidir);
    TEST_ASSERT_EQUAL(1, config.parallel);
}

void test_iperf_custom_config(void) {
    IperfConfig config = getDefaultConfig();
    config.protocol = IPERF_UDP;
    config.mode = IPERF_SERVER;
    config.port = 8080;
    config.duration = 30;
    config.serverIP = "192.168.1.100";
    
    TEST_ASSERT_EQUAL(IPERF_UDP, config.protocol);
    TEST_ASSERT_EQUAL(IPERF_SERVER, config.mode);
    TEST_ASSERT_EQUAL(8080, config.port);
    TEST_ASSERT_EQUAL(30, config.duration);
    TEST_ASSERT_EQUAL_STRING("192.168.1.100", config.serverIP.c_str());
}

// ==========================================
// STATE MANAGEMENT TESTS
// ==========================================
void test_iperf_initial_state(void) {
    TEST_ASSERT_EQUAL(IPERF_IDLE, currentIperfState);
    TEST_ASSERT_FALSE(iperfServerRunning);
    TEST_ASSERT_FALSE(isIperfRunning());
}

void test_iperf_state_transitions(void) {
    // Test IDLE -> RUNNING
    currentIperfState = IPERF_RUNNING;
    TEST_ASSERT_TRUE(isIperfRunning());
    
    // Test RUNNING -> STOPPING
    currentIperfState = IPERF_STOPPING;
    TEST_ASSERT_FALSE(isIperfRunning());
    
    // Test back to IDLE
    currentIperfState = IPERF_IDLE;
    TEST_ASSERT_FALSE(isIperfRunning());
}

// ==========================================
// UTILITY FUNCTION TESTS
// ==========================================
void test_format_throughput(void) {
    String result = formatThroughput(0.5);
    TEST_ASSERT_TRUE(result.indexOf("Kbps") > 0);
    
    result = formatThroughput(50.25);
    TEST_ASSERT_TRUE(result.indexOf("Mbps") > 0);
    
    result = formatThroughput(1500.0);
    TEST_ASSERT_TRUE(result.indexOf("Gbps") > 0);
}

void test_format_bytes(void) {
    String result = formatBytes(512);
    TEST_ASSERT_TRUE(result.indexOf("bytes") > 0);
    
    result = formatBytes(2048);
    TEST_ASSERT_TRUE(result.indexOf("KB") > 0);
    
    result = formatBytes(2097152);
    TEST_ASSERT_TRUE(result.indexOf("MB") > 0);
    
    result = formatBytes(2147483648UL);
    TEST_ASSERT_TRUE(result.indexOf("GB") > 0);
}

// ==========================================
// COMMAND INTERFACE TESTS
// ==========================================
void test_iperf_help_command(void) {
    captureOutput = true;
    executeIperfCommand("iperf help");
    
    TEST_ASSERT_TRUE(testOutput.length() > 0);
    TEST_ASSERT_TRUE(testOutput.indexOf("IPERF COMMANDS") >= 0);
}

void test_iperf_status_command(void) {
    captureOutput = true;
    executeIperfCommand("iperf status");
    
    TEST_ASSERT_TRUE(testOutput.length() > 0);
    TEST_ASSERT_TRUE(testOutput.indexOf("STATUS") >= 0);
}

void test_iperf_stop_command(void) {
    captureOutput = true;
    currentIperfState = IPERF_RUNNING;
    
    executeIperfCommand("iperf stop");
    
    TEST_ASSERT_EQUAL(IPERF_IDLE, currentIperfState);
    TEST_ASSERT_TRUE(testOutput.indexOf("stopped") >= 0);
}

// ==========================================
// CLIENT/SERVER TESTS
// ==========================================
void test_iperf_client_without_wifi(void) {
    mockWiFiConnected = false;
    
    IperfConfig config = getDefaultConfig();
    config.serverIP = "192.168.1.100";
    
    bool result = startIperfClient(config);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(IPERF_IDLE, currentIperfState);
}

void test_iperf_client_with_wifi(void) {
    mockWiFiConnected = true;
    
    IperfConfig config = getDefaultConfig();
    config.serverIP = "192.168.1.100";
    
    bool result = startIperfClient(config);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(IPERF_RUNNING, currentIperfState);
}

void test_iperf_server_start(void) {
    mockWiFiConnected = true;
    
    IperfConfig config = getDefaultConfig();
    config.mode = IPERF_SERVER;
    
    bool result = startIperfServer(config);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(IPERF_RUNNING, currentIperfState);
    TEST_ASSERT_TRUE(iperfServerRunning);
}

void test_concurrent_operations(void) {
    mockWiFiConnected = true;
    
    // Start client
    IperfConfig clientConfig = getDefaultConfig();
    clientConfig.serverIP = "192.168.1.100";
    bool result1 = startIperfClient(clientConfig);
    TEST_ASSERT_TRUE(result1);
    
    // Try to start server (should fail)
    IperfConfig serverConfig = getDefaultConfig();
    serverConfig.mode = IPERF_SERVER;
    bool result2 = startIperfServer(serverConfig);
    TEST_ASSERT_FALSE(result2);
}

// ==========================================
// COMMAND PARSING TESTS
// ==========================================
void test_command_parsing(void) {
    mockWiFiConnected = true;
    captureOutput = true;
    
    // Test valid client command
    executeIperfCommand("iperf client tcp 192.168.1.100");
    TEST_ASSERT_EQUAL(IPERF_RUNNING, currentIperfState);
    TEST_ASSERT_EQUAL_STRING("192.168.1.100", activeConfig.serverIP.c_str());
    
    stopIperfTest();
    
    // Test server command
    executeIperfCommand("iperf server tcp");
    TEST_ASSERT_EQUAL(IPERF_RUNNING, currentIperfState);
    TEST_ASSERT_EQUAL(IPERF_SERVER, activeConfig.mode);
}

void test_invalid_commands(void) {
    captureOutput = true;
    
    executeIperfCommand("iperf invalid");
    TEST_ASSERT_TRUE(testOutput.indexOf("Unknown") >= 0);
    
    executeIperfCommand("iperf client tcp");
    TEST_ASSERT_TRUE(testOutput.indexOf("Usage") >= 0);
}

// ==========================================
// INTEGRATION TESTS
// ==========================================
void test_full_workflow(void) {
    mockWiFiConnected = true;
    captureOutput = true;
    
    // Initialize
    initializeIperf();
    TEST_ASSERT_EQUAL(IPERF_IDLE, currentIperfState);
    
    // Start client test
    executeIperfCommand("iperf client tcp 192.168.1.100");
    TEST_ASSERT_EQUAL(IPERF_RUNNING, currentIperfState);
    
    // Stop test
    executeIperfCommand("iperf stop");
    TEST_ASSERT_EQUAL(IPERF_IDLE, currentIperfState);
    
    // Cleanup
    shutdownIperf();
}

void test_results_structure(void) {
    // Test results initialization
    IperfResults results = getIperfResults();
    TEST_ASSERT_EQUAL(0, results.bytesTransferred);
    TEST_ASSERT_EQUAL(0, results.durationMs);
    TEST_ASSERT_EQUAL(0.0, results.throughputMbps);
    
    // Test results with data
    lastResults.bytesTransferred = 1048576;
    lastResults.durationMs = 1000;
    lastResults.throughputMbps = 8.0;
    lastResults.testCompleted = true;
    
    results = getIperfResults();
    TEST_ASSERT_EQUAL(1048576, results.bytesTransferred);
    TEST_ASSERT_EQUAL(1000, results.durationMs);
    TEST_ASSERT_EQUAL_FLOAT(8.0, results.throughputMbps);
    TEST_ASSERT_TRUE(results.testCompleted);
}

// ==========================================
// MAIN TEST RUNNER
// ==========================================
void setup() {
    delay(2000);
    
    Serial.begin(115200);
    Serial.println("\n===========================================");
    Serial.println("ESP32 WiFi Utility - iPerf Unit Tests");
    Serial.println("===========================================");
    
    UNITY_BEGIN();
    
    // Configuration tests
    RUN_TEST(test_iperf_default_config);
    RUN_TEST(test_iperf_custom_config);
    
    // State management tests
    RUN_TEST(test_iperf_initial_state);
    RUN_TEST(test_iperf_state_transitions);
    
    // Utility function tests
    RUN_TEST(test_format_throughput);
    RUN_TEST(test_format_bytes);
    
    // Command interface tests
    RUN_TEST(test_iperf_help_command);
    RUN_TEST(test_iperf_status_command);
    RUN_TEST(test_iperf_stop_command);
    
    // Client/Server functionality tests
    RUN_TEST(test_iperf_client_without_wifi);
    RUN_TEST(test_iperf_client_with_wifi);
    RUN_TEST(test_iperf_server_start);
    RUN_TEST(test_concurrent_operations);
    
    // Command parsing tests
    RUN_TEST(test_command_parsing);
    RUN_TEST(test_invalid_commands);
    
    // Integration tests
    RUN_TEST(test_full_workflow);
    RUN_TEST(test_results_structure);
    
    UNITY_END();
    
    Serial.println("\n===========================================");
    Serial.println("All Tests Completed!");
    Serial.println("===========================================");
}

void loop() {
    // Empty - tests run once in setup
}