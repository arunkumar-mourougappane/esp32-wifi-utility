#pragma once

// Test configuration for ESP32 WiFi Utility
// This file contains shared test configuration and mock definitions

#include <Arduino.h>
#include <unity.h>

// ==========================================
// TEST CONFIGURATION
// ==========================================
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

// Test configuration file
// This file contains common test configurations and utilities

#include <Arduino.h>

// Test timeout settings
#define TEST_TIMEOUT_MS 5000
#define QUICK_TEST_TIMEOUT_MS 1000

// Test network settings
#define TEST_SSID "TestNetwork"
#define TEST_PASSWORD "TestPassword123"

// Test thresholds
#define MAX_SCAN_TIME_MS 3000
#define MIN_FREE_HEAP 20000
#define MAX_CHANNEL_SCAN_TIME_MS 5000

// Channel analysis test constants
#define TEST_CHANNEL_COUNT 14
#define MAX_CONGESTION_SCORE 100.0
#define MIN_CONGESTION_SCORE 0.0
#define ACCEPTABLE_CONGESTION_THRESHOLD 50.0

// Mock data for testing
#define MOCK_RSSI_STRONG -30
#define MOCK_RSSI_MEDIUM -60
#define MOCK_RSSI_WEAK -80

// Test frequencies for 2.4GHz channels
#define CHANNEL_1_FREQ 2412
#define CHANNEL_6_FREQ 2437
#define CHANNEL_11_FREQ 2462

// Performance benchmarks
#define MAX_SCAN_DURATION_MS 5000
#define MAX_COMMAND_PROCESSING_MS 500
#define MIN_RECOMMENDED_FREE_HEAP 50000

// Conditional compilation helpers
#ifdef USE_NEOPIXEL
    #define NEOPIXEL_AVAILABLE 1
    #define NEOPIXEL_PIN 48  // Default pin for Feather ESP32-S3 TFT
    #define NEOPIXEL_COUNT 1
#else
    #define NEOPIXEL_AVAILABLE 0
    #define BUILTIN_LED_PIN 2  // Standard ESP32 built-in LED
#endif

// Board identification
#ifdef ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT
    #define BOARD_TYPE "Feather ESP32-S3 TFT"
    #define HAS_TFT_DISPLAY 1
#else
    #define BOARD_TYPE "Standard ESP32"
    #define HAS_TFT_DISPLAY 0
#endif

// Test utilities
inline bool isWithinRange(float value, float target, float tolerance) {
    return (value >= (target - tolerance)) && (value <= (target + tolerance));
}

inline unsigned long getCurrentTime() {
    return millis();
}

inline bool isValidChannel(uint8_t channel) {
    return (channel >= 1 && channel <= 14);
}

inline bool isRecommendedChannel(uint8_t channel) {
    return (channel == 1 || channel == 6 || channel == 11);
}

inline float mockCongestionScore(uint8_t networkCount) {
    // Simple mock congestion calculation: 10% base + 5% per network
    return constrain(10.0 + (networkCount * 5.0), 0.0, 100.0);
}

// Test assertion helpers
#define TEST_ASSERT_CHANNEL_VALID(channel) \
    TEST_ASSERT_TRUE_MESSAGE(isValidChannel(channel), "Channel must be 1-14")

#define TEST_ASSERT_CONGESTION_VALID(score) \
    TEST_ASSERT_TRUE_MESSAGE(isWithinRange(score, 50.0, 50.0), "Congestion score must be 0-100")

#define TEST_ASSERT_FREQUENCY_VALID(freq) \
    TEST_ASSERT_TRUE_MESSAGE((freq >= 2400 && freq <= 2500), "Frequency must be in 2.4GHz band")

#endif // TEST_CONFIG_H
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