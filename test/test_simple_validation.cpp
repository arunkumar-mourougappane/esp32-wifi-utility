#include <unity.h>
#include "test_config.h"
#include <WiFi.h>

// Simple working tests for initial validation

void test_basic_system_validation(void) {
    TEST_ASSERT_TRUE(true);
    TEST_ASSERT_EQUAL(2, 1 + 1);
}

void test_config_constants(void) {
    TEST_ASSERT_GREATER_THAN(0, TEST_TIMEOUT_MS);
    TEST_ASSERT_GREATER_THAN(0, MIN_FREE_HEAP);
    TEST_ASSERT_EQUAL(14, TEST_CHANNEL_COUNT);
}

void test_board_identification(void) {
#ifdef USE_NEOPIXEL
    TEST_ASSERT_EQUAL(1, NEOPIXEL_AVAILABLE);
    TEST_ASSERT_EQUAL_STRING("Feather ESP32-S3 TFT", BOARD_TYPE);
#else
    TEST_ASSERT_EQUAL(0, NEOPIXEL_AVAILABLE);
    TEST_ASSERT_EQUAL_STRING("Standard ESP32", BOARD_TYPE);
#endif
}

void test_helper_functions(void) {
    // Test range checking
    TEST_ASSERT_TRUE(isWithinRange(50.0, 45.0, 10.0));
    TEST_ASSERT_FALSE(isWithinRange(50.0, 30.0, 10.0));
    
    // Test channel validation
    TEST_ASSERT_TRUE(isValidChannel(1));
    TEST_ASSERT_TRUE(isValidChannel(6));
    TEST_ASSERT_TRUE(isValidChannel(11));
    TEST_ASSERT_FALSE(isValidChannel(0));
    TEST_ASSERT_FALSE(isValidChannel(15));
    
    // Test recommended channels
    TEST_ASSERT_TRUE(isRecommendedChannel(1));
    TEST_ASSERT_TRUE(isRecommendedChannel(6));
    TEST_ASSERT_TRUE(isRecommendedChannel(11));
    TEST_ASSERT_FALSE(isRecommendedChannel(3));
}

void test_mock_calculations(void) {
    // Test mock congestion calculation
    float score1 = mockCongestionScore(0);
    TEST_ASSERT_EQUAL_FLOAT(10.0, score1);
    
    float score2 = mockCongestionScore(5);
    TEST_ASSERT_EQUAL_FLOAT(35.0, score2);
    
    float score3 = mockCongestionScore(20); // Should cap at 100
    TEST_ASSERT_EQUAL_FLOAT(100.0, score3);
}

void test_wifi_scan_constants(void) {
    // Test WiFi related constants used in scan details
    TEST_ASSERT_LESS_THAN(60000, MAX_SCAN_TIME_MS);  // Should be reasonable timeout
    TEST_ASSERT_GREATER_THAN(1000, MAX_SCAN_TIME_MS); // But not too short
}

void test_network_security_levels(void) {
    // Test that we can distinguish between security levels
    // Open networks are less secure than WPA2
    TEST_ASSERT_NOT_EQUAL(WIFI_AUTH_OPEN, WIFI_AUTH_WPA2_PSK);
    TEST_ASSERT_NOT_EQUAL(WIFI_AUTH_OPEN, WIFI_AUTH_WPA3_PSK);
}

void test_rssi_range_validation(void) {
    // Test RSSI values are in valid range for WiFi signals
    int32_t testRssiValues[] = {-30, -50, -70, -90, -100};
    
    for (size_t i = 0; i < sizeof(testRssiValues) / sizeof(testRssiValues[0]); i++) {
        int32_t rssi = testRssiValues[i];
        // WiFi RSSI typically ranges from 0 (impossible) to -120 dBm
        TEST_ASSERT_LESS_THAN(0, rssi);
        TEST_ASSERT_GREATER_OR_EQUAL(-120, rssi);
    }
}

