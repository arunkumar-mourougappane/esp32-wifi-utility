/**
 * @file channel_analyzer.cpp
 * @brief WiFi channel congestion analysis implementation
 * 
 * This file implements comprehensive channel analysis functionality:
 * - 2.4GHz spectrum scanning across channels 1-14
 * - Congestion scoring based on network count and signal strength
 * - Overlapping channel interference detection
 * - Best/worst channel recommendations
 * - Continuous background monitoring support
 * - Channel utilization visualization data
 * 
 * @author Arunkumar Mourougappane
 * @version 3.0.0
 * @date 2026-01-17
 */

#include "channel_analyzer.h"
#include "config.h"
#ifdef USE_NEOPIXEL
#include "led_controller.h"
#endif
#include <WiFi.h>
#include <vector>
#include <algorithm>

// ==========================================
// GLOBAL VARIABLES
// ==========================================
ChannelAnalysisResults lastChannelAnalysis;
ChannelScanConfig channelScanConfig;
bool channelMonitoringActive = false;
unsigned long lastChannelScan = 0;

// Internal monitoring variables
static unsigned long lastMonitoringUpdate = 0;
static uint8_t monitoringInterval = 30; // seconds

// Channel frequencies in MHz (2.4GHz band)
static const uint16_t channelFrequencies[14] = {
    0,    // Channel 0 (invalid)
    2412, // Channel 1
    2417, // Channel 2
    2422, // Channel 3
    2427, // Channel 4
    2432, // Channel 5
    2437, // Channel 6
    2442, // Channel 7
    2447, // Channel 8
    2452, // Channel 9
    2457, // Channel 10
    2462, // Channel 11
    2467, // Channel 12
    2472, // Channel 13
};

// ==========================================
// INITIALIZATION
// ==========================================
void initializeChannelAnalysis() {
    // Initialize global variables
    memset(&lastChannelAnalysis, 0, sizeof(ChannelAnalysisResults));
    channelScanConfig = getDefaultChannelScanConfig();
    channelMonitoringActive = false;
    lastChannelScan = 0;
    
    Serial.println("🔧 Channel Congestion Analyzer initialized");
}

