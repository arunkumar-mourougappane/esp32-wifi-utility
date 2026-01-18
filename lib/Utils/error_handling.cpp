/**
 * @file error_handling.cpp
 * @brief Unified error handling implementation
 * 
 * This file implements error handling functionality:
 * - Error code to human-readable message mapping
 * - Centralized error reporting
 * - Error severity categorization
 * - System-wide error code definitions
 * 
 * @author Arunkumar Mourougappane
 * @version 4.2.0
 * @date 2026-01-17
 */

#include "error_handling.h"

// ==========================================
// ERROR MESSAGE MAPPING
// ==========================================

const char* getErrorMessage(ErrorCode code) {
    switch (code) {
        case ErrorCode::OK:
            return "Success";
            
        // General errors
        case ErrorCode::UNKNOWN_ERROR:
            return "Unknown error occurred";
        case ErrorCode::INVALID_PARAMETER:
            return "Invalid parameter";
        case ErrorCode::TIMEOUT:
            return "Operation timed out";
        case ErrorCode::OUT_OF_MEMORY:
            return "Out of memory";
        case ErrorCode::NOT_INITIALIZED:
            return "Component not initialized";
        case ErrorCode::ALREADY_INITIALIZED:
            return "Component already initialized";
            
        // WiFi errors
        case ErrorCode::WIFI_INIT_FAILED:
            return "WiFi initialization failed";
        case ErrorCode::WIFI_SCAN_FAILED:
            return "WiFi scan failed";
        case ErrorCode::WIFI_CONNECT_FAILED:
            return "WiFi connection failed";
        case ErrorCode::WIFI_DISCONNECT_FAILED:
            return "WiFi disconnect failed";
        case ErrorCode::WIFI_AP_START_FAILED:
            return "Access Point start failed";
        case ErrorCode::WIFI_AP_STOP_FAILED:
            return "Access Point stop failed";
        case ErrorCode::WIFI_INVALID_SSID:
            return "Invalid WiFi SSID";
        case ErrorCode::WIFI_INVALID_PASSWORD:
            return "Invalid WiFi password";
            
        // Network errors
        case ErrorCode::NETWORK_CONNECTION_FAILED:
            return "Network connection failed";
        case ErrorCode::NETWORK_TIMEOUT:
            return "Network operation timed out";
        case ErrorCode::DNS_RESOLUTION_FAILED:
            return "DNS resolution failed";
        case ErrorCode::SOCKET_ERROR:
            return "Socket error";
        case ErrorCode::IPERF_INIT_FAILED:
            return "iPerf initialization failed";
        case ErrorCode::IPERF_START_FAILED:
            return "iPerf start failed";
            
        // Hardware errors
        case ErrorCode::LED_INIT_FAILED:
            return "LED initialization failed";
        case ErrorCode::NEOPIXEL_INIT_FAILED:
            return "NeoPixel initialization failed";
        case ErrorCode::PIN_CONFIG_ERROR:
            return "Pin configuration error";
            
        // Storage errors
        case ErrorCode::NVS_INIT_FAILED:
            return "NVS initialization failed";
        case ErrorCode::NVS_READ_FAILED:
            return "NVS read operation failed";
        case ErrorCode::NVS_WRITE_FAILED:
            return "NVS write operation failed";
        case ErrorCode::CONFIG_LOAD_FAILED:
            return "Configuration load failed";
        case ErrorCode::CONFIG_SAVE_FAILED:
            return "Configuration save failed";
            
        // Web server errors
        case ErrorCode::WEBSERVER_INIT_FAILED:
            return "Web server initialization failed";
        case ErrorCode::WEBSERVER_START_FAILED:
            return "Web server start failed";
        case ErrorCode::WEBSERVER_STOP_FAILED:
            return "Web server stop failed";
        case ErrorCode::WEBSERVER_REQUEST_FAILED:
            return "Web server request failed";
            
        // Analysis errors
        case ErrorCode::CHANNEL_ANALYSIS_FAILED:
            return "Channel analysis failed";
        case ErrorCode::LATENCY_TEST_FAILED:
            return "Latency test failed";
        case ErrorCode::ANALYSIS_TIMEOUT:
            return "Analysis operation timed out";
            
        default:
            return "Unknown error code";
    }
}
