#pragma once

#include <Arduino.h>
#include "config.h"

#ifdef USE_RTOS
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

// ==========================================
// EVENT MANAGER
// ==========================================

/**
 * @file event_manager.h
 * @brief RTOS Event Group Management System
 * 
 * Provides centralized management of FreeRTOS event groups
 * for system-wide event signaling and synchronization.
 * 
 * @version 4.1.0
 * @date 2025-10-17
 */

// ==========================================
// EVENT BIT DEFINITIONS
// ==========================================

/**
 * @brief System event bits
 * 
 * Each bit represents a system-wide state or event that
 * tasks can wait for or signal.
 */
#define WIFI_CONNECTED_BIT      (1 << 0)  ///< WiFi connected to network
#define WIFI_SCANNING_BIT       (1 << 1)  ///< WiFi scan in progress
#define WEB_SERVER_RUNNING_BIT  (1 << 2)  ///< Web server is running
#define ANALYSIS_RUNNING_BIT    (1 << 3)  ///< Analysis operation in progress
#define AP_MODE_ACTIVE_BIT      (1 << 4)  ///< Access Point mode active

#define ALL_EVENT_BITS          (WIFI_CONNECTED_BIT | \
                                 WIFI_SCANNING_BIT | \
                                 WEB_SERVER_RUNNING_BIT | \
                                 ANALYSIS_RUNNING_BIT | \
                                 AP_MODE_ACTIVE_BIT)

// ==========================================
// EVENT GROUP HANDLE
// ==========================================

extern EventGroupHandle_t systemEventGroup;

// ==========================================
// EVENT WAIT CONFIGURATION
// ==========================================

#define EVENT_WAIT_TIMEOUT_MS 5000

// ==========================================
// CORE FUNCTIONS
// ==========================================

/**
 * @brief Initialize event manager
 * 
 * Creates the system event group.
 * 
 * @return true if initialized successfully, false otherwise
 */
bool initializeEventManager();

/**
 * @brief Shutdown event manager
 * 
 * Deletes the event group and frees resources.
 * 
 * @return true if shutdown successful, false otherwise
 */
bool shutdownEventManager();

/**
 * @brief Check if event manager is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool isEventManagerInitialized();

// ==========================================
// EVENT BIT OPERATIONS
// ==========================================

/**
 * @brief Set event bit(s)
 * 
 * Sets one or more event bits to signal events to waiting tasks.
 * 
 * @param bits Event bit(s) to set (can be ORed together)
 * @return Event group value after setting bits
 */
EventBits_t setEventBits(EventBits_t bits);

/**
 * @brief Clear event bit(s)
 * 
 * Clears one or more event bits.
 * 
 * @param bits Event bit(s) to clear (can be ORed together)
 * @return Event group value after clearing bits
 */
EventBits_t clearEventBits(EventBits_t bits);

/**
 * @brief Get current event bits
 * 
 * Returns the current value of all event bits.
 * 
 * @return Current event group value
 */
EventBits_t getEventBits();

/**
 * @brief Check if specific bit(s) are set
 * 
 * @param bits Event bit(s) to check
 * @return true if all specified bits are set, false otherwise
 */
bool isEventBitSet(EventBits_t bits);

/**
 * @brief Check if any of the specified bits are set
 * 
 * @param bits Event bit(s) to check
 * @return true if any of the specified bits are set, false otherwise
 */
bool isAnyEventBitSet(EventBits_t bits);

// ==========================================
// EVENT WAITING FUNCTIONS
// ==========================================

/**
 * @brief Wait for event bit(s) to be set
 * 
 * Blocks until all specified bits are set or timeout occurs.
 * Automatically clears the bits after successful wait.
 * 
 * @param bits Event bit(s) to wait for (all must be set)
 * @param clearOnExit If true, clear bits after successful wait
 * @param timeoutMs Maximum time to wait in milliseconds
 * @return Event bits that were set, or 0 if timeout
 */
EventBits_t waitForEventBits(EventBits_t bits, 
                             bool clearOnExit = true, 
                             uint32_t timeoutMs = EVENT_WAIT_TIMEOUT_MS);

/**
 * @brief Wait for any event bit(s) to be set
 * 
 * Blocks until any of the specified bits are set or timeout occurs.
 * 
 * @param bits Event bit(s) to wait for (any can be set)
 * @param clearOnExit If true, clear bits after successful wait
 * @param timeoutMs Maximum time to wait in milliseconds
 * @return Event bits that were set, or 0 if timeout
 */
