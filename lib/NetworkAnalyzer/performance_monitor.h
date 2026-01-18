/**
 * @file performance_monitor.h
 * @brief Performance monitoring and profiling utilities
 * 
 * Provides tools to measure and track execution times, CPU usage,
 * and performance bottlenecks. Supports thread-safe RTOS monitoring
 * and statistical analysis of execution metrics.
 * 
 * @author Arunkumar Mourougappane
 * @version 4.2.0
 * @date 2026-01-17
 */

#pragma once

#include <Arduino.h>

// ==========================================
// PERFORMANCE MONITORING SYSTEM
// ==========================================

/**
 * @file performance_monitor.h
 * @brief Performance monitoring and profiling utilities
 * 
 * Provides tools to measure and track execution times,
 * CPU usage, and performance bottlenecks.
 * 
 * @version 4.2.0
 * @date 2025-10-25
 */

// ==========================================
// PERFORMANCE MONITOR CLASS
// ==========================================

/**
 * @brief Performance monitor for tracking execution metrics
 * 
 * Tracks min, max, average execution times and sample counts.
 * Thread-safe in RTOS mode.
 */
class PerformanceMonitor {
private:
    const char* name;
    uint32_t minExecutionTimeUs;
    uint32_t maxExecutionTimeUs;
    uint64_t totalExecutionTimeUs;
    uint32_t sampleCount;
    uint32_t lastExecutionTimeUs;
    bool initialized;
    
public:
    /**
     * @brief Constructor
     * 
     * @param monitorName Name for this monitor (for logging)
     */
    PerformanceMonitor(const char* monitorName = "Unknown");
    
    /**
     * @brief Record a sample execution time
     * 
     * @param executionTimeUs Execution time in microseconds
     */
    void recordSample(uint32_t executionTimeUs);
    
    /**
     * @brief Get minimum execution time
     * 
     * @return Minimum time in microseconds
     */
    uint32_t getMinTimeUs() const { return minExecutionTimeUs; }
    
    /**
     * @brief Get maximum execution time
     * 
     * @return Maximum time in microseconds
     */
    uint32_t getMaxTimeUs() const { return maxExecutionTimeUs; }
    
    /**
     * @brief Get average execution time
     * 
     * @return Average time in microseconds
     */
    uint32_t getAvgTimeUs() const;
    
    /**
     * @brief Get last execution time
     * 
     * @return Last recorded time in microseconds
     */
    uint32_t getLastTimeUs() const { return lastExecutionTimeUs; }
    
    /**
     * @brief Get sample count
     * 
     * @return Number of samples recorded
     */
    uint32_t getSampleCount() const { return sampleCount; }
    
    /**
     * @brief Reset all statistics
     */
    void reset();
    
    /**
     * @brief Print statistics to serial
     */
    void printStats() const;
    
    /**
     * @brief Check if monitor has data
     * 
     * @return True if at least one sample recorded
     */
    bool hasData() const { return sampleCount > 0; }
};

// ==========================================
// SCOPED TIMER CLASS
// ==========================================

/**
 * @brief RAII-style timer for automatic measurement
 * 
 * Measures execution time of a scope and automatically
 * records it to a PerformanceMonitor on destruction.
 * 
 * Usage:
 * @code
 * PerformanceMonitor scanMonitor("WiFiScan");
 * {
 *     ScopedTimer timer(scanMonitor);
 *     performWiFiScan();
 * } // Time automatically recorded here
 * @endcode
 */
class ScopedTimer {
private:
    PerformanceMonitor& monitor;
    uint32_t startTimeUs;
    bool stopped;
    
public:
    /**
     * @brief Constructor - starts timing
     * 
     * @param mon Performance monitor to record to
     */
    ScopedTimer(PerformanceMonitor& mon);
    
    /**
     * @brief Destructor - stops timing and records
     */
    ~ScopedTimer();
    
    /**
     * @brief Manually stop timing
     * 
     * @return Elapsed time in microseconds
     */
    uint32_t stop();
    
    /**
     * @brief Get elapsed time without stopping
     * 
     * @return Elapsed time in microseconds
     */
    uint32_t elapsed() const;
    
    // Prevent copying
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
};

// ==========================================
// CONVENIENCE MACROS
// ==========================================

/**
 * @brief Measure execution time of a code block
 * 
 * Usage:
 * @code
 * MEASURE_PERFORMANCE(myMonitor, {
 *     // Code to measure
 *     doExpensiveOperation();
 * });
 * @endcode
 */
#define MEASURE_PERFORMANCE(monitor, code) \
    do { \
        ScopedTimer _timer(monitor); \
        code \
    } while(0)

/**
 * @brief Measure and log execution time
 * 
 * Usage:
 * @code
 * MEASURE_AND_LOG("WiFiScan", {
 *     performWiFiScan();
 * });
 * @endcode
 */
#define MEASURE_AND_LOG(name, code) \
    do { \
        uint32_t _start = micros(); \
        code \
        uint32_t _duration = micros() - _start; \
        LOG_DEBUG("Perf", "%s took %u us", name, _duration); \
    } while(0)

// ==========================================
// GLOBAL PERFORMANCE MONITORS
// ==========================================

// Pre-defined monitors for common operations
extern PerformanceMonitor g_wifiScanMonitor;
extern PerformanceMonitor g_wifiConnectMonitor;
extern PerformanceMonitor g_channelAnalysisMonitor;
extern PerformanceMonitor g_latencyTestMonitor;
extern PerformanceMonitor g_commandProcessMonitor;
extern PerformanceMonitor g_webRequestMonitor;

/**
 * @brief Initialize all global monitors
 */
void initializePerformanceMonitors();

/**
 * @brief Print statistics for all monitors
 */
void printAllPerformanceStats();

/**
 * @brief Reset all monitors
 */
void resetAllPerformanceMonitors();