// ==========================================
// CORE SCANNING FUNCTIONS
// ==========================================
ChannelAnalysisResults performChannelCongestionScan(const ChannelScanConfig& config) {
    ChannelAnalysisResults results;
    memset(&results, 0, sizeof(ChannelAnalysisResults));
    
    results.scan_timestamp = millis();
    unsigned long scanStart = millis();
    
    Serial.println("🔍 Starting comprehensive channel congestion analysis...");
    
#ifdef USE_NEOPIXEL
    // Show blue color during channel scanning
    setNeoPixelColor(0, 0, 255);
#endif
    
    // Perform WiFi scan
    int networkCount = WiFi.scanNetworks(false, config.include_hidden_networks);
    
    if (networkCount == 0) {
        Serial.println("❌ No networks found during channel scan");
        results.scan_duration_ms = millis() - scanStart;
        return results;
    }
    
    results.total_networks = networkCount;
    
    // Initialize channel data
    for (int i = 0; i < 14; i++) {
        results.channels[i].channel = i;
        results.channels[i].network_count = 0;
        results.channels[i].strongest_rssi = -100;
        results.channels[i].average_rssi = -100;
        results.channels[i].congestion_score = 0;
        results.channels[i].overlapping_networks = 0;
        results.channels[i].is_recommended = false;
        results.channels[i].dominant_network = "";
    }
    
    // Collect network data per channel
    for (int i = 0; i < networkCount; i++) {
        uint8_t ch = WiFi.channel(i);
        if (!isValidChannel(ch)) continue;
        
        int32_t rssi = WiFi.RSSI(i);
        String ssid = WiFi.SSID(i);
        
        // Update channel statistics
        ChannelCongestionData& channelData = results.channels[ch];
        channelData.network_count++;
        
        // Track strongest signal and dominant network
        if (rssi > channelData.strongest_rssi) {
            channelData.strongest_rssi = rssi;
            channelData.dominant_network = ssid;
        }
        
        // Calculate running average RSSI
        if (channelData.network_count == 1) {
            channelData.average_rssi = rssi;
        } else {
            channelData.average_rssi = (channelData.average_rssi * (channelData.network_count - 1) + rssi) / channelData.network_count;
        }
    }
    
    // Calculate congestion scores and analyze overlaps
    float totalCongestion = 0;
    uint8_t activeChannels = 0;
    
    for (int ch = 1; ch <= 13; ch++) {
        ChannelCongestionData& channelData = results.channels[ch];
        
        if (channelData.network_count > 0) {
            activeChannels++;
            
            // Calculate congestion score
            channelData.congestion_score = calculateCongestionScore(
                ch, channelData.network_count, 
                channelData.average_rssi, channelData.strongest_rssi
            );
            
            totalCongestion += channelData.congestion_score;
        }
    }
    
    // Calculate overall congestion
    if (activeChannels > 0) {
        results.overall_congestion = totalCongestion / activeChannels;
    }
    
    // Analyze channel overlap
    analyzeChannelOverlap(results);
    
    // Find best and worst channels
    uint8_t bestChannel = 1, worstChannel = 1;
    float lowestScore = 100, highestScore = 0;
    
    for (int ch = 1; ch <= 13; ch++) {
        float score = results.channels[ch].congestion_score;
        if (score < lowestScore) {
            lowestScore = score;
            bestChannel = ch;
        }
        if (score > highestScore) {
            highestScore = score;
            worstChannel = ch;
        }
    }
    
    results.best_channel_2g4 = bestChannel;
    results.worst_channel_2g4 = worstChannel;
    
    // Detect interference if detailed analysis is enabled
    if (config.detailed_analysis) {
        detectInterference(results);
    }
    
    // Mark recommended channels
    std::vector<uint8_t> recommended = recommendChannels(results);
    for (uint8_t ch : recommended) {
        if (ch <= 13) {
            results.channels[ch].is_recommended = true;
        }
    }
    
    results.scan_duration_ms = millis() - scanStart;
    lastChannelAnalysis = results;
    lastChannelScan = millis();
    
    Serial.printf("✅ Channel analysis completed in %dms\n", results.scan_duration_ms);
    
#ifdef USE_NEOPIXEL
    // Show green for completed scan
    setNeoPixelColor(0, 255, 0);
    delay(500);
#endif
    
    // Clean up scan results
    WiFi.scanDelete();
    
    return results;
}

ChannelAnalysisResults quickChannelScan() {
    ChannelScanConfig config = getDefaultChannelScanConfig();
    config.detailed_analysis = false;
    config.scan_duration_ms = 1000; // Quick 1-second scan
    return performChannelCongestionScan(config);
}

// ==========================================
// ANALYSIS FUNCTIONS
// ==========================================
void analyzeChannelOverlap(ChannelAnalysisResults& results) {
    // WiFi channels overlap: each channel affects ±2 channels
    // Channel 1 overlaps with 1,2,3,4,5
    // Channel 6 overlaps with 4,5,6,7,8
    // Channel 11 overlaps with 9,10,11,12,13
    
    for (int ch = 1; ch <= 13; ch++) {
        ChannelCongestionData& channelData = results.channels[ch];
        channelData.overlapping_networks = 0;
        
        // Count overlapping networks (±2 channels)
        for (int overlap_ch = max(1, ch - 2); overlap_ch <= min(13, ch + 2); overlap_ch++) {
            if (overlap_ch != ch && results.channels[overlap_ch].network_count > 0) {
                channelData.overlapping_networks += results.channels[overlap_ch].network_count;
            }
        }
    }
}

float calculateCongestionScore(uint8_t channel, uint8_t networkCount, 
                              int32_t avgRssi, int32_t strongestRssi) {
    float score = 0;
    
    // Base score from network count (0-50 points)
    score += min(50.0f, networkCount * 8.0f);
    
    // Signal strength impact (0-30 points)
    // Stronger signals = more interference
    if (strongestRssi > -40) score += 30;
    else if (strongestRssi > -50) score += 25;
    else if (strongestRssi > -60) score += 20;
    else if (strongestRssi > -70) score += 15;
    else if (strongestRssi > -80) score += 10;
    else score += 5;
    
    // Average signal strength (0-20 points)
    if (avgRssi > -50) score += 20;
    else if (avgRssi > -60) score += 15;
    else if (avgRssi > -70) score += 10;
    else if (avgRssi > -80) score += 5;
    
    // Ensure score is in valid range
    return min(100.0f, max(0.0f, score));
}

std::vector<uint8_t> recommendChannels(const ChannelAnalysisResults& results) {
    std::vector<std::pair<uint8_t, float>> channelScores;
    
    // Standard non-overlapping channels in priority order
    std::vector<uint8_t> standardChannels = {1, 6, 11};
    
    // Add standard channels with their scores
    for (uint8_t ch : standardChannels) {
        float score = results.channels[ch].congestion_score;
        channelScores.push_back({ch, score});
    }
    
    // Add other channels if they're significantly better
    for (int ch = 1; ch <= 13; ch++) {
        if (std::find(standardChannels.begin(), standardChannels.end(), ch) == standardChannels.end()) {
            float score = results.channels[ch].congestion_score;
            // Only recommend non-standard channels if they're much better
            if (score < 20) { // Very low congestion
                channelScores.push_back({ch, score});
            }
        }
    }
    
    // Sort by congestion score (lower is better)
    std::sort(channelScores.begin(), channelScores.end(), 
              [](const std::pair<uint8_t, float>& a, const std::pair<uint8_t, float>& b) {
                  return a.second < b.second;
              });
    
    std::vector<uint8_t> recommended;
    for (const std::pair<uint8_t, float>& pair : channelScores) {
        recommended.push_back(pair.first);
    }
    
    return recommended;
}

// ==========================================
// MONITORING FUNCTIONS
// ==========================================
void startChannelMonitoring(uint8_t intervalSeconds) {
    channelMonitoringActive = true;
    monitoringInterval = intervalSeconds;
    lastMonitoringUpdate = millis();
    
    Serial.printf("🔄 Channel monitoring started (interval: %d seconds)\n", intervalSeconds);
    
    // Perform initial scan
    quickChannelScan();
}

void stopChannelMonitoring() {
    channelMonitoringActive = false;
    Serial.println("⏹️ Channel monitoring stopped");
}

void handleChannelMonitoringTasks() {
    if (!channelMonitoringActive) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastMonitoringUpdate >= (monitoringInterval * 1000)) {
        Serial.println("📊 Performing scheduled channel analysis...");
        quickChannelScan();
        lastMonitoringUpdate = currentTime;
    }
}

// ==========================================
// DISPLAY FUNCTIONS
// ==========================================
void printChannelAnalysisResults(const ChannelAnalysisResults& results) {
    Serial.println("\n📡 === Comprehensive Channel Congestion Analysis ===");
    Serial.printf("⏰ Scan Time: %lu ms | Networks Found: %d | Overall Congestion: %.1f%%\n", 
                  results.scan_duration_ms, results.total_networks, results.overall_congestion);
    Serial.println("┌────┬─────────┬──────────┬───────┬─────────────────────────┬────────────┐");
    Serial.println("│ CH │ Networks│ Congestion│  RSSI │      Dominant Network   │ Overlap │ R │");
    Serial.println("├────┼─────────┼──────────┼───────┼─────────────────────────┼────────────┤");
    
    for (int ch = 1; ch <= 13; ch++) {
        const ChannelCongestionData& data = results.channels[ch];
        
        String congestionBar = "";
        int bars = (int)(data.congestion_score / 10);
        for (int i = 0; i < 10; i++) {
            congestionBar += (i < bars) ? "█" : "░";
        }
        
        String rssiStr = (data.network_count > 0) ? String(data.strongest_rssi) + "dBm" : "N/A";
        String networkName = data.dominant_network.substring(0, 20); // Truncate long names
        char recommended = data.is_recommended ? 'Y' : 'N';
        
        Serial.printf("│%3d │%8d │%s│%7s│ %-23s │%11d │ %c │\n",
                      ch, data.network_count, congestionBar.c_str(), 
                      rssiStr.c_str(), networkName.c_str(), 
                      data.overlapping_networks, recommended);
    }
    
    Serial.println("└────┴─────────┴──────────┴───────┴─────────────────────────┴────────────┘");
    Serial.printf("📊 Best Channel: %d (%.1f%% congestion)\n", 
                  results.best_channel_2g4, results.channels[results.best_channel_2g4].congestion_score);
    Serial.printf("⚠️  Worst Channel: %d (%.1f%% congestion)\n", 
                  results.worst_channel_2g4, results.channels[results.worst_channel_2g4].congestion_score);
    
    if (results.interference_detected) {
        Serial.println("⚡ Non-WiFi interference detected on some channels");
    }
    
    Serial.println("R = Recommended for AP use\n");
}

void printChannelCongestionSummary(const ChannelAnalysisResults& results) {
    Serial.println("📊 === Channel Congestion Summary ===");
    
    // Show congestion levels
    uint8_t lowCongestion = 0, mediumCongestion = 0, highCongestion = 0;
    for (int ch = 1; ch <= 13; ch++) {
        float score = results.channels[ch].congestion_score;
        if (score < 30) lowCongestion++;
        else if (score < 70) mediumCongestion++;
        else if (score > 0) highCongestion++;
    }
    
    Serial.printf("🟢 Low congestion channels: %d\n", lowCongestion);
    Serial.printf("🟡 Medium congestion channels: %d\n", mediumCongestion);
    Serial.printf("🔴 High congestion channels: %d\n", highCongestion);
    Serial.printf("📊 Overall spectrum utilization: %.1f%%\n", results.overall_congestion);
    
    // Show top 3 busiest channels
    std::vector<std::pair<uint8_t, uint8_t>> busyChannels;
    for (int ch = 1; ch <= 13; ch++) {
        if (results.channels[ch].network_count > 0) {
            busyChannels.push_back({ch, results.channels[ch].network_count});
        }
    }
    
    std::sort(busyChannels.begin(), busyChannels.end(),
              [](const std::pair<uint8_t, uint8_t>& a, const std::pair<uint8_t, uint8_t>& b) { 
                  return a.second > b.second; 
              });
    
    Serial.println("\n📈 Busiest Channels:");
    int count = 0;
    for (const std::pair<uint8_t, uint8_t>& pair : busyChannels) {
        if (count >= 3) break;
        Serial.printf("  %d. Channel %d: %d networks\n", 
                      count + 1, pair.first, pair.second);
        count++;
    }
    Serial.println();
}

void printChannelRecommendations(const ChannelAnalysisResults& results) {
    Serial.println("💡 === Channel Recommendations ===");
    
    std::vector<uint8_t> recommended = recommendChannels(results);
    
    Serial.println("🎯 Recommended channels for AP deployment (in order of preference):");
    for (size_t i = 0; i < recommended.size() && i < 5; i++) {
        uint8_t ch = recommended[i];
        const ChannelCongestionData& data = results.channels[ch];
        Serial.printf("  %zu. Channel %d - %.1f%% congestion, %d networks, %s MHz\n",
                      i + 1, ch, data.congestion_score, data.network_count, 
                      String(getChannelFrequency(ch)).c_str());
    }
    
    // Explain why these channels are recommended
    Serial.println("\n📋 Recommendation factors:");
    Serial.println("  • Channels 1, 6, 11 are preferred (non-overlapping)");
    Serial.println("  • Lower congestion scores are better");
    Serial.println("  • Fewer networks mean less competition");
    Serial.println("  • Consider signal strength and overlap\n");
}

// ==========================================
// UTILITY FUNCTIONS
// ==========================================
ChannelScanConfig getDefaultChannelScanConfig() {
    ChannelScanConfig config;
    config.include_hidden_networks = true;
    config.scan_duration_ms = 3000;
    config.detailed_analysis = true;
    config.continuous_monitoring = false;
    config.monitoring_interval_sec = 30;
    return config;
}

void detectInterference(ChannelAnalysisResults& results) {
    // Simple interference detection based on unusual signal patterns
    results.interference_detected = false;
    
    for (int ch = 1; ch <= 13; ch++) {
        const ChannelCongestionData& data = results.channels[ch];
        
        // Check for potential interference indicators
        if (data.network_count > 0) {
            // Large gap between strongest and average RSSI might indicate interference
            int32_t rssiGap = data.strongest_rssi - data.average_rssi;
            if (rssiGap > 30) { // More than 30dBm difference
                results.interference_detected = true;
            }
            
            // Very high congestion on non-standard channels
            if ((ch != 1 && ch != 6 && ch != 11) && data.congestion_score > 80) {
                results.interference_detected = true;
            }
        }
    }
}

float getChannelUtilization(uint8_t channel) {
    if (!isValidChannel(channel)) return 0;
    return lastChannelAnalysis.channels[channel].congestion_score;
}

