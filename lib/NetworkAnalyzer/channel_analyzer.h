/**
 * @file channel_analyzer.h
 * @brief WiFi channel congestion analysis and spectrum monitoring
 * 
 * This header defines structures and functions for analyzing WiFi channel
 * congestion, interference detection, and optimal channel recommendation.
 * Supports comprehensive 2.4GHz spectrum analysis with overlapping channel
 * detection and continuous monitoring capabilities.
 * 
 * @author Arunkumar Mourougappane
 * @version 3.0.0
 * @date 2026-01-17
 */

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <vector>

// ==========================================
// CHANNEL CONGESTION ANALYSIS STRUCTURES
// ==========================================

/**
 * @brief Structure to hold channel congestion data
 */
typedef struct {
    uint8_t channel;                    // Channel number (1-14)
    uint8_t network_count;              // Number of networks on this channel
    int32_t strongest_rssi;             // Strongest signal on this channel
    int32_t average_rssi;               // Average RSSI for this channel
    float congestion_score;             // Congestion score (0-100, higher = more congested)
    uint8_t overlapping_networks;       // Networks that overlap with this channel
    bool is_recommended;                // True if channel is recommended for use
    String dominant_network;            // SSID of strongest network on channel
} ChannelCongestionData;

/**
 * @brief Structure for comprehensive channel analysis results
 */
typedef struct {
    ChannelCongestionData channels[14]; // Channel data for channels 0-13 (1-14)
    uint8_t total_networks;             // Total networks scanned
    uint8_t best_channel_2g4;           // Best 2.4GHz channel recommendation
    uint8_t worst_channel_2g4;          // Most congested 2.4GHz channel
    float overall_congestion;           // Overall spectrum congestion (0-100)
    unsigned long scan_timestamp;       // When scan was performed
    uint16_t scan_duration_ms;          // How long the scan took
    bool interference_detected;         // True if non-WiFi interference detected
} ChannelAnalysisResults;

/**
 * @brief Channel scanning configuration
 */
typedef struct {
    bool include_hidden_networks;       // Include hidden SSIDs in analysis
    uint16_t scan_duration_ms;          // How long to scan each channel
    bool detailed_analysis;             // Perform detailed interference analysis
    bool continuous_monitoring;         // Enable continuous background monitoring
    uint8_t monitoring_interval_sec;    // Interval for continuous monitoring
} ChannelScanConfig;

// ==========================================
// GLOBAL VARIABLES
// ==========================================
extern ChannelAnalysisResults lastChannelAnalysis;
extern ChannelScanConfig channelScanConfig;
extern bool channelMonitoringActive;
extern unsigned long lastChannelScan;

// ==========================================
// FUNCTION DECLARATIONS
// ==========================================

/**
 * @brief Initialize the channel congestion analyzer
 */
void initializeChannelAnalysis();

/**
 * @brief Perform comprehensive channel congestion analysis
 * @param config Scan configuration options
 * @return ChannelAnalysisResults Complete analysis results
 */
ChannelAnalysisResults performChannelCongestionScan(const ChannelScanConfig& config);

/**
 * @brief Quick channel congestion scan with default settings
 * @return ChannelAnalysisResults Basic analysis results
 */
ChannelAnalysisResults quickChannelScan();

/**
 * @brief Analyze channel overlap and interference
 * @param results Analysis results to enhance with overlap data
 */
void analyzeChannelOverlap(ChannelAnalysisResults& results);

/**
 * @brief Calculate congestion score for a specific channel
 * @param channel Channel number (1-14)
 * @param networkCount Number of networks on channel
 * @param avgRssi Average RSSI on channel
 * @param strongestRssi Strongest signal on channel
 * @return float Congestion score (0-100)
 */
float calculateCongestionScore(uint8_t channel, uint8_t networkCount, 
                              int32_t avgRssi, int32_t strongestRssi);

/**
 * @brief Recommend best channels for AP deployment
 * @param results Channel analysis results
 * @return std::vector<uint8_t> Recommended channels in priority order
 */
std::vector<uint8_t> recommendChannels(const ChannelAnalysisResults& results);

/**
 * @brief Start continuous channel monitoring
 * @param intervalSeconds Monitoring interval in seconds
 */
void startChannelMonitoring(uint8_t intervalSeconds = 30);

/**
 * @brief Stop continuous channel monitoring
 */
void stopChannelMonitoring();

/**
 * @brief Handle background channel monitoring tasks
 * Call this in main loop when monitoring is active
 */
void handleChannelMonitoringTasks();

/**
 * @brief Print comprehensive channel analysis results
 * @param results Analysis results to display
 */
void printChannelAnalysisResults(const ChannelAnalysisResults& results);

/**
 * @brief Print channel congestion summary
 * @param results Analysis results to summarize
 */
void printChannelCongestionSummary(const ChannelAnalysisResults& results);

/**
 * @brief Print channel recommendations
 * @param results Analysis results for recommendations
 */
void printChannelRecommendations(const ChannelAnalysisResults& results);

/**
 * @brief Export channel analysis results to JSON format
 * @param results Analysis results to export
 * @return String JSON formatted results
 */
String exportChannelAnalysisToJSON(const ChannelAnalysisResults& results);

/**
 * @brief Get default channel scan configuration
 * @return ChannelScanConfig Default configuration
 */
ChannelScanConfig getDefaultChannelScanConfig();

/**
 * @brief Detect non-WiFi interference on channels
 * @param results Analysis results to enhance with interference detection
 */
void detectInterference(ChannelAnalysisResults& results);

/**
 * @brief Get channel utilization percentage
 * @param channel Channel number (1-14)
 * @return float Utilization percentage (0-100)
 */
float getChannelUtilization(uint8_t channel);

/**
 * @brief Generate channel optimization recommendations
 * @param results Channel analysis results
 * @return String Detailed recommendations text
 */
String generateChannelOptimizationReport(const ChannelAnalysisResults& results);

/**
 * @brief Get current channel monitoring status
 * @return String Status information
 */
String getChannelMonitoringStatus();

/**
 * @brief Validate channel number for 2.4GHz band
 * @param channel Channel number to validate
 * @return bool True if valid 2.4GHz channel
 */
bool isValidChannel(uint8_t channel);

/**
 * @brief Get channel frequency in MHz
 * @param channel Channel number (1-14)
 * @return uint16_t Frequency in MHz
 */
uint16_t getChannelFrequency(uint8_t channel);

/**
 * @brief Check if two channels overlap
 * @param channel1 First channel
 * @param channel2 Second channel
 * @return bool True if channels overlap
 */
bool channelsOverlap(uint8_t channel1, uint8_t channel2);

/**
 * @brief Get the last channel analysis results
 * @return ChannelAnalysisResults Most recent analysis
 */
ChannelAnalysisResults getLastChannelAnalysis();

/**
 * @brief Reset channel analysis data
 */
void resetChannelAnalysis();