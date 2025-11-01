#pragma once

#include <Arduino.h>

// ==========================================
// UNIFIED ERROR HANDLING SYSTEM
// ==========================================

/**
 * @file error_handling.h
 * @brief Unified error handling and result types
 * 
 * Provides consistent error handling across all modules
 * with proper error propagation and logging.
 * 
 * @version 4.2.0
 * @date 2025-10-25
 */

/**
 * @brief System-wide error codes
 */
enum class ErrorCode : uint8_t {
    OK = 0,
    
    // General errors (1-9)
    UNKNOWN_ERROR = 1,
    INVALID_PARAMETER = 2,
    TIMEOUT = 3,
    OUT_OF_MEMORY = 4,
    NOT_INITIALIZED = 5,
    ALREADY_INITIALIZED = 6,
    
    // WiFi errors (30-49)
    WIFI_INIT_FAILED = 30,
    WIFI_SCAN_FAILED = 31,
    WIFI_CONNECT_FAILED = 32,
    WIFI_DISCONNECT_FAILED = 33,
    WIFI_AP_START_FAILED = 34,
    WIFI_AP_STOP_FAILED = 35,
    WIFI_INVALID_SSID = 36,
    WIFI_INVALID_PASSWORD = 37,
    
    // Network errors (50-69)
    NETWORK_CONNECTION_FAILED = 50,
    NETWORK_TIMEOUT = 51,
    DNS_RESOLUTION_FAILED = 52,
    SOCKET_ERROR = 53,
    IPERF_INIT_FAILED = 54,
    IPERF_START_FAILED = 55,
    
    // Hardware errors (70-89)
    LED_INIT_FAILED = 70,
    NEOPIXEL_INIT_FAILED = 71,
    PIN_CONFIG_ERROR = 72,
    
    // Storage errors (90-109)
    NVS_INIT_FAILED = 90,
    NVS_READ_FAILED = 91,
    NVS_WRITE_FAILED = 92,
    CONFIG_LOAD_FAILED = 93,
    CONFIG_SAVE_FAILED = 94,
    
    // Web server errors (110-129)
    WEBSERVER_INIT_FAILED = 110,
    WEBSERVER_START_FAILED = 111,
    WEBSERVER_STOP_FAILED = 112,
    WEBSERVER_REQUEST_FAILED = 113,
    
    // Analysis errors (130-149)
    CHANNEL_ANALYSIS_FAILED = 130,
    LATENCY_TEST_FAILED = 131,
    ANALYSIS_TIMEOUT = 132,
};

/**
 * @brief Get human-readable error message
 * 
 * @param code Error code
 * @return Const string with error description
 */
const char* getErrorMessage(ErrorCode code);

// ==========================================
// RESULT TYPE
// ==========================================

/**
 * @brief Result type for operations that can fail
 * 
 * Provides type-safe error handling without exceptions.
 * 
 * @tparam T Type of successful result value
 */
template<typename T = void>
class Result {
private:
    ErrorCode errorCode;
    T value;
    const char* customMessage;
    
public:
    // Success constructor
    Result(T val) : errorCode(ErrorCode::OK), value(val), customMessage(nullptr) {}
    
    // Error constructor
    Result(ErrorCode code, const char* msg = nullptr) 
        : errorCode(code), value(T()), customMessage(msg) {}
    
    // Check if result is successful
    bool isOk() const { return errorCode == ErrorCode::OK; }
    bool isError() const { return errorCode != ErrorCode::OK; }
    
    // Get error code
    ErrorCode getError() const { return errorCode; }
    
    // Get error message
    const char* getMessage() const {
        if (customMessage) return customMessage;
        return getErrorMessage(errorCode);
    }
    
    // Get value (only if successful)
    T getValue() const {
        if (isError()) {
            // In production, this should log error
            return T();
        }
        return value;
    }
    
    // Get value with default fallback
    T getValueOr(T defaultValue) const {
        return isOk() ? value : defaultValue;
    }
    
    // Operator overloads for convenience
    operator bool() const { return isOk(); }
    T operator*() const { return getValue(); }
};

// Specialization for void return type
template<>
class Result<void> {
private:
    ErrorCode errorCode;
    const char* customMessage;
    
public:
    // Success constructor
    Result() : errorCode(ErrorCode::OK), customMessage(nullptr) {}
    
    // Error constructor
    Result(ErrorCode code, const char* msg = nullptr) 
        : errorCode(code), customMessage(msg) {}
    
    // Check if result is successful
    bool isOk() const { return errorCode == ErrorCode::OK; }
    bool isError() const { return errorCode != ErrorCode::OK; }
    
    // Get error code
    ErrorCode getError() const { return errorCode; }
    
    // Get error message
    const char* getMessage() const {
        if (customMessage) return customMessage;
        return getErrorMessage(errorCode);
    }
    
    // Operator overloads
    operator bool() const { return isOk(); }
};

// ==========================================
// HELPER MACROS
// ==========================================

/**
 * @brief Return error if result is not OK
 * 
 * Usage: RETURN_IF_ERROR(someFunction());
 */
#define RETURN_IF_ERROR(expr) \
    do { \
        auto _result = (expr); \
        if (!_result.isOk()) { \
            return _result; \
        } \
    } while(0)

/**
 * @brief Log error if result is not OK
 * 
 * Usage: LOG_IF_ERROR(someFunction());
 */
#define LOG_IF_ERROR(expr) \
    do { \
        auto _result = (expr); \
        if (!_result.isOk()) { \
            log(LogLevel::ERROR, "ErrorCheck", "Error: %s", _result.getMessage()); \
        } \
    } while(0)

// Forward declaration for logging
enum class LogLevel : uint8_t;
void log(LogLevel level, const char* tag, const char* format, ...);
