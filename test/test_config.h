#pragma once

// Test configuration for ESP32 WiFi Utility
// This file contains shared test configuration and mock definitions

#include <Arduino.h>
#include <unity.h>

// ==========================================
// TEST CONFIGURATION
// ==========================================
#define TEST_TIMEOUT_MS 5000
#define TEST_BUFFER_SIZE 1024
#define TEST_DEFAULT_PORT 5201

// ==========================================
// MOCK NETWORK ADDRESSES
// ==========================================
#define MOCK_SERVER_IP "192.168.1.100"
#define MOCK_CLIENT_IP "192.168.1.101"
#define MOCK_AP_IP "192.168.4.1"

// ==========================================
// TEST HELPER MACROS
// ==========================================
#define TEST_ASSERT_IN_RANGE(value, min, max) \
    TEST_ASSERT_TRUE_MESSAGE(((value) >= (min) && (value) <= (max)), \
                           "Value not in expected range")

#define TEST_ASSERT_THROUGHPUT_VALID(mbps) \
    TEST_ASSERT_TRUE_MESSAGE((mbps) > 0.0 && (mbps) < 1000.0, \
                           "Throughput value is not valid")

#define TEST_ASSERT_STRING_NOT_EMPTY(str) \
    TEST_ASSERT_TRUE_MESSAGE((str).length() > 0, \
                           "String should not be empty")

// ==========================================
// MOCK WIFI STATES
// ==========================================
extern bool mockWiFiConnected;
extern bool mockAPMode;
extern String mockConnectedSSID;
extern IPAddress mockLocalIP;

// ==========================================
// TEST HELPER FUNCTIONS
// ==========================================
void setupMockWiFi(bool connected = true, bool apMode = false);
void resetMockWiFi();
void setupTestEnvironment();
void cleanupTestEnvironment();

// ==========================================
// ASSERTION HELPERS
// ==========================================
void assertIperfConfigValid(const struct IperfConfig& config);
void assertIperfResultsValid(const struct IperfResults& results);
void assertCommandOutput(const String& expectedOutput);

// ==========================================
// TIMING HELPERS
// ==========================================
void waitForCondition(bool (*condition)(), unsigned long timeoutMs = TEST_TIMEOUT_MS);
void simulateDelay(unsigned long ms);

// ==========================================
// MOCK IMPLEMENTATION FLAGS
// ==========================================
extern bool enableNetworkMocking;
extern bool simulateNetworkErrors;
extern bool simulateSlowNetwork;