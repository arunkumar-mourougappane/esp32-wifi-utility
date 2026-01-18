/**
 * @file signal_monitor.cpp
 * @brief WiFi signal strength monitoring implementation
 * 
 * This file implements signal strength monitoring and quality analysis:
 * - RSSI (signal strength) measurement and conversion
 * - Signal quality percentage calculation (0-100%)
 * - Descriptive quality text (Excellent, Good, Fair, Weak)
 * - Real-time monitoring of connected network
 * - Nearby network signal scanning
 * - Continuous monitoring with configurable intervals
 * 
 * @author Arunkumar Mourougappane
 * @version 4.3.0
 * @date 2026-01-17
 */

#include "signal_monitor.h"
#include "logging.h"
#include <WiFi.h>
#include <vector>

#define TAG_SIGNAL "Signal"

// Global state
bool signalMonitoringEnabled = false;
uint8_t signalMonitoringInterval = 5;
unsigned long lastSignalUpdate = 0;

// ==========================================
// RSSI CONVERSION FUNCTIONS
// ==========================================

uint8_t rssiToQuality(int32_t rssi) {
    // RSSI typically ranges from -30 (excellent) to -90 (unusable)
    // Convert to 0-100 scale
    if (rssi >= -50) return 100;
    if (rssi <= -100) return 0;
    return 2 * (rssi + 100);
}

String rssiToQualityText(int32_t rssi) {
    if (rssi >= -50) return "Excellent";
    if (rssi >= -60) return "Good";
    if (rssi >= -70) return "Fair";
    if (rssi >= -80) return "Weak";
    return "Very Weak";
}

// ==========================================
// SIGNAL INFO RETRIEVAL
// ==========================================

SignalInfo getCurrentSignalStrength() {
    SignalInfo info;
    info.timestamp = millis();
    info.isConnected = false;
    
    if (WiFi.status() == WL_CONNECTED) {
        info.ssid = WiFi.SSID();
        info.rssi = WiFi.RSSI();
        info.quality = rssiToQuality(info.rssi);
        info.qualityText = rssiToQualityText(info.rssi);
        info.isConnected = true;
        
        LOG_DEBUG(TAG_SIGNAL, "Connected to %s: %d dBm (%s)", 
                  info.ssid.c_str(), info.rssi, info.qualityText.c_str());
    } else {
        info.ssid = "Not Connected";
        info.rssi = -100;
        info.quality = 0;
        info.qualityText = "No Signal";
        
        LOG_DEBUG(TAG_SIGNAL, "No connection");
    }
    
    return info;
}

std::vector<SignalInfo> getNearbySignalStrengths(int maxNetworks) {
    std::vector<SignalInfo> networks;
    
    LOG_DEBUG(TAG_SIGNAL, "Scanning for nearby networks...");
    
    // Perform scan (show hidden networks, don't show duplicates)
    int n = WiFi.scanNetworks(false, true, false, 300); // async=false, show_hidden=true, passive=false, max_ms_per_chan=300
    
    if (n == 0) {
        LOG_INFO(TAG_SIGNAL, "No networks found");
        return networks;
    }
    
    LOG_INFO(TAG_SIGNAL, "Found %d networks", n);
    
    // Get current connected SSID
    String connectedSSID = WiFi.SSID();
    bool isConnected = (WiFi.status() == WL_CONNECTED);
    
    // Limit to maxNetworks
    int count = (n < maxNetworks) ? n : maxNetworks;
    
    for (int i = 0; i < count; i++) {
        SignalInfo info;
        String ssid = WiFi.SSID(i);
        // Handle hidden networks (empty SSID)
        info.ssid = (ssid.length() > 0) ? ssid : "<Hidden Network>";
        info.rssi = WiFi.RSSI(i);
        info.quality = rssiToQuality(info.rssi);
        info.qualityText = rssiToQualityText(info.rssi);
        info.isConnected = (isConnected && info.ssid == connectedSSID);
        info.timestamp = millis();
        
        networks.push_back(info);
        
        LOG_DEBUG(TAG_SIGNAL, "%d: %s - %d dBm (%s)%s", 
                  i+1, info.ssid.c_str(), info.rssi, info.qualityText.c_str(),
                  info.isConnected ? " [CONNECTED]" : "");
    }
    
    // Clean up scan results
    WiFi.scanDelete();
    
    return networks;
}

// ==========================================
// DISPLAY FUNCTIONS
// ==========================================

void printSignalInfo(const SignalInfo& info) {
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println("Signal Strength Information");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.printf("Network:  %s%s\n", info.ssid.c_str(), 
                  info.isConnected ? " [CONNECTED]" : "");
    Serial.printf("RSSI:     %d dBm\n", info.rssi);
    Serial.printf("Quality:  %d%% (%s)\n", info.quality, info.qualityText.c_str());
    
    // Display signal meter
    displaySignalMeter(info.rssi);
    
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
}

void displaySignalMeter(int32_t rssi) {
    const int barWidth = 40;
    int quality = rssiToQuality(rssi);
    int bars = (quality * barWidth) / 100;
    
    Serial.print("Signal:   [");
    for (int i = 0; i < barWidth; i++) {
        if (i < bars) {
            Serial.print("█");
        } else {
            Serial.print("░");
        }
    }
    Serial.println("]");
}

// ==========================================
// CONTINUOUS MONITORING
// ==========================================

void startSignalMonitoring(uint8_t intervalSeconds) {
    signalMonitoringEnabled = true;
    signalMonitoringInterval = intervalSeconds;
    lastSignalUpdate = 0; // Force immediate update
    
    LOG_INFO(TAG_SIGNAL, "Signal monitoring started (interval: %d seconds)", intervalSeconds);
    Serial.println("Signal monitoring active. Press any key to stop.");
}

void stopSignalMonitoring() {
    signalMonitoringEnabled = false;
    LOG_INFO(TAG_SIGNAL, "Signal monitoring stopped");
}

void updateSignalMonitoring() {
    if (!signalMonitoringEnabled) return;
    
    // Check for stop command
    if (Serial.available() > 0) {
        Serial.read(); // Clear buffer
        stopSignalMonitoring();
        return;
    }
    
    // Check if it's time to update
    unsigned long now = millis();
    if (now - lastSignalUpdate >= (signalMonitoringInterval * 1000)) {
        lastSignalUpdate = now;
        
        // Get and display current signal
        SignalInfo info = getCurrentSignalStrength();
        
        Serial.println();
        Serial.printf("[%02d:%02d:%02d] ", 
                     (int)((now / 3600000) % 24),
                     (int)((now / 60000) % 60),
                     (int)((now / 1000) % 60));
        
        if (info.isConnected) {
            Serial.printf("%s: %d dBm | %d%% | %s\n",
                         info.ssid.c_str(), info.rssi, 
                         info.quality, info.qualityText.c_str());
            
            // Mini meter
            int bars = (info.quality / 10);
            Serial.print("          Signal: [");
            for (int i = 0; i < 10; i++) {
                Serial.print(i < bars ? "█" : "░");
            }
            Serial.println("]");
        } else {
            Serial.println("Not connected to any network");
        }
    }
}