String generateChannelOptimizationReport(const ChannelAnalysisResults& results) {
    String report = "📊 Channel Optimization Report\n";
    report += "==============================\n\n";
    
    report += "Current Spectrum Conditions:\n";
    report += "- Total networks detected: " + String(results.total_networks) + "\n";
    report += "- Overall congestion: " + String(results.overall_congestion, 1) + "%\n";
    report += "- Best channel: " + String(results.best_channel_2g4) + "\n";
    report += "- Worst channel: " + String(results.worst_channel_2g4) + "\n\n";
    
    std::vector<uint8_t> recommended = recommendChannels(results);
    report += "Optimization Recommendations:\n";
    
    if (recommended.size() > 0) {
        report += "1. Use channel " + String(recommended[0]) + " for best performance\n";
        
        if (recommended.size() > 1) {
            report += "2. Alternative: channel " + String(recommended[1]) + "\n";
        }
        
        if (recommended.size() > 2) {
            report += "3. Backup option: channel " + String(recommended[2]) + "\n";
        }
    }
    
    report += "\nAdditional Tips:\n";
    report += "- Avoid channels with >70% congestion\n";
    report += "- Monitor performance after changes\n";
    report += "- Consider 5GHz if available\n";
    
    return report;
}

String getChannelMonitoringStatus() {
    String status = "Channel Monitoring: ";
    
    if (channelMonitoringActive) {
        unsigned long elapsed = (millis() - lastMonitoringUpdate) / 1000;
        status += "ACTIVE (interval: " + String(monitoringInterval) + "s, ";
        status += "last update: " + String(elapsed) + "s ago)";
    } else {
        status += "INACTIVE";
    }
    
    if (lastChannelScan > 0) {
        unsigned long lastScanAge = (millis() - lastChannelScan) / 1000;
        status += " | Last scan: " + String(lastScanAge) + "s ago";
    }
    
    return status;
}

bool isValidChannel(uint8_t channel) {
    return (channel >= 1 && channel <= 13);
}

uint16_t getChannelFrequency(uint8_t channel) {
    if (channel >= 1 && channel <= 13) {
        return channelFrequencies[channel];
    }
    return 0;
}

bool channelsOverlap(uint8_t channel1, uint8_t channel2) {
    if (!isValidChannel(channel1) || !isValidChannel(channel2)) {
        return false;
    }
    
    // WiFi channels overlap if they are within ±2 channels
    return abs(channel1 - channel2) <= 2;
}

ChannelAnalysisResults getLastChannelAnalysis() {
    return lastChannelAnalysis;
}

void resetChannelAnalysis() {
    memset(&lastChannelAnalysis, 0, sizeof(ChannelAnalysisResults));
    lastChannelScan = 0;
}

String exportChannelAnalysisToJSON(const ChannelAnalysisResults& results) {
    String json = "{";
    json += "\"timestamp\":" + String(results.scan_timestamp) + ",";
    json += "\"scan_duration_ms\":" + String(results.scan_duration_ms) + ",";
    json += "\"total_networks\":" + String(results.total_networks) + ",";
    json += "\"overall_congestion\":" + String(results.overall_congestion) + ",";
    json += "\"best_channel\":" + String(results.best_channel_2g4) + ",";
    json += "\"worst_channel\":" + String(results.worst_channel_2g4) + ",";
    json += "\"interference_detected\":" + String(results.interference_detected ? "true" : "false") + ",";
    
    json += "\"channels\":[";
    for (int ch = 1; ch <= 13; ch++) {
        if (ch > 1) json += ",";
        const ChannelCongestionData& data = results.channels[ch];
        json += "{";
        json += "\"channel\":" + String(ch) + ",";
        json += "\"network_count\":" + String(data.network_count) + ",";
        json += "\"congestion_score\":" + String(data.congestion_score) + ",";
        json += "\"strongest_rssi\":" + String(data.strongest_rssi) + ",";
        json += "\"average_rssi\":" + String(data.average_rssi) + ",";
        json += "\"overlapping_networks\":" + String(data.overlapping_networks) + ",";
        json += "\"is_recommended\":" + String(data.is_recommended ? "true" : "false") + ",";
        json += "\"dominant_network\":\"" + data.dominant_network + "\"";
        json += "}";
    }
    json += "]";
    json += "}";
    
    return json;
}