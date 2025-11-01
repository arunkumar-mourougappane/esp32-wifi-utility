#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <AsyncUDP.h>

// ==========================================
// JITTER & LATENCY ANALYSIS CONFIGURATION
// ==========================================
#define PING_DEFAULT_COUNT 10
#define PING_DEFAULT_INTERVAL 1000  // ms between pings
#define PING_DEFAULT_TIMEOUT 5000   // ms timeout for ping response
#define PING_MAX_COUNT 100
#define JITTER_BUFFER_SIZE 50
#define LATENCY_STATS_WINDOW 100

// ==========================================
// TEST TYPES AND STATES
// ==========================================
enum LatencyTestType {
  LATENCY_ICMP_PING = 0,     // ICMP ping test (if supported)
  LATENCY_UDP_ECHO = 1,      // UDP echo test
  LATENCY_TCP_CONNECT = 2,   // TCP connection time test
  LATENCY_HTTP_REQUEST = 3   // HTTP request latency test
};

enum LatencyTestState {
  LATENCY_IDLE = 0,
  LATENCY_RUNNING = 1,
  LATENCY_COMPLETED = 2,
  LATENCY_ERROR = 3
};

// ==========================================
// DATA STRUCTURES
// ==========================================
struct PingResult {
  bool success;
  float latency_ms;
  unsigned long timestamp;
  uint16_t sequence;
  String errorMessage;
};

struct JitterStats {
  float min_latency_ms;
  float max_latency_ms;
  float avg_latency_ms;
  float jitter_ms;          // Average jitter
  float max_jitter_ms;      // Maximum jitter observed
  float packet_loss_percent;
  uint32_t packets_sent;
  uint32_t packets_received;
  uint32_t packets_lost;
};

struct LatencyConfig {
  String target_host;
  uint16_t target_port;
  LatencyTestType test_type;
  uint16_t packet_count;
  uint16_t packet_size;
  uint32_t interval_ms;
  uint32_t timeout_ms;
  bool continuous_mode;
};

struct LatencyTestResults {
  bool test_completed;
  LatencyTestState state;
  JitterStats statistics;
  PingResult results[PING_MAX_COUNT];
  uint16_t results_count;
  unsigned long test_duration_ms;
  String error_message;
};

// ==========================================
// GLOBAL VARIABLES
// ==========================================
extern LatencyTestState currentLatencyState;
extern LatencyConfig activeLatencyConfig;
extern LatencyTestResults lastLatencyResults;

// ==========================================
// CORE FUNCTIONS
// ==========================================

/**
 * @brief Initialize the jitter and latency analysis system
 */
void initializeLatencyAnalysis();

/**
 * @brief Shutdown and cleanup latency analysis resources
 */
void shutdownLatencyAnalysis();

/**
 * @brief Start a latency and jitter test
 * @param config Test configuration parameters
 * @return true if test started successfully, false otherwise
 */
bool startLatencyTest(const LatencyConfig& config);

/**
 * @brief Stop the currently running latency test
 */
void stopLatencyTest();

/**
 * @brief Handle background latency test tasks
 * This function should be called repeatedly in the main loop
 */
void handleLatencyTasks();

/**
 * @brief Send a single latency probe packet
 */
void sendLatencyProbe();

/**
 * @brief Send UDP echo probe packet
 * @param sendTime Timestamp when packet was sent
 */
void sendUdpEchoProbe(unsigned long sendTime);

/**
 * @brief Send TCP connect probe
 * @param sendTime Timestamp when connection attempt started
 */
void sendTcpConnectProbe(unsigned long sendTime);

/**
 * @brief Send HTTP latency probe
 * @param sendTime Timestamp when request started
 */
void sendHttpLatencyProbe(unsigned long sendTime);

/**
 * @brief Process incoming latency test responses
 */
void processLatencyResponses();

/**
 * @brief Get the current test status and progress
 * @return Current test state and progress information
 */
String getLatencyStatus();

// ==========================================
// TEST EXECUTION FUNCTIONS
// ==========================================

/**
 * @brief Execute UDP echo latency test
 * @param config Test configuration
 * @return true if test execution started successfully
 */
bool executeUdpEchoTest(const LatencyConfig& config);

/**
 * @brief Execute TCP connection latency test
 * @param config Test configuration
 * @return true if test execution started successfully
 */
bool executeTcpConnectTest(const LatencyConfig& config);

/**
 * @brief Execute HTTP request latency test
 * @param config Test configuration
 * @return true if test execution started successfully
 */
bool executeHttpLatencyTest(const LatencyConfig& config);

/**
 * @brief Execute comprehensive network analysis
 * @param target_host Target host for analysis
 * @return Complete analysis results
 */
JitterStats executeNetworkAnalysis(const String& target_host);

// ==========================================
// STATISTICS AND ANALYSIS FUNCTIONS
// ==========================================

/**
 * @brief Calculate jitter statistics from ping results
 * @param results Array of ping results
 * @param count Number of results
 * @return Calculated jitter and latency statistics
 */
JitterStats calculateJitterStats(PingResult* results, uint16_t count);

/**
 * @brief Update running statistics with new ping result
 * @param result New ping result to add
 */
void updateRunningStats(const PingResult& result);

/**
 * @brief Print detailed latency test results
 * @param results Test results to display
 */
void printLatencyResults(const LatencyTestResults& results);

/**
 * @brief Print real-time statistics during test execution
 */
void printRunningLatencyStats();

/**
 * @brief Export results to JSON format
 * @param results Test results to export
 * @return JSON formatted string
 */
String exportLatencyResultsJSON(const LatencyTestResults& results);

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

/**
 * @brief Get default latency test configuration
 * @param test_type Type of latency test
 * @return Default configuration for specified test type
 */
LatencyConfig getDefaultLatencyConfig(LatencyTestType test_type);

/**
 * @brief Validate latency test configuration
 * @param config Configuration to validate
 * @return true if configuration is valid
 */
bool validateLatencyConfig(const LatencyConfig& config);

/**
 * @brief Print latency test configuration
 * @param config Configuration to display
 */
void printLatencyConfig(const LatencyConfig& config);

/**
 * @brief Convert test type to string
 * @param type Test type enum
 * @return String representation of test type
 */
String latencyTestTypeToString(LatencyTestType type);

/**
 * @brief Convert test state to string
 * @param state Test state enum
 * @return String representation of test state
 */
String latencyTestStateToString(LatencyTestState state);

// ==========================================
// ADVANCED ANALYSIS FUNCTIONS
// ==========================================

/**
 * @brief Perform network quality assessment
 * @param target_host Target for assessment
 * @return Network quality score (0-100)
 */
uint8_t assessNetworkQuality(const String& target_host);

/**
 * @brief Detect network issues based on latency patterns
 * @param results Test results to analyze
 * @return Array of detected issues
 */
String* detectNetworkIssues(const LatencyTestResults& results);

/**
 * @brief Generate network optimization recommendations
 * @param stats Network statistics
 * @return Optimization recommendations
 */
String generateOptimizationRecommendations(const JitterStats& stats);

// ==========================================
// GETTER FUNCTIONS FOR EXTERNAL ACCESS
// ==========================================

/**
 * @brief Get current latency test state
 * @return Current test state
 */
LatencyTestState getLatencyTestState();

/**
 * @brief Get last completed latency test results
 * @return Last test results structure
 */
LatencyTestResults getLastLatencyResults();