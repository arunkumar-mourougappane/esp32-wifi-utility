/**
 * @file logging.cpp
 * @brief Structured logging system implementation
 * 
 * This file implements the logging system:
 * - Configurable log levels (DEBUG, INFO, WARN, ERROR)
 * - Optional timestamps and color output
 * - Component-tagged messages
 * - Printf-style formatted logging
 * - Global log level filtering
 * 
 * @author Arunkumar Mourougappane
 * @version 5.0.0
 * @date 2026-01-17
 */

#include "logging.h"
#include "config.h"

// ==========================================
// GLOBAL STATE
// ==========================================

static LogLevel g_currentLogLevel = LogLevel::INFO;
static bool g_showTimestamps = true;
static bool g_useColors = false;

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_DEBUG   "\033[90m"  // Gray
#define COLOR_INFO    "\033[37m"  // White
#define COLOR_WARN    "\033[33m"  // Yellow
#define COLOR_ERROR   "\033[31m"  // Red

// ==========================================
// CONFIGURATION FUNCTIONS
// ==========================================

void setLogLevel(LogLevel level) {
    g_currentLogLevel = level;
}

LogLevel getLogLevel() {
    return g_currentLogLevel;
}

void setLogTimestamps(bool enable) {
    g_showTimestamps = enable;
}

void setLogColors(bool enable) {
    g_useColors = enable;
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

static const char* getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        default:              return "?????";
    }
}

static const char* getLevelColor(LogLevel level) {
    if (!g_useColors) return "";
    
    switch (level) {
        case LogLevel::DEBUG: return COLOR_DEBUG;
        case LogLevel::INFO:  return COLOR_INFO;
        case LogLevel::WARN:  return COLOR_WARN;
        case LogLevel::ERROR: return COLOR_ERROR;
        default:              return COLOR_RESET;
    }
}

// ==========================================
// CORE LOGGING FUNCTIONS
// ==========================================

void vlog(LogLevel level, const char* tag, const char* format, va_list args) {
    // Check if we should log this message
    if (level < g_currentLogLevel) {
        return;
    }
    
    // Print timestamp if enabled
    if (g_showTimestamps) {
        unsigned long ms = millis();
        unsigned long seconds = ms / 1000;
        unsigned long milliseconds = ms % 1000;
        Serial.printf("[%5lu.%03lu] ", seconds, milliseconds);
    }
    
    // Print level with color
    const char* color = getLevelColor(level);
    const char* levelStr = getLevelString(level);
    
    if (g_useColors) {
        Serial.print(color);
    }
    Serial.printf("[%s]", levelStr);
    if (g_useColors) {
        Serial.print(COLOR_RESET);
    }
    
    // Print tag
    Serial.printf("[%-10s] ", tag);
    
    // Print message
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.println(buffer);
}

void log(LogLevel level, const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(level, tag, format, args);
    va_end(args);
}

// ==========================================
// ADVANCED FEATURES
// ==========================================

void logHexDump(LogLevel level, const char* tag, const void* data, size_t length, const char* message) {
    if (level < g_currentLogLevel) {
        return;
    }
    
    if (message) {
        log(level, tag, "%s (%u bytes):", message, length);
    }
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    const size_t bytesPerLine = 16;
    
    for (size_t i = 0; i < length; i += bytesPerLine) {
        // Print offset
        Serial.printf("  %04X: ", i);
        
        // Print hex values
        for (size_t j = 0; j < bytesPerLine; j++) {
            if (i + j < length) {
                Serial.printf("%02X ", bytes[i + j]);
            } else {
                Serial.print("   ");
            }
            
            // Add extra space in the middle
            if (j == 7) Serial.print(" ");
        }
        
        // Print ASCII representation
        Serial.print(" |");
        for (size_t j = 0; j < bytesPerLine && (i + j) < length; j++) {
            uint8_t c = bytes[i + j];
            Serial.print((c >= 32 && c <= 126) ? (char)c : '.');
        }
        Serial.println("|");
    }
}

void logMemoryStats(const char* tag) {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t heapSize = ESP.getHeapSize();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    
    log(LogLevel::INFO, tag, "Memory Stats:");
    log(LogLevel::INFO, tag, "  Total Heap:     %u bytes (%.1f KB)", 
        heapSize, heapSize / 1024.0);
    log(LogLevel::INFO, tag, "  Free Heap:      %u bytes (%.1f KB, %.1f%%)", 
        freeHeap, freeHeap / 1024.0, (freeHeap * 100.0) / heapSize);
    log(LogLevel::INFO, tag, "  Min Free Heap:  %u bytes (%.1f KB)", 
        minFreeHeap, minFreeHeap / 1024.0);
    log(LogLevel::INFO, tag, "  Used Heap:      %u bytes (%.1f KB)", 
        heapSize - freeHeap, (heapSize - freeHeap) / 1024.0);
}

void logTaskStats(const char* tag) {
    log(LogLevel::WARN, tag, "Task statistics not available");
}
