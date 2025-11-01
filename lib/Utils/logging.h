#pragma once

#include <Arduino.h>
#include <stdarg.h>

// ==========================================
// STRUCTURED LOGGING SYSTEM
// ==========================================

/**
 * @file logging.h
 * @brief Structured logging with configurable levels
 * 
 * Provides centralized logging with different severity levels,
 * timestamps, and categorization by component.
 * 
 * @version 4.2.0
 * @date 2025-10-25
 */

// ==========================================
// LOG LEVELS
// ==========================================

/**
 * @brief Log severity levels
 */
enum class LogLevel : uint8_t {
    DEBUG = 0,   ///< Detailed debugging information
    INFO = 1,    ///< General informational messages
    WARN = 2,    ///< Warning messages
    ERROR = 3,   ///< Error messages
    NONE = 4     ///< Disable all logging
};

// ==========================================
// GLOBAL CONFIGURATION
// ==========================================

/**
 * @brief Set minimum log level
 * 
 * Messages below this level will not be printed.
 * 
 * @param level Minimum log level
 */
void setLogLevel(LogLevel level);

/**
 * @brief Get current log level
 * 
 * @return Current minimum log level
 */
LogLevel getLogLevel();

/**
 * @brief Enable/disable timestamps in logs
 * 
 * @param enable True to show timestamps, false to hide
 */
void setLogTimestamps(bool enable);

/**
 * @brief Enable/disable color output (for terminals that support ANSI)
 * 
 * @param enable True to enable colors, false to disable
 */
void setLogColors(bool enable);

// ==========================================
// LOGGING FUNCTIONS
// ==========================================

/**
 * @brief Log a message with specified level and tag
 * 
 * @param level Log severity level
 * @param tag Component tag (e.g., "WiFi", "RTOS", "LED")
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 */
void log(LogLevel level, const char* tag, const char* format, ...);

/**
 * @brief Log a message with va_list (for internal use)
 * 
 * @param level Log severity level
 * @param tag Component tag
 * @param format Printf-style format string
 * @param args Variable argument list
 */
void vlog(LogLevel level, const char* tag, const char* format, va_list args);

// ==========================================
// CONVENIENCE MACROS
// ==========================================

/**
 * @brief Log debug message
 * 
 * Usage: LOG_DEBUG("WiFi", "Scanning channel %d", channel);
 */
#define LOG_DEBUG(tag, ...) log(LogLevel::DEBUG, tag, __VA_ARGS__)

/**
 * @brief Log info message
 * 
 * Usage: LOG_INFO("System", "Initialization complete");
 */
#define LOG_INFO(tag, ...) log(LogLevel::INFO, tag, __VA_ARGS__)

/**
 * @brief Log warning message
 * 
 * Usage: LOG_WARN("Memory", "Heap low: %d bytes", heapSize);
 */
#define LOG_WARN(tag, ...) log(LogLevel::WARN, tag, __VA_ARGS__)

/**
 * @brief Log error message
 * 
 * Usage: LOG_ERROR("RTOS", "Queue creation failed");
 */
#define LOG_ERROR(tag, ...) log(LogLevel::ERROR, tag, __VA_ARGS__)

// ==========================================
// COMPONENT-SPECIFIC TAGS
// ==========================================

// Standard component tags for consistency
#define TAG_MAIN "Main"
#define TAG_RTOS "RTOS"
#define TAG_WIFI "WiFi"
#define TAG_AP "AP"
#define TAG_WEB "Web"
#define TAG_LED "LED"
#define TAG_CMD "Command"
#define TAG_IPERF "iPerf"
#define TAG_CHANNEL "Channel"
#define TAG_LATENCY "Latency"
#define TAG_QUEUE "Queue"
#define TAG_MUTEX "Mutex"
#define TAG_TASK "Task"
#define TAG_MEMORY "Memory"
#define TAG_CONFIG "Config"

// ==========================================
// ADVANCED FEATURES
// ==========================================

/**
 * @brief Dump hex data to log
 * 
 * @param level Log level
 * @param tag Component tag
 * @param data Pointer to data
 * @param length Data length in bytes
 * @param message Optional message prefix
 */
void logHexDump(LogLevel level, const char* tag, const void* data, size_t length, const char* message = nullptr);

/**
 * @brief Log memory statistics
 * 
 * @param tag Component tag
 */
void logMemoryStats(const char* tag);

/**
 * @brief Log task statistics (RTOS mode only)
 * 
 * @param tag Component tag
 */
void logTaskStats(const char* tag);
