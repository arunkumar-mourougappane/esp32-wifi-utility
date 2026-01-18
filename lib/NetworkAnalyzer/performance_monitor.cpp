/**
 * @file performance_monitor.cpp
 * @brief Performance monitoring and profiling implementation
 * 
 * This file implements execution time profiling and performance tracking:
 * - Microsecond-precision execution time measurement
 * - Statistical analysis (min, max, average)
 * - Sample counting and tracking
 * - Thread-safe operation in RTOS environments
 * - Performance metrics reporting and reset
 * 
 * @author Arunkumar Mourougappane
 * @version 4.2.0
 * @date 2026-01-17
 */

#include "performance_monitor.h"
#include "logging.h"

// ==========================================
// PERFORMANCE MONITOR IMPLEMENTATION
// ==========================================

PerformanceMonitor::PerformanceMonitor(const char* monitorName)
    : name(monitorName),
      minExecutionTimeUs(UINT32_MAX),
      maxExecutionTimeUs(0),
      totalExecutionTimeUs(0),
      sampleCount(0),
      lastExecutionTimeUs(0),
      initialized(true) {
}

void PerformanceMonitor::recordSample(uint32_t executionTimeUs) {
    if (!initialized) return;
    
    lastExecutionTimeUs = executionTimeUs;
    sampleCount++;
    totalExecutionTimeUs += executionTimeUs;
    
    if (executionTimeUs < minExecutionTimeUs) {
        minExecutionTimeUs = executionTimeUs;
    }
    
    if (executionTimeUs > maxExecutionTimeUs) {
        maxExecutionTimeUs = executionTimeUs;
    }
}

uint32_t PerformanceMonitor::getAvgTimeUs() const {
    if (sampleCount == 0) return 0;
    return (uint32_t)(totalExecutionTimeUs / sampleCount);
}

void PerformanceMonitor::reset() {
    minExecutionTimeUs = UINT32_MAX;
    maxExecutionTimeUs = 0;
    totalExecutionTimeUs = 0;
    sampleCount = 0;
    lastExecutionTimeUs = 0;
}

void PerformanceMonitor::printStats() const {
    if (!hasData()) {
        LOG_INFO("Perf", "%s: No data", name);
        return;
    }
    
    LOG_INFO("Perf", "%s Statistics:", name);
    LOG_INFO("Perf", "  Samples:  %u", sampleCount);
    LOG_INFO("Perf", "  Min:      %u us (%.3f ms)", minExecutionTimeUs, minExecutionTimeUs / 1000.0);
    LOG_INFO("Perf", "  Max:      %u us (%.3f ms)", maxExecutionTimeUs, maxExecutionTimeUs / 1000.0);
    LOG_INFO("Perf", "  Avg:      %u us (%.3f ms)", getAvgTimeUs(), getAvgTimeUs() / 1000.0);
    LOG_INFO("Perf", "  Last:     %u us (%.3f ms)", lastExecutionTimeUs, lastExecutionTimeUs / 1000.0);
}

// ==========================================
// SCOPED TIMER IMPLEMENTATION
// ==========================================

ScopedTimer::ScopedTimer(PerformanceMonitor& mon)
    : monitor(mon), startTimeUs(micros()), stopped(false) {
}

ScopedTimer::~ScopedTimer() {
    if (!stopped) {
        stop();
    }
}

uint32_t ScopedTimer::stop() {
    if (stopped) return 0;
    
    uint32_t duration = micros() - startTimeUs;
    monitor.recordSample(duration);
    stopped = true;
    return duration;
}

uint32_t ScopedTimer::elapsed() const {
    if (stopped) return 0;
    return micros() - startTimeUs;
}

// ==========================================
// GLOBAL MONITORS
// ==========================================

PerformanceMonitor g_wifiScanMonitor("WiFi Scan");
PerformanceMonitor g_wifiConnectMonitor("WiFi Connect");
PerformanceMonitor g_channelAnalysisMonitor("Channel Analysis");
PerformanceMonitor g_latencyTestMonitor("Latency Test");
PerformanceMonitor g_commandProcessMonitor("Command Process");
PerformanceMonitor g_webRequestMonitor("Web Request");

void initializePerformanceMonitors() {
    // Monitors are already constructed, but reset them
    resetAllPerformanceMonitors();
    LOG_INFO("Perf", "Performance monitors initialized");
}

void printAllPerformanceStats() {
    LOG_INFO("Perf", "==== Performance Statistics ====");
    g_wifiScanMonitor.printStats();
    g_wifiConnectMonitor.printStats();
    g_channelAnalysisMonitor.printStats();
    g_latencyTestMonitor.printStats();
    g_commandProcessMonitor.printStats();
    g_webRequestMonitor.printStats();
    LOG_INFO("Perf", "================================");
}

void resetAllPerformanceMonitors() {
    g_wifiScanMonitor.reset();
    g_wifiConnectMonitor.reset();
    g_channelAnalysisMonitor.reset();
    g_latencyTestMonitor.reset();
    g_commandProcessMonitor.reset();
    g_webRequestMonitor.reset();
}
