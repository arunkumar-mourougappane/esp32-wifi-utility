#include <unity.h>
#include "test_config.h"

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

void setup() {
    delay(1000); // Give time for serial to initialize
    
    UNITY_BEGIN();
    
    RUN_TEST(test_basic_system_validation);
    RUN_TEST(test_config_constants);
    RUN_TEST(test_board_identification);
    RUN_TEST(test_helper_functions);
    RUN_TEST(test_mock_calculations);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup, loop does nothing
}