#include "analysis_task.h"

#ifdef USE_RTOS

#include "mutex_manager.h"
#include "queue_manager.h"
#include "rtos_manager.h"
#include "channel_analyzer.h"
#include "config.h"

// ==========================================
// GLOBAL ANALYSIS TASK INSTANCE
// ==========================================
static AnalysisTask* analysisTask = nullptr;

// ==========================================
// ANALYSIS TASK IMPLEMENTATION
// ==========================================

AnalysisTask::AnalysisTask(const char* name, uint32_t stackSize, TaskPriority priority, BaseType_t core)
    : TaskBase(name, stackSize, priority, core),
      currentState(AnalysisTaskState::UNINITIALIZED),
      nextRequestId(1),
      concurrentOpsSemaphore(nullptr),
      activeOperations(0),
      requestQueue(nullptr),
      progressQueue(nullptr),
      resultQueue(nullptr) {
    
    // Create counting semaphore for max 2 concurrent operations
    concurrentOpsSemaphore = xSemaphoreCreateCounting(MAX_CONCURRENT_ANALYSES, MAX_CONCURRENT_ANALYSES);
    if (concurrentOpsSemaphore == nullptr) {
        Serial.println("[AnalysisTask] ERROR: Failed to create counting semaphore");
    }
    
    // Create request queue
    requestQueue = xQueueCreate(ANALYSIS_REQUEST_QUEUE_LENGTH, sizeof(AnalysisRequest));
    if (requestQueue == nullptr) {
        Serial.println("[AnalysisTask] ERROR: Failed to create request queue");
    }
    
    // Create progress queue
    progressQueue = xQueueCreate(ANALYSIS_PROGRESS_QUEUE_LENGTH, sizeof(AnalysisProgress));
    if (progressQueue == nullptr) {
        Serial.println("[AnalysisTask] ERROR: Failed to create progress queue");
    }
    
    // Create result queue
    resultQueue = xQueueCreate(ANALYSIS_RESULT_QUEUE_LENGTH, sizeof(AnalysisTaskResult));
    if (resultQueue == nullptr) {
        Serial.println("[AnalysisTask] ERROR: Failed to create result queue");
    }
    
    // Initialize active operations
    for (int i = 0; i < MAX_CONCURRENT_ANALYSES; i++) {
        activeOps[i].active = false;
        activeOps[i].type = AnalysisType::NONE;
        activeOps[i].requestId = 0;
        activeOps[i].startTime = 0;
        activeOps[i].cancellationRequested = false;
    }
}

AnalysisTask::~AnalysisTask() {
    if (concurrentOpsSemaphore != nullptr) {
        vSemaphoreDelete(concurrentOpsSemaphore);
        concurrentOpsSemaphore = nullptr;
    }
    
    if (requestQueue != nullptr) {
        vQueueDelete(requestQueue);
        requestQueue = nullptr;
    }
    
    if (progressQueue != nullptr) {
        vQueueDelete(progressQueue);
        progressQueue = nullptr;
    }
    
    if (resultQueue != nullptr) {
        vQueueDelete(resultQueue);
        resultQueue = nullptr;
    }
}

void AnalysisTask::setup() {
    Serial.println("[AnalysisTask] Initializing...");
    
    if (requestQueue == nullptr || progressQueue == nullptr || resultQueue == nullptr) {
        Serial.println("[AnalysisTask] ERROR: Queues not created");
        transitionState(AnalysisTaskState::ERROR);
        return;
    }
    
    if (concurrentOpsSemaphore == nullptr) {
        Serial.println("[AnalysisTask] ERROR: Semaphore not created");
        transitionState(AnalysisTaskState::ERROR);
        return;
    }
    
    // Transition to IDLE state
    transitionState(AnalysisTaskState::IDLE);
    
    Serial.println("[AnalysisTask] Initialized successfully");
    Serial.printf("[AnalysisTask] - Request queue: %d slots\n", ANALYSIS_REQUEST_QUEUE_LENGTH);
    Serial.printf("[AnalysisTask] - Progress queue: %d slots\n", ANALYSIS_PROGRESS_QUEUE_LENGTH);
    Serial.printf("[AnalysisTask] - Result queue: %d slots\n", ANALYSIS_RESULT_QUEUE_LENGTH);
    Serial.printf("[AnalysisTask] - Max concurrent operations: %d\n", MAX_CONCURRENT_ANALYSES);
    Serial.printf("[AnalysisTask] - Core: %d (WiFi Core)\n", xPortGetCoreID());
}

void AnalysisTask::loop() {
    // Process request queue
    processRequestQueue();
    
    // Small delay to prevent tight loop
    vTaskDelay(pdMS_TO_TICKS(50));
}

void AnalysisTask::cleanup() {
    Serial.println("[AnalysisTask] Cleaning up...");
    
    // Cancel all active operations
    for (int i = 0; i < MAX_CONCURRENT_ANALYSES; i++) {
        if (activeOps[i].active) {
            activeOps[i].cancellationRequested = true;
        }
    }
    
    transitionState(AnalysisTaskState::UNINITIALIZED);
}

// ==========================================
// REQUEST QUEUE PROCESSING
// ==========================================

void AnalysisTask::processRequestQueue() {
    AnalysisRequest request;
    
    // Check if there's a request in the queue (non-blocking)
    if (xQueueReceive(requestQueue, &request, 0) == pdTRUE) {
        Serial.printf("[AnalysisTask] Processing request %u: %s\n", 
                     request.requestId, 
                     analysisTypeToString(request.type));
        
        // Try to acquire semaphore (wait up to 100ms)
        if (xSemaphoreTake(concurrentOpsSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
            transitionState(AnalysisTaskState::PROCESSING);
            
            // Find free slot
            int slot = findFreeSlot();
            if (slot >= 0) {
                // Mark slot as active
                activeOps[slot].active = true;
                activeOps[slot].type = request.type;
                activeOps[slot].requestId = request.requestId;
                activeOps[slot].startTime = millis();
                activeOps[slot].cancellationRequested = false;
                activeOperations++;
                
                // Process the request in background
                bool success = processRequest(request);
                
                if (!success && !isCancellationRequested(slot)) {
                    Serial.printf("[AnalysisTask] WARNING: Request %u processing failed\n", request.requestId);
                }
                
                // Release the slot
                releaseSlot(slot);
            } else {
                Serial.println("[AnalysisTask] ERROR: No free slot available");
                xSemaphoreGive(concurrentOpsSemaphore);
            }
            
            transitionState(AnalysisTaskState::IDLE);
        } else {
            // Couldn't acquire semaphore, put request back
            Serial.println("[AnalysisTask] All slots busy, requeueing request");
            xQueueSendToFront(requestQueue, &request, 0);
        }
    }
}

bool AnalysisTask::processRequest(const AnalysisRequest& request) {
    // Route request to appropriate handler
    switch (request.type) {
        case AnalysisType::CHANNEL_SCAN:
            return handleChannelScan(request, findActiveSlot(request.requestId));
            
        case AnalysisType::LATENCY_TEST:
            return handleLatencyTest(request, findActiveSlot(request.requestId));
            
        case AnalysisType::IPERF_TEST:
            return handleIPerfTest(request, findActiveSlot(request.requestId));
            
        default:
            Serial.printf("[AnalysisTask] ERROR: Unknown request type %d\n", (int)request.type);
            return false;
    }
}

int AnalysisTask::findFreeSlot() {
    for (int i = 0; i < MAX_CONCURRENT_ANALYSES; i++) {
        if (!activeOps[i].active) {
            return i;
        }
    }
    return -1;
}

int AnalysisTask::findActiveSlot(uint32_t requestId) {
    for (int i = 0; i < MAX_CONCURRENT_ANALYSES; i++) {
        if (activeOps[i].active && activeOps[i].requestId == requestId) {
            return i;
        }
    }
    return -1;
}

void AnalysisTask::releaseSlot(int slot) {
    if (slot >= 0 && slot < MAX_CONCURRENT_ANALYSES) {
        activeOps[slot].active = false;
        activeOps[slot].type = AnalysisType::NONE;
        activeOps[slot].requestId = 0;
        activeOps[slot].cancellationRequested = false;
        activeOperations--;
        
        // Release semaphore
        xSemaphoreGive(concurrentOpsSemaphore);
    }
}

// ==========================================
// ANALYSIS HANDLERS
// ==========================================

bool AnalysisTask::handleChannelScan(const AnalysisRequest& request, int slot) {
    reportProgress(request.requestId, AnalysisType::CHANNEL_SCAN, 0, "Starting channel scan");
    
    AnalysisTaskResult result;
    result.type = AnalysisType::CHANNEL_SCAN;
    result.requestId = request.requestId;
    result.timestamp = millis();
    unsigned long startTime = millis();
    
    // Perform channel scan
    reportProgress(request.requestId, AnalysisType::CHANNEL_SCAN, 25, "Scanning channels");
    
    ChannelAnalysisResults scanResults = performChannelCongestionScan(request.channelConfig);
    
    // Check for cancellation
    if (isCancellationRequested(slot)) {
        reportProgress(request.requestId, AnalysisType::CHANNEL_SCAN, 100, "Cancelled");
        result.success = false;
        result.errorMessage = "Operation cancelled";
        result.duration_ms = millis() - startTime;
        reportResult(result);
        return false;
    }
    
    reportProgress(request.requestId, AnalysisType::CHANNEL_SCAN, 75, "Analyzing results");
    
    // Copy results
    result.channelResults = scanResults;
    result.success = true;
    result.duration_ms = millis() - startTime;
    
    reportProgress(request.requestId, AnalysisType::CHANNEL_SCAN, 100, "Complete");
    reportResult(result);
    
    return true;
}

bool AnalysisTask::handleLatencyTest(const AnalysisRequest& request, int slot) {
    reportProgress(request.requestId, AnalysisType::LATENCY_TEST, 0, "Starting latency test");
    
    AnalysisTaskResult result;
    result.type = AnalysisType::LATENCY_TEST;
    result.requestId = request.requestId;
    result.timestamp = millis();
    unsigned long startTime = millis();
    
    // Start latency test
    bool testStarted = startLatencyTest(request.latencyConfig);
    if (!testStarted) {
        result.success = false;
        result.errorMessage = "Failed to start latency test";
        result.duration_ms = millis() - startTime;
        reportResult(result);
        return false;
    }
    
    // Monitor progress - use lastLatencyResults which is updated by handleLatencyTasks()
    uint16_t packetCount = request.latencyConfig.packet_count;
    uint16_t lastReported = 0;
    unsigned long testDuration = packetCount * request.latencyConfig.interval_ms;
    unsigned long testStart = millis();
    
    while ((millis() - testStart) < testDuration) {
        // Check for cancellation
        if (isCancellationRequested(slot)) {
            stopLatencyTest();
            reportProgress(request.requestId, AnalysisType::LATENCY_TEST, 100, "Cancelled");
            result.success = false;
            result.errorMessage = "Operation cancelled";
            result.duration_ms = millis() - startTime;
            reportResult(result);
            return false;
        }
        
        // Call handleLatencyTasks() to process test
        handleLatencyTasks();
        
        // Report progress based on elapsed time
        uint8_t progress = ((millis() - testStart) * 100) / testDuration;
        if (progress > 100) progress = 100;
        
        if (progress > lastReported + 10) {
            reportProgress(request.requestId, AnalysisType::LATENCY_TEST, progress, 
                          String("Progress: ") + progress + "%");
            lastReported = progress;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Get final results
    LatencyTestResults testResults = getLastLatencyResults();
    result.latencyResults = testResults;
    result.success = testResults.test_completed;
    result.duration_ms = millis() - startTime;
    
    reportProgress(request.requestId, AnalysisType::LATENCY_TEST, 100, "Complete");
    reportResult(result);
    
    return result.success;
}

bool AnalysisTask::handleIPerfTest(const AnalysisRequest& request, int slot) {
    reportProgress(request.requestId, AnalysisType::IPERF_TEST, 0, "Starting iPerf test");
    
    AnalysisTaskResult result;
    result.type = AnalysisType::IPERF_TEST;
    result.requestId = request.requestId;
    result.timestamp = millis();
    unsigned long startTime = millis();
    
    // Start iPerf test
    bool testStarted = false;
    if (request.iperfConfig.mode == IPERF_CLIENT) {
        testStarted = startIperfClient(request.iperfConfig);
    } else {
        testStarted = startIperfServer(request.iperfConfig);
    }
    
    if (!testStarted) {
        result.success = false;
        result.errorMessage = "Failed to start iPerf test";
        result.duration_ms = millis() - startTime;
        reportResult(result);
        return false;
    }
    
    // Monitor progress
    int duration = request.iperfConfig.duration;
    unsigned long testStartMs = millis();
    uint8_t lastReported = 0;
    
    while (isIperfRunning()) {
        // Check for cancellation
        if (isCancellationRequested(slot)) {
            stopIperfTest();
            reportProgress(request.requestId, AnalysisType::IPERF_TEST, 100, "Cancelled");
            result.success = false;
            result.errorMessage = "Operation cancelled";
            result.duration_ms = millis() - startTime;
            reportResult(result);
            return false;
        }
        
        // Report progress based on elapsed time
        unsigned long elapsed = millis() - testStartMs;
        uint8_t progress = (elapsed * 100) / (duration * 1000);
        if (progress > 100) progress = 100;
        
        if (progress > lastReported + 10) {
            reportProgress(request.requestId, AnalysisType::IPERF_TEST, progress, 
                          String("Testing: ") + progress + "%");
            lastReported = progress;
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // Get final results
    IperfResults testResults = getIperfResults();
    result.iperfResults = testResults;
    result.success = (testResults.bytesTransferred > 0 && testResults.testCompleted);
    result.duration_ms = millis() - startTime;
    
    reportProgress(request.requestId, AnalysisType::IPERF_TEST, 100, "Complete");
    reportResult(result);
    
    return result.success;
}

// ==========================================
// PROGRESS AND RESULT REPORTING
// ==========================================

void AnalysisTask::reportProgress(uint32_t requestId, AnalysisType type, uint8_t percent, const String& message) {
    AnalysisProgress progress;
    progress.type = type;
    progress.requestId = requestId;
    progress.progressPercent = percent;
    progress.statusMessage = message;
    progress.timestamp = millis();
    
    // Try to send to progress queue (don't block if full)
    if (xQueueSend(progressQueue, &progress, 0) != pdTRUE) {
        Serial.println("[AnalysisTask] WARNING: Progress queue full, dropping update");
    }
}

void AnalysisTask::reportResult(const AnalysisTaskResult& result) {
    // Try to send to result queue (wait up to 100ms)
    if (xQueueSend(resultQueue, &result, pdMS_TO_TICKS(100)) != pdTRUE) {
        Serial.println("[AnalysisTask] WARNING: Result queue full, dropping result");
    }
}

// ==========================================
// CANCELLATION SUPPORT
// ==========================================

bool AnalysisTask::isCancellationRequested(int slot) {
    if (slot >= 0 && slot < MAX_CONCURRENT_ANALYSES) {
        return activeOps[slot].cancellationRequested;
    }
    return false;
}

void AnalysisTask::markCancelled(int slot) {
    if (slot >= 0 && slot < MAX_CONCURRENT_ANALYSES) {
        activeOps[slot].cancellationRequested = true;
    }
}

// ==========================================
// STATE MANAGEMENT
// ==========================================

void AnalysisTask::transitionState(AnalysisTaskState newState) {
    if (currentState != newState) {
        Serial.printf("[AnalysisTask] State: %s -> %s\n", 
                     stateToString(currentState), 
                     stateToString(newState));
        currentState = newState;
    }
}

const char* AnalysisTask::stateToString(AnalysisTaskState state) {
    switch (state) {
        case AnalysisTaskState::UNINITIALIZED:  return "UNINITIALIZED";
        case AnalysisTaskState::IDLE:           return "IDLE";
        case AnalysisTaskState::PROCESSING:     return "PROCESSING";
        case AnalysisTaskState::ERROR:          return "ERROR";
        default:                                return "UNKNOWN";
    }
}

const char* AnalysisTask::analysisTypeToString(AnalysisType type) {
    switch (type) {
        case AnalysisType::CHANNEL_SCAN:    return "CHANNEL_SCAN";
        case AnalysisType::LATENCY_TEST:    return "LATENCY_TEST";
        case AnalysisType::IPERF_TEST:      return "IPERF_TEST";
        case AnalysisType::NONE:            return "NONE";
        default:                            return "UNKNOWN";
    }
}

// ==========================================
// PUBLIC API
// ==========================================

uint32_t AnalysisTask::queueRequest(const AnalysisRequest& request) {
    if (requestQueue == nullptr) {
        Serial.println("[AnalysisTask] ERROR: Request queue not initialized");
        return 0;
    }
    
    // Try to send to queue with 100ms timeout
    if (xQueueSend(requestQueue, &request, pdMS_TO_TICKS(100)) == pdTRUE) {
        return request.requestId;
    } else {
        Serial.println("[AnalysisTask] WARNING: Request queue full, dropping request");
        return 0;
    }
}

bool AnalysisTask::cancelRequest(uint32_t requestId) {
    // Find active operation with this request ID
    for (int i = 0; i < MAX_CONCURRENT_ANALYSES; i++) {
        if (activeOps[i].active && activeOps[i].requestId == requestId) {
            markCancelled(i);
            Serial.printf("[AnalysisTask] Cancellation requested for request %u\n", requestId);
            return true;
        }
    }
    return false;
}

bool AnalysisTask::getProgress(AnalysisProgress& progress) {
    if (progressQueue == nullptr) return false;
    return xQueueReceive(progressQueue, &progress, 0) == pdTRUE;
}

bool AnalysisTask::getResult(AnalysisTaskResult& result) {
    if (resultQueue == nullptr) return false;
    return xQueueReceive(resultQueue, &result, 0) == pdTRUE;
}

uint32_t AnalysisTask::getQueuedRequestCount() const {
    if (requestQueue == nullptr) return 0;
    return uxQueueMessagesWaiting(requestQueue);
}

// ==========================================
// GLOBAL ANALYSIS TASK MANAGEMENT
// ==========================================

bool initializeAnalysisTask() {
    if (analysisTask != nullptr) {
        Serial.println("[AnalysisTask] Already initialized");
        return true;
    }
    
    // Create AnalysisTask instance with MEDIUM priority on Core 0 (WiFi Core)
    analysisTask = new AnalysisTask("AnalysisTask", ANALYSIS_TASK_STACK_SIZE * sizeof(StackType_t), static_cast<TaskPriority>(2), 0);
    if (analysisTask == nullptr) {
        Serial.println("[AnalysisTask] ERROR: Failed to create AnalysisTask instance");
        return false;
    }
    
    // Start the task
    if (!analysisTask->start()) {
        Serial.println("[AnalysisTask] ERROR: Failed to start AnalysisTask");
        delete analysisTask;
        analysisTask = nullptr;
        return false;
    }
    
    Serial.println("[AnalysisTask] Started successfully");
    return true;
}

void shutdownAnalysisTask() {
    if (analysisTask == nullptr) {
        Serial.println("[AnalysisTask] Not running");
        return;
    }
    
    Serial.println("[AnalysisTask] Shutting down...");
    analysisTask->stop();
    delete analysisTask;
    analysisTask = nullptr;
    Serial.println("[AnalysisTask] Shutdown complete");
}

bool isAnalysisTaskRunning() {
    return analysisTask != nullptr && analysisTask->isRunning();
}

uint32_t queueChannelScan(const ChannelScanConfig& config) {
    if (analysisTask == nullptr) {
        Serial.println("[AnalysisTask] ERROR: AnalysisTask not initialized");
        return 0;
    }
    
    AnalysisRequest request;
    request.type = AnalysisType::CHANNEL_SCAN;
    request.requestId = millis();  // Simple request ID
    request.timestamp = millis();
    request.cancellable = true;
    request.channelConfig = config;
    
    return analysisTask->queueRequest(request);
}

uint32_t queueLatencyTest(const LatencyConfig& config) {
    if (analysisTask == nullptr) {
        Serial.println("[AnalysisTask] ERROR: AnalysisTask not initialized");
        return 0;
    }
    
    AnalysisRequest request;
    request.type = AnalysisType::LATENCY_TEST;
    request.requestId = millis() + 1;  // Simple request ID
    request.timestamp = millis();
    request.cancellable = true;
    request.latencyConfig = config;
    
    return analysisTask->queueRequest(request);
}

uint32_t queueIPerfTest(const IperfConfig& config) {
    if (analysisTask == nullptr) {
        Serial.println("[AnalysisTask] ERROR: AnalysisTask not initialized");
        return 0;
    }
    
    AnalysisRequest request;
    request.type = AnalysisType::IPERF_TEST;
    request.requestId = millis() + 2;  // Simple request ID
    request.timestamp = millis();
    request.cancellable = true;
    request.iperfConfig = config;
    
    return analysisTask->queueRequest(request);
}

bool cancelAnalysisRequest(uint32_t requestId) {
    if (analysisTask == nullptr) {
        return false;
    }
    return analysisTask->cancelRequest(requestId);
}

bool getAnalysisProgress(AnalysisProgress& progress) {
    if (analysisTask == nullptr) {
        return false;
    }
    return analysisTask->getProgress(progress);
}

bool getAnalysisResult(AnalysisTaskResult& result) {
    if (analysisTask == nullptr) {
        return false;
    }
    return analysisTask->getResult(result);
}

AnalysisTaskState getAnalysisTaskState() {
    if (analysisTask == nullptr) {
        return AnalysisTaskState::UNINITIALIZED;
    }
    return analysisTask->getState();
}

uint8_t getActiveAnalysisCount() {
    if (analysisTask == nullptr) {
        return 0;
    }
    return analysisTask->getActiveOperationCount();
}

uint32_t getQueuedAnalysisCount() {
    if (analysisTask == nullptr) {
        return 0;
    }
    return analysisTask->getQueuedRequestCount();
}

#endif // USE_RTOS