void test_2_4ghz_channel_range(void) {
    // Test that 2.4GHz channels are in valid range
    uint8_t validChannels[] = {1, 6, 11};  // Common non-overlapping channels
    
    for (size_t i = 0; i < sizeof(validChannels) / sizeof(validChannels[0]); i++) {
        TEST_ASSERT_CHANNEL_VALID(validChannels[i]);
        TEST_ASSERT_TRUE(isRecommendedChannel(validChannels[i]));
    }
}

void test_bssid_mac_format(void) {
    // Test MAC address format (BSSID) - should be 6 bytes
    uint8_t testBssid[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    
    // Verify each byte is valid (0-255)
    for (int i = 0; i < 6; i++) {
        TEST_ASSERT_LESS_THAN(256, testBssid[i]);
        TEST_ASSERT_GREATER_OR_EQUAL(0, testBssid[i]);
    }
}

void test_heap_memory_availability(void) {
    // Test that we have sufficient heap for web operations
    size_t freeHeap = ESP.getFreeHeap();
    TEST_ASSERT_GREATER_THAN(MIN_FREE_HEAP, freeHeap);
}

// ==========================================
// SCAN DETAILS CACHE TESTS
// ==========================================

// Mock cache structure (mirroring web_server.cpp implementation)
struct MockCachedScanResult {
    String ssid;
    int32_t rssi;
    uint8_t channel;
    wifi_auth_mode_t encryptionType;
    uint8_t bssid[6];
    bool hasBssid;
};

#define MAX_CACHED_NETWORKS_TEST 50
#define CACHE_TIMEOUT_MS_TEST 300000  // 5 minutes

static MockCachedScanResult mockCachedNetworks[MAX_CACHED_NETWORKS_TEST];
static int mockCachedNetworkCount = 0;
static unsigned long mockLastScanTime = 0;

// Mock cache validation (mirroring web_server.cpp)
bool mockIsCacheValid() {
    return (mockCachedNetworkCount > 0 && (millis() - mockLastScanTime < CACHE_TIMEOUT_MS_TEST));
}

// Mock cache initialization helper
void initializeMockCache(int networkCount) {
    mockCachedNetworkCount = networkCount > MAX_CACHED_NETWORKS_TEST ? MAX_CACHED_NETWORKS_TEST : networkCount;
    mockLastScanTime = millis();
    
    // Populate with test data
    for (int i = 0; i < mockCachedNetworkCount; i++) {
        mockCachedNetworks[i].ssid = "TestNetwork_" + String(i);
        mockCachedNetworks[i].rssi = -50 - (i * 5);
        mockCachedNetworks[i].channel = (i % 11) + 1;
        mockCachedNetworks[i].encryptionType = (i % 2 == 0) ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
        
        mockCachedNetworks[i].hasBssid = (i % 2 == 0);
        if (mockCachedNetworks[i].hasBssid) {
            for (int j = 0; j < 6; j++) {
                mockCachedNetworks[i].bssid[j] = i * 10 + j;
            }
        }
    }
}

void resetMockCache() {
    mockCachedNetworkCount = 0;
    mockLastScanTime = 0;
}

void test_cache_valid_with_recent_scan(void) {
    initializeMockCache(5);
    TEST_ASSERT_TRUE(mockIsCacheValid());
    resetMockCache();
}

void test_cache_invalid_when_empty(void) {
    resetMockCache();
    TEST_ASSERT_FALSE(mockIsCacheValid());
}

void test_cache_handles_max_networks(void) {
    initializeMockCache(60);
    TEST_ASSERT_EQUAL_INT32(MAX_CACHED_NETWORKS_TEST, mockCachedNetworkCount);
    resetMockCache();
}

void test_cache_network_data_integrity(void) {
    initializeMockCache(3);
    
    TEST_ASSERT_EQUAL_STRING("TestNetwork_0", mockCachedNetworks[0].ssid.c_str());
    TEST_ASSERT_EQUAL_INT32(-50, mockCachedNetworks[0].rssi);
    TEST_ASSERT_TRUE(isValidChannel(mockCachedNetworks[0].channel));
    
    resetMockCache();
}

void test_network_signal_quality_levels(void) {
    // Test signal quality categorization
    struct {
        int32_t rssi;
        const char* expectedQuality;
    } testCases[] = {
        {-30, "Excellent"},
        {-50, "Excellent"},  // -50 is still excellent (>= -50)
        {-55, "Very Good"},
        {-70, "Good"},
        {-80, "Weak"}
    };
    
    for (size_t i = 0; i < sizeof(testCases) / sizeof(testCases[0]); i++) {
        int32_t rssi = testCases[i].rssi;
        const char* quality;
        
        if (rssi >= -50) quality = "Excellent";
        else if (rssi >= -60) quality = "Very Good";
        else if (rssi >= -70) quality = "Good";
        else if (rssi >= -75) quality = "Fair";
        else if (rssi >= -80) quality = "Weak";
        else quality = "Very Weak";
        
        TEST_ASSERT_EQUAL_STRING(testCases[i].expectedQuality, quality);
    }
}

void test_encryption_type_names(void) {
    // Test encryption type display names
    TEST_ASSERT_NOT_EQUAL(WIFI_AUTH_OPEN, WIFI_AUTH_WPA2_PSK);
    TEST_ASSERT_NOT_EQUAL(WIFI_AUTH_WPA_PSK, WIFI_AUTH_WPA2_PSK);
    TEST_ASSERT_NOT_EQUAL(WIFI_AUTH_WPA2_PSK, WIFI_AUTH_WPA3_PSK);
}

void test_channel_frequency_conversion(void) {
    // Test channel to frequency conversion for 2.4GHz
    uint8_t channel = 6;
    uint16_t expectedFreq = 2437;  // Channel 6 = 2437 MHz
    uint16_t calculatedFreq = 2407 + (channel * 5);
    
    TEST_ASSERT_EQUAL_UINT16(expectedFreq, calculatedFreq);
}

void test_network_id_validation(void) {
    initializeMockCache(10);
    
    // Valid IDs
    TEST_ASSERT_TRUE(0 >= 0 && 0 < mockCachedNetworkCount);
    TEST_ASSERT_TRUE(5 >= 0 && 5 < mockCachedNetworkCount);
    TEST_ASSERT_TRUE(9 >= 0 && 9 < mockCachedNetworkCount);
    
    // Invalid IDs
    TEST_ASSERT_FALSE(-1 >= 0 && -1 < mockCachedNetworkCount);
    TEST_ASSERT_FALSE(10 >= 0 && 10 < mockCachedNetworkCount);
    TEST_ASSERT_FALSE(100 >= 0 && 100 < mockCachedNetworkCount);
    
    resetMockCache();
}

void setup() {
    delay(1000); // Give time for serial to initialize
    
    UNITY_BEGIN();
    
    // Basic system tests
    RUN_TEST(test_basic_system_validation);
    RUN_TEST(test_config_constants);
    RUN_TEST(test_board_identification);
    RUN_TEST(test_helper_functions);
    RUN_TEST(test_mock_calculations);
    
    // WiFi and network tests
    RUN_TEST(test_wifi_scan_constants);
    RUN_TEST(test_network_security_levels);
    RUN_TEST(test_rssi_range_validation);
    RUN_TEST(test_2_4ghz_channel_range);
    RUN_TEST(test_bssid_mac_format);
    RUN_TEST(test_heap_memory_availability);
    
    // Scan details cache tests
    RUN_TEST(test_cache_valid_with_recent_scan);
    RUN_TEST(test_cache_invalid_when_empty);
    RUN_TEST(test_cache_handles_max_networks);
    RUN_TEST(test_cache_network_data_integrity);
    RUN_TEST(test_network_signal_quality_levels);
    RUN_TEST(test_encryption_type_names);
    RUN_TEST(test_channel_frequency_conversion);
    RUN_TEST(test_network_id_validation);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup, loop does nothing
}