EventBits_t waitForAnyEventBit(EventBits_t bits, 
                               bool clearOnExit = true, 
                               uint32_t timeoutMs = EVENT_WAIT_TIMEOUT_MS);

// ==========================================
// SPECIFIC EVENT FUNCTIONS
// ==========================================

/**
 * @brief Signal WiFi connected
 * 
 * Sets the WIFI_CONNECTED_BIT event.
 */
void signalWiFiConnected();

/**
 * @brief Signal WiFi disconnected
 * 
 * Clears the WIFI_CONNECTED_BIT event.
 */
void signalWiFiDisconnected();

/**
 * @brief Check if WiFi is connected
 * 
 * @return true if WIFI_CONNECTED_BIT is set
 */
bool isWiFiConnectedEvent();

/**
 * @brief Wait for WiFi connection
 * 
 * Blocks until WiFi is connected or timeout.
 * 
 * @param timeoutMs Maximum time to wait in milliseconds
 * @return true if connected, false if timeout
 */
bool waitForWiFiConnection(uint32_t timeoutMs = EVENT_WAIT_TIMEOUT_MS);

/**
 * @brief Signal WiFi scan started
 * 
 * Sets the WIFI_SCANNING_BIT event.
 */
void signalWiFiScanStarted();

/**
 * @brief Signal WiFi scan completed
 * 
 * Clears the WIFI_SCANNING_BIT event.
 */
void signalWiFiScanCompleted();

/**
 * @brief Check if WiFi scan is in progress
 * 
 * @return true if WIFI_SCANNING_BIT is set
 */
bool isWiFiScanningEvent();

/**
 * @brief Wait for WiFi scan to complete
 * 
 * Blocks until scan completes or timeout.
 * 
 * @param timeoutMs Maximum time to wait in milliseconds
 * @return true if scan completed, false if timeout
 */
bool waitForWiFiScanComplete(uint32_t timeoutMs = 10000);

/**
 * @brief Signal web server started
 * 
 * Sets the WEB_SERVER_RUNNING_BIT event.
 */
void signalWebServerStarted();

/**
 * @brief Signal web server stopped
 * 
 * Clears the WEB_SERVER_RUNNING_BIT event.
 */
void signalWebServerStopped();

/**
 * @brief Check if web server is running
 * 
 * @return true if WEB_SERVER_RUNNING_BIT is set
 */
bool isWebServerRunningEvent();

/**
 * @brief Signal analysis started
 * 
 * Sets the ANALYSIS_RUNNING_BIT event.
 */
void signalAnalysisStarted();

/**
 * @brief Signal analysis completed
 * 
 * Clears the ANALYSIS_RUNNING_BIT event.
 */
void signalAnalysisCompleted();

/**
 * @brief Check if analysis is running
 * 
 * @return true if ANALYSIS_RUNNING_BIT is set
 */
bool isAnalysisRunningEvent();

/**
 * @brief Wait for analysis to complete
 * 
 * Blocks until analysis completes or timeout.
 * 
 * @param timeoutMs Maximum time to wait in milliseconds
 * @return true if analysis completed, false if timeout
 */
bool waitForAnalysisComplete(uint32_t timeoutMs = 30000);

/**
 * @brief Signal AP mode started
 * 
 * Sets the AP_MODE_ACTIVE_BIT event.
 */
void signalAPModeStarted();

/**
 * @brief Signal AP mode stopped
 * 
 * Clears the AP_MODE_ACTIVE_BIT event.
 */
void signalAPModeStopped();

/**
 * @brief Check if AP mode is active
 * 
 * @return true if AP_MODE_ACTIVE_BIT is set
 */
bool isAPModeActiveEvent();

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

/**
 * @brief Print current event bits
 * 
 * Prints the current state of all event bits to serial.
 */
void printEventBits();

/**
 * @brief Get event bit name
 * 
 * Returns human-readable name for event bit.
 * 
 * @param bit Event bit to get name for
 * @return Name string
 */
const char* getEventBitName(EventBits_t bit);

/**
 * @brief Clear all event bits
 * 
 * Resets all event bits to 0.
 */
void clearAllEventBits();

#endif // USE_RTOS
