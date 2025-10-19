#pragma once

#include <Arduino.h>
#include "task_base.h"
#include "queue_manager.h"
#include "config.h"
#include "channel_analyzer.h"
#include "latency_analyzer.h"
#include "iperf_manager.h"

#ifdef USE_RTOS

// ==========================================
// CONSTANTS
// ==========================================
#define ANALYSIS_REQUEST_QUEUE_LENGTH 5
#define ANALYSIS_PROGRESS_QUEUE_LENGTH 10
#define ANALYSIS_RESULT_QUEUE_LENGTH 5
#define MAX_CONCURRENT_ANALYSES 2           // Maximum 2 concurrent analyses

// ==========================================
// ANALYSIS REQUEST STRUCTURES
// ==========================================

/**
 * @brief Analysis operation types
 */
enum class AnalysisType {
    CHANNEL_SCAN,       ///< Channel congestion analysis
    LATENCY_TEST,       ///< Network latency testing
    IPERF_TEST,         ///< iPerf throughput testing
    NONE                ///< No operation
};

/**
 * @brief Analysis request structure
 */
struct AnalysisRequest {
    AnalysisType type;
    uint32_t requestId;         ///< Unique request identifier
    uint32_t timestamp;         ///< Request timestamp
    bool cancellable;           ///< Can this request be cancelled?
    
    // Type-specific parameters (only one will be valid based on type)
    ChannelScanConfig channelConfig;
    LatencyConfig latencyConfig;
    IperfConfig iperfConfig;
};

/**
 * @brief Analysis progress update structure
 */
struct AnalysisProgress {
    AnalysisType type;
    uint32_t requestId;
    uint8_t progressPercent;    ///< 0-100
    String statusMessage;
    unsigned long timestamp;
};

/**
 * @brief Analysis task result structure
 */
struct AnalysisTaskResult {
    AnalysisType type;
    uint32_t requestId;
    bool success;
    String errorMessage;
    unsigned long duration_ms;
    unsigned long timestamp;
    
    // Type-specific results (only one will be valid based on type)
    ChannelAnalysisResults channelResults;
    LatencyTestResults latencyResults;
    IperfResults iperfResults;
};

/**
 * @brief Analysis task state
 */
enum class AnalysisTaskState {
    UNINITIALIZED,      ///< Task not initialized
    IDLE,               ///< Task running, waiting for requests
    PROCESSING,         ///< Processing analysis request
    ERROR               ///< Error state
};

// ==========================================
// ANALYSIS TASK CLASS
// ==========================================

/**
 * @brief Analysis Task
 * 
 * Manages long-running analysis operations in background with progress reporting.
 * Supports concurrent execution (max 2) and cancellation.
 */
class AnalysisTask : public TaskBase {
private:
    // State management
    AnalysisTaskState currentState;
    uint32_t nextRequestId;
    
    // Concurrent operation management
    SemaphoreHandle_t concurrentOpsSemaphore;  // Counting semaphore (max 2)
    uint8_t activeOperations;
    
    // Request and result queues
    QueueHandle_t requestQueue;
    QueueHandle_t progressQueue;
    QueueHandle_t resultQueue;
    
    // Active operations tracking
    struct ActiveOperation {
        bool active;
        AnalysisType type;
        uint32_t requestId;
        unsigned long startTime;
        bool cancellationRequested;
    };
    ActiveOperation activeOps[MAX_CONCURRENT_ANALYSES];
    
    // Request processing
    void processRequestQueue();
    bool processRequest(const AnalysisRequest& request);
    int findFreeSlot();
    int findActiveSlot(uint32_t requestId);
    void releaseSlot(int slot);
    
    // Analysis handlers
    bool handleChannelScan(const AnalysisRequest& request, int slot);
    bool handleLatencyTest(const AnalysisRequest& request, int slot);
    bool handleIPerfTest(const AnalysisRequest& request, int slot);
    
    // Progress reporting
    void reportProgress(uint32_t requestId, AnalysisType type, uint8_t percent, const String& message);
    void reportResult(const AnalysisTaskResult& result);
    
    // Cancellation
    bool isCancellationRequested(int slot);
    void markCancelled(int slot);
    
    // State transitions
    void transitionState(AnalysisTaskState newState);
    const char* stateToString(AnalysisTaskState state);
    const char* analysisTypeToString(AnalysisType type);

protected:
    /**
     * @brief Task setup - Initialize analysis handling
     */
    void setup() override;
    
    /**
     * @brief Task main loop - Process analysis requests
     */
    void loop() override;
    
    /**
     * @brief Task cleanup
     */
    void cleanup() override;

public:
    /**
     * @brief Constructor
     * @param name Task name
     * @param stackSize Stack size in bytes (default 12288)
     * @param priority Task priority (default MEDIUM)
     * @param core Core to run on (default 0 - WiFi Core)
     */
    AnalysisTask(const char* name, 
                 uint32_t stackSize = 12288,
                 TaskPriority priority = static_cast<TaskPriority>(2),
                 BaseType_t core = 0);
    
    /**
     * @brief Destructor
     */
    virtual ~AnalysisTask();
    
    /**
     * @brief Queue an analysis request (thread-safe)
     * @param request Request to queue
     * @return Request ID (0 if failed)
     */
    uint32_t queueRequest(const AnalysisRequest& request);
    
    /**
     * @brief Cancel an analysis operation
     * @param requestId Request ID to cancel
     * @return true if cancellation requested
     */
    bool cancelRequest(uint32_t requestId);
    
    /**
     * @brief Get progress update (non-blocking)
     * @param progress Output progress structure
     * @return true if progress available
     */
    bool getProgress(AnalysisProgress& progress);
    
    /**
     * @brief Get result (non-blocking)
     * @param result Output result structure
     * @return true if result available
     */
    bool getResult(AnalysisTaskResult& result);
    
    /**
     * @brief Get current task state
     * @return Current state
     */
    AnalysisTaskState getState() const { return currentState; }
    
    /**
     * @brief Get number of active operations
     * @return Active operation count
     */
    uint8_t getActiveOperationCount() const { return activeOperations; }
    
    /**
     * @brief Get request queue fill level
     * @return Number of requests in queue
     */
    uint32_t getQueuedRequestCount() const;
};

// ==========================================
// GLOBAL ANALYSIS TASK MANAGEMENT
// ==========================================

/**
 * @brief Initialize Analysis Task
 * @return true if initialization successful
 */
bool initializeAnalysisTask();

/**
 * @brief Shutdown Analysis Task
 */
void shutdownAnalysisTask();

/**
 * @brief Check if Analysis Task is running
 * @return true if running
 */
bool isAnalysisTaskRunning();

/**
 * @brief Queue a channel scan analysis
 * @param config Channel scan configuration
 * @return Request ID (0 if failed)
 */
uint32_t queueChannelScan(const ChannelScanConfig& config);

/**
 * @brief Queue a latency test
 * @param config Latency test configuration
 * @return Request ID (0 if failed)
 */
uint32_t queueLatencyTest(const LatencyConfig& config);

/**
 * @brief Queue an iPerf test
 * @param config iPerf test configuration
 * @return Request ID (0 if failed)
 */
uint32_t queueIPerfTest(const IperfConfig& config);

/**
 * @brief Cancel an analysis operation
 * @param requestId Request ID to cancel
 * @return true if cancellation requested
 */
bool cancelAnalysisRequest(uint32_t requestId);

/**
 * @brief Get analysis progress (non-blocking)
 * @param progress Output progress structure
 * @return true if progress available
 */
bool getAnalysisProgress(AnalysisProgress& progress);

/**
 * @brief Get analysis result (non-blocking)
 * @param result Output result structure
 * @return true if result available
 */
bool getAnalysisResult(AnalysisTaskResult& result);

/**
 * @brief Get Analysis Task state
 * @return Current state
 */
AnalysisTaskState getAnalysisTaskState();

/**
 * @brief Get active operation count
 * @return Number of active operations
 */
uint8_t getActiveAnalysisCount();

/**
 * @brief Get queued request count
 * @return Number of requests in queue
 */
uint32_t getQueuedAnalysisCount();

#endif // USE_RTOS
