/**
 * @file signal_monitor.h
 * @brief WiFi signal strength monitoring and quality analysis
 * 
 * This header defines structures and functions for monitoring WiFi signal
 * strength (RSSI), calculating signal quality metrics, and tracking
 * connection quality over time. Provides RSSI-to-quality conversion
 * and nearby network signal scanning.
 * 
 * @author Arunkumar Mourougappane
 * @version 4.3.0
 * @date 2026-01-17
 */

#pragma once
#include <Arduino.h>
#include <vector>
#include "config.h"

// ==========================================
// SIGNAL STRENGTH MONITORING
// ==========================================

/**
 * @brief Structure to hold signal strength information
 */
struct SignalInfo {
    String ssid;
    int32_t rssi;           // Signal strength in dBm
    uint8_t quality;        // Signal quality percentage (0-100)
    String qualityText;     // Quality description (Excellent, Good, Fair, Weak, No Signal)
    bool isConnected;       // True if this is the connected network
    unsigned long timestamp;
};

/**
 * @brief Gets current signal strength of connected network
 * @return SignalInfo structure with current connection details
 */
SignalInfo getCurrentSignalStrength();

/**
 * @brief Gets signal strength of all nearby networks
 * @param maxNetworks Maximum number of networks to return (default: 10)
 * @return Array of SignalInfo structures
 */
std::vector<SignalInfo> getNearbySignalStrengths(int maxNetworks = 10);

/**
 * @brief Converts RSSI (dBm) to quality percentage
 * @param rssi Signal strength in dBm
 * @return Quality percentage (0-100)
 */
uint8_t rssiToQuality(int32_t rssi);

/**
 * @brief Converts RSSI to descriptive quality text
 * @param rssi Signal strength in dBm
 * @return Quality description string
 */
String rssiToQualityText(int32_t rssi);

/**
 * @brief Prints signal strength information to Serial
 * @param info SignalInfo structure to display
 */
void printSignalInfo(const SignalInfo& info);

/**
 * @brief Displays signal strength meter in Serial (ASCII art)
 * @param rssi Signal strength in dBm
 */
void displaySignalMeter(int32_t rssi);

/**
 * @brief Starts continuous signal monitoring (prints to Serial every N seconds)
 * @param intervalSeconds Update interval in seconds (default: 5)
 */
void startSignalMonitoring(uint8_t intervalSeconds = 5);

/**
 * @brief Stops continuous signal monitoring
 */
void stopSignalMonitoring();

/**
 * @brief Updates signal monitoring (call in loop)
 */
void updateSignalMonitoring();

// ==========================================
// GLOBAL STATE
// ==========================================
extern bool signalMonitoringEnabled;
extern uint8_t signalMonitoringInterval;
extern unsigned long lastSignalUpdate;
