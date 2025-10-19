# RTOS API Reference

**Version**: 4.1.0  
**Date**: 2025-10-18  
**Status**: Production

## Table of Contents

1. [Introduction](#introduction)
2. [RTOSManager](#rtosmanager)
3. [QueueManager](#queuemanager)
4. [MutexManager](#mutexmanager)
5. [TaskBase](#taskbase)
6. [Task Classes](#task-classes)
7. [Data Structures](#data-structures)
8. [Examples](#examples)

## Introduction

This document provides complete API documentation for the ESP32 WiFi Utility RTOS implementation.
All APIs are thread-safe and designed for concurrent use across multiple tasks.

### API Conventions

- All functions return `bool` on success/failure unless otherwise specified
- Timeout values are in milliseconds (`TickType_t` internally)
- All queue operations support timeout (0 = no wait, portMAX_DELAY = wait forever)
- Mutexes must be released by the same task that acquired them
- Task names are limited to 16 characters (FreeRTOS limitation)

### Header Files

```cpp
#include "rtos_manager.h"      // Core RTOS initialization and monitoring
#include "queue_manager.h"     // Queue operations and data structures
#include "mutex_manager.h"     // Mutex operations and RAII wrappers
#include "task_base.h"         // Task base class
```

## RTOSManager

Core RTOS infrastructure manager for initialization, monitoring, and statistics.

### Functions

#### `bool initializeRTOS()`

Initialize the RTOS system including queues, mutexes, and monitoring.

**Returns**: `true` on success, `false` on failure

**Example**:

```cpp
void setup() {
    if (!initializeRTOS()) {
        Serial.println("RTOS initialization failed!");
        return;
    }
    Serial.println("RTOS initialized successfully");
}
```

**Side Effects**:

- Creates all system queues
- Creates all system mutexes
- Initializes statistics collection
- Sets RTOS state to RUNNING

---

#### `void shutdownRTOS()`

Gracefully shut down the RTOS system.

**Example**:

```cpp
void systemShutdown() {
    Serial.println("Shutting down RTOS...");
    shutdownRTOS();
    Serial.println("RTOS shutdown complete");
}
```

**Side Effects**:

- Stops all tasks
- Deletes all queues
- Deletes all mutexes
- Sets RTOS state to SHUTTING_DOWN

---

#### `bool isRTOSRunning()`

Check if RTOS is currently running.

**Returns**: `true` if RTOS is running, `false` otherwise

**Example**:

```cpp
if (isRTOSRunning()) {
    // Safe to use RTOS features
    sendCommand(cmd, 100);
} else {
    Serial.println("RTOS not running");
}
```

---

#### `RTOSState getRTOSState()`

Get current RTOS state.

**Returns**: Current `RTOSState` enum value

**States**:

- `UNINITIALIZED`: RTOS not yet initialized
- `INITIALIZING`: Initialization in progress
- `RUNNING`: Normal operation
- `ERROR`: Error state
- `SHUTTING_DOWN`: Shutdown in progress

**Example**:

```cpp
RTOSState state = getRTOSState();
switch (state) {
    case RTOSState::RUNNING:
        Serial.println("RTOS operational");
        break;
    case RTOSState::ERROR:
        Serial.println("RTOS error!");
        break;
    default:
        Serial.println("RTOS not ready");
}
```

---

#### `RTOSStatistics getRTOSStatistics()`

Get comprehensive RTOS statistics.

**Returns**: `RTOSStatistics` structure with current statistics

**Example**:

```cpp
RTOSStatistics stats = getRTOSStatistics();
Serial.printf("Free Heap: %lu bytes\n", stats.freeHeapSize);
Serial.printf("Task Count: %lu\n", stats.taskCount);
Serial.printf("Uptime: %lu ms\n", stats.uptimeMs);
```

---

#### `void updateRTOSStatistics()`

Manually trigger statistics update.

**Example**:

```cpp
// Update stats before logging
updateRTOSStatistics();
RTOSStatistics stats = getRTOSStatistics();
logStatistics(stats);
```

**Note**: Statistics are automatically updated every 5 seconds.

---

#### `void printRTOSStatistics()`

Print formatted RTOS statistics to Serial.

**Example**:

```cpp
void printSystemStatus() {
    Serial.println("\n=== RTOS Statistics ===");
    printRTOSStatistics();
}
```

**Output Format**:

```
=== RTOS Statistics ===
State: RUNNING
Uptime: 45320 ms
Free Heap: 180456 bytes (Min: 175832 bytes)
Tasks: 5, Queues: 4, Mutexes: 4
```

---

#### `bool checkRTOSHealth()`

Perform comprehensive health check.

**Returns**: `true` if system healthy, `false` if issues detected

**Checks**:

- RTOS state is RUNNING
- Free heap > 50 KB
- All tasks are running
- No mutex deadlocks

**Example**:

```cpp
if (!checkRTOSHealth()) {
    Serial.println("RTOS health check FAILED");
    printRTOSStatistics();
    // Take corrective action
}
```

---

### Data Structures

#### `RTOSStatistics`

```cpp
struct RTOSStatistics {
    uint32_t totalHeapSize;        // Total heap size in bytes
    uint32_t freeHeapSize;         // Current free heap
    uint32_t minFreeHeapSize;      // Minimum free heap ever
    uint32_t taskCount;            // Number of active tasks
    uint32_t queueCount;           // Number of queues
    uint32_t mutexCount;           // Number of mutexes
    uint32_t semaphoreCount;       // Number of semaphores
    unsigned long uptimeMs;        // System uptime
    unsigned long lastUpdateMs;    // Last stats update time
};
```

---

## QueueManager

Manages all FreeRTOS queues for inter-task communication.

### Queue Handles

Global queue handles (use via helper functions, not directly):

```cpp
extern QueueHandle_t commandQueue;      // 10 items
extern QueueHandle_t wifiEventQueue;    // 20 items
extern QueueHandle_t statusQueue;       // 15 items
extern QueueHandle_t analysisResultQueue; // 5 items
extern QueueHandle_t webRequestQueue;   // 8 items
```

### Command Queue Functions

#### `bool sendCommand(const CommandRequest& cmd, TickType_t timeout)`

Send command to command queue.

**Parameters**:

- `cmd`: Command request structure
- `timeout`: Max wait time in ms (0 = no wait, portMAX_DELAY = wait forever)

**Returns**: `true` on success, `false` on timeout or error

**Example**:

```cpp
CommandRequest cmd;
cmd.type = CommandRequest::WIFI_SCAN;
cmd.requestId = millis();
cmd.timestamp = millis();

if (sendCommand(cmd, 100)) {
    Serial.println("Command sent");
} else {
    Serial.println("Command queue full");
}
```

---

#### `bool receiveCommand(CommandRequest& cmd, TickType_t timeout)`

Receive command from command queue.

**Parameters**:

- `cmd`: Reference to store received command
- `timeout`: Max wait time in ms

**Returns**: `true` if command received, `false` on timeout

**Example**:

```cpp
void commandTaskLoop() {
    CommandRequest cmd;
    if (receiveCommand(cmd, 100)) {
        processCommand(cmd);
    }
}
```

---

#### `bool peekCommand(CommandRequest& cmd, TickType_t timeout)`

Peek at next command without removing from queue.

**Returns**: `true` if command available, `false` otherwise

**Example**:

```cpp
CommandRequest cmd;
if (peekCommand(cmd, 0)) {
    Serial.printf("Next command: %d\n", cmd.type);
    // Command still in queue
}
```

---

#### `uint32_t getCommandQueueCount()`

Get number of items currently in command queue.

**Returns**: Item count (0 to 10)

**Example**:

```cpp
uint32_t pending = getCommandQueueCount();
Serial.printf("Commands pending: %lu\n", pending);
```

---

#### `bool isCommandQueueFull()`

Check if command queue is full.

**Returns**: `true` if full, `false` if space available

**Example**:

```cpp
if (isCommandQueueFull()) {
    Serial.println("Command queue full, waiting...");
    vTaskDelay(pdMS_TO_TICKS(100));
}
```

---

### WiFi Event Queue Functions

#### `bool sendWiFiEvent(const WiFiEvent& event, TickType_t timeout)`

Send WiFi event to event queue.

**Example**:

```cpp
WiFiEvent event;
event.type = WiFiEvent::CONNECTED;
strncpy(event.ssid, WiFi.SSID().c_str(), 32);
event.rssi = WiFi.RSSI();
event.timestamp = millis();

sendWiFiEvent(event, 100);
```

---

#### `bool receiveWiFiEvent(WiFiEvent& event, TickType_t timeout)`

Receive WiFi event from queue.

**Example**:

```cpp
void ledTaskLoop() {
    WiFiEvent event;
    if (receiveWiFiEvent(event, 100)) {
        updateLEDForEvent(event);
    }
}
```

---

#### `uint32_t getWiFiEventQueueCount()`

Get number of events in queue.

**Example**:

```cpp
uint32_t eventCount = getWiFiEventQueueCount();
if (eventCount > 15) {
    Serial.println("Event queue backing up!");
}
```

---

### Status Queue Functions

#### `bool sendSystemStatus(const SystemStatus& status, TickType_t timeout)`

Send system status update.

**Example**:

```cpp
SystemStatus status;
status.wifiConnected = WiFi.isConnected();
status.freeHeap = esp_get_free_heap_size();
status.uptime = millis();
status.timestamp = millis();

sendSystemStatus(status, 100);
```

---

#### `bool receiveSystemStatus(SystemStatus& status, TickType_t timeout)`

Receive system status update.

**Example**:

```cpp
void webServerLoop() {
    SystemStatus status;
    if (receiveSystemStatus(status, 0)) {
        updateDashboard(status);
    }
}
```

---

### Analysis Result Queue Functions

#### `bool sendAnalysisResult(const AnalysisResult& result, TickType_t timeout)`

Send analysis results.

**Example**:

```cpp
AnalysisResult result;
result.type = AnalysisResult::CHANNEL_SCAN;
result.channelData[1] = calculateChannelScore(1);
// ... populate other channels
result.timestamp = millis();

sendAnalysisResult(result, 100);
```

---

#### `bool receiveAnalysisResult(AnalysisResult& result, TickType_t timeout)`

Receive analysis results.

**Example**:

```cpp
AnalysisResult result;
if (receiveAnalysisResult(result, 1000)) {
    displayAnalysisResults(result);
}
```

---

### Data Structures

#### `CommandRequest`

```cpp
struct CommandRequest {
    enum CommandType {
        WIFI_SCAN,
        WIFI_CONNECT,
        WIFI_DISCONNECT,
        AP_START,
        AP_STOP,
        CHANNEL_ANALYSIS,
        LATENCY_TEST,
        IPERF_TEST,
        STATUS_REQUEST,
        HELP_REQUEST,
        UNKNOWN
    } type;

    String commandString;      // Original command
    String argument;           // First argument (e.g., SSID)
    String secondArgument;     // Second argument (e.g., password)
    uint32_t requestId;        // Unique ID
    uint32_t timestamp;        // Creation time
};
```

---

#### `WiFiEvent`

```cpp
struct WiFiEvent {
    enum EventType {
        SCAN_STARTED,
        SCAN_COMPLETE,
        CONNECT_STARTED,
        CONNECTED,
        DISCONNECTED,
        AP_STARTED,
        AP_STOPPED,
        GOT_IP,
        LOST_IP
    } type;

    char ssid[33];            // Network SSID
    int8_t rssi;              // Signal strength
    uint8_t channel;          // WiFi channel
    uint32_t timestamp;       // Event time
};
```

---

#### `SystemStatus`

```cpp
struct SystemStatus {
    bool wifiConnected;       // WiFi connection status
    bool apMode;              // AP mode active
    int connectedClients;     // Number of AP clients
    uint32_t freeHeap;        // Free heap bytes
    uint32_t uptime;          // Uptime in ms
    char ipAddress[16];       // IP address string
    uint32_t timestamp;       // Status timestamp
};
```

---

## MutexManager

Manages mutex creation, operations, and deadlock detection.

### Mutex Handles

Global mutex handles:

```cpp
extern SemaphoreHandle_t wifiMutex;        // WiFi operations
extern SemaphoreHandle_t webServerMutex;   // Web server state
extern SemaphoreHandle_t scanResultsMutex; // Scan results
extern SemaphoreHandle_t serialMutex;      // Serial port
```

### Functions

#### `bool lockWiFiMutex(TickType_t timeout)`

Acquire WiFi mutex.

**Returns**: `true` if acquired, `false` on timeout

**Example**:

```cpp
if (lockWiFiMutex(100)) {
    // Protected WiFi operation
    WiFi.begin(ssid, password);
    unlockWiFiMutex();
} else {
    Serial.println("WiFi mutex timeout");
}
```

---

#### `void unlockWiFiMutex()`

Release WiFi mutex.

**Note**: Must be called by the same task that acquired the mutex.

---

#### `bool lockWebServerMutex(TickType_t timeout)`

Acquire web server mutex.

**Example**:

```cpp
if (lockWebServerMutex(50)) {
    updateWebServerState();
    unlockWebServerMutex();
}
```

---

#### `void unlockWebServerMutex()`

Release web server mutex.

---

#### `bool lockScanResultsMutex(TickType_t timeout)`

Acquire scan results mutex.

**Example**:

```cpp
if (lockScanResultsMutex(100)) {
    processScanResults();
    unlockScanResultsMutex();
}
```

---

#### `void unlockScanResultsMutex()`

Release scan results mutex.

---

#### `bool lockSerialMutex(TickType_t timeout)`

Acquire serial port mutex.

**Example**:

```cpp
void safeSerialPrint(const char* msg) {
    if (lockSerialMutex(50)) {
        Serial.println(msg);
        unlockSerialMutex();
    }
}
```

---

#### `void unlockSerialMutex()`

Release serial port mutex.

---

### RAII Mutex Guard

#### `class MutexGuard`

RAII wrapper for automatic mutex management.

**Constructor**: Acquires mutex  
**Destructor**: Releases mutex automatically

**Example**:

```cpp
void protectedOperation() {
    MutexGuard guard(wifiMutex, 100);
    if (!guard.acquired()) {
        Serial.println("Failed to acquire mutex");
        return;
    }

    // Mutex automatically released when guard goes out of scope
    WiFi.begin(ssid, password);
}
```

**Advantages**:

- Exception-safe (mutex released even on early return)
- No need to remember to unlock
- Prevents deadlocks from forgotten unlocks

---

### Mutex Statistics

#### `MutexStatistics getMutexStatistics(SemaphoreHandle_t mutex)`

Get detailed statistics for a mutex.

**Example**:

```cpp
MutexStatistics stats = getMutexStatistics(wifiMutex);
Serial.printf("WiFi Mutex - Locks: %lu, Failures: %lu\n",
              stats.lockCount, stats.lockFailures);
Serial.printf("Avg Wait: %lu ms, Max Wait: %lu ms\n",
              stats.avgWaitTimeMs, stats.maxWaitTimeMs);
```

---

#### `void printMutexStatistics(SemaphoreHandle_t mutex)`

Print formatted mutex statistics.

**Example**:

```cpp
Serial.println("\n=== WiFi Mutex Statistics ===");
printMutexStatistics(wifiMutex);
```

---

## TaskBase

Base class for creating FreeRTOS tasks.

### Constructor

```cpp
TaskBase(const char* taskName,
         uint32_t stackSize,
         TaskPriority priority,
         BaseType_t coreId = tskNO_AFFINITY)
```

**Parameters**:

- `taskName`: Task name (max 16 chars)
- `stackSize`: Stack size in bytes
- `priority`: Task priority enum
- `coreId`: CPU core (0, 1, or tskNO_AFFINITY)

**Example**:

```cpp
class MyTask : public TaskBase {
public:
    MyTask() : TaskBase("MyTask", 4096, TaskPriority::PRIORITY_MEDIUM, 1) {
        // Constructor
    }
};
```

---

### Virtual Functions (Must Override)

#### `void setup()`

Called once when task starts. Perform initialization here.

**Example**:

```cpp
void setup() override {
    Serial.println("MyTask starting");
    initializeResources();
}
```

---

#### `void loop()`

Called repeatedly while task is running. Main task logic.

**Example**:

```cpp
void loop() override {
    doWork();
    vTaskDelay(pdMS_TO_TICKS(10));  // Yield CPU
}
```

---

#### `void cleanup()`

Called when task is stopping. Release resources here.

**Example**:

```cpp
void cleanup() override {
    releaseResources();
    Serial.println("MyTask stopped");
}
```

---

### Public Functions

#### `bool start()`

Create and start the task.

**Returns**: `true` on success, `false` on failure

**Example**:

```cpp
MyTask* task = new MyTask();
if (task->start()) {
    Serial.println("Task started");
} else {
    Serial.println("Task start failed");
    delete task;
}
```

---

#### `void stop()`

Request task to stop.

**Example**:

```cpp
void shutdownTask(TaskBase* task) {
    task->stop();
    // Wait for task to actually stop
    while (task->getState() != TaskState::STOPPED) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

---

#### `void suspend()`

Suspend task execution.

**Example**:

```cpp
void pauseTask(TaskBase* task) {
    task->suspend();
    Serial.println("Task suspended");
}
```

---

#### `void resume()`

Resume suspended task.

**Example**:

```cpp
void unpauseTask(TaskBase* task) {
    task->resume();
    Serial.println("Task resumed");
}
```

---

#### `bool isRunning()`

Check if task is running.

**Returns**: `true` if running, `false` otherwise

**Example**:

```cpp
if (myTask->isRunning()) {
    Serial.println("Task is active");
}
```

---

#### `TaskState getState()`

Get current task state.

**Returns**: `TaskState` enum value

**Example**:

```cpp
TaskState state = myTask->getState();
switch (state) {
    case TaskState::RUNNING:
        Serial.println("Running");
        break;
    case TaskState::STOPPED:
        Serial.println("Stopped");
        break;
    default:
        Serial.println("Other state");
}
```

---

#### `void setPriority(TaskPriority newPriority)`

Change task priority at runtime.

**Example**:

```cpp
// Temporarily boost priority for urgent work
myTask->setPriority(TaskPriority::PRIORITY_HIGH);
// ... urgent work ...
myTask->setPriority(TaskPriority::PRIORITY_MEDIUM);
```

---

#### `TaskPriority getPriority()`

Get current task priority.

**Example**:

```cpp
TaskPriority currentPriority = myTask->getPriority();
```

---

#### `TaskStatistics getStatistics()`

Get detailed task statistics.

**Example**:

```cpp
TaskStatistics stats = myTask->getStatistics();
Serial.printf("Loop count: %lu\n", stats.loopCount);
Serial.printf("Stack high water: %lu bytes\n", stats.stackHighWaterMark);
Serial.printf("Avg loop time: %lu ms\n", stats.avgLoopTimeMs);
```

---

#### `void printStatistics()`

Print formatted task statistics.

**Example**:

```cpp
Serial.println("\n=== MyTask Statistics ===");
myTask->printStatistics();
```

---

### Protected Functions

#### `void requestStop()`

Signal to task loop that stop was requested. Call from `loop()` to stop.

**Example**:

```cpp
void loop() override {
    if (shouldStop) {
        requestStop();
        return;
    }
    doWork();
}
```

---

#### `void updateStatistics()`

Update task statistics. Automatically called but can be manually triggered.

---

## Task Classes

### CommandTask

Processes user commands from serial interface.

**Location**: Core 1  
**Priority**: HIGHEST (5)  
**Stack**: 6 KB

**Public Methods**:

```cpp
void queueCommand(const String& cmdStr);  // Add command to queue
bool isBusy();                             // Check if processing
```

**Example**:

```cpp
extern CommandTask* commandTask;

void handleUserInput(String input) {
    commandTask->queueCommand(input);
}
```

---

### WiFiTask

Manages WiFi connections, scanning, and events.

**Location**: Core 0  
**Priority**: HIGH (4)  
**Stack**: 8 KB

**Public Methods**:

```cpp
void startScan();                          // Initiate WiFi scan
void connectToNetwork(const char* ssid, const char* pass);
void disconnect();
bool isConnected();
```

---

### WebServerTask

Handles HTTP requests and web interface.

**Location**: Core 0  
**Priority**: MEDIUM (3)  
**Stack**: 12 KB

**Public Methods**:

```cpp
void startServer();
void stopServer();
bool isServerRunning();
```

---

### LEDTask

Controls LED animations and visual feedback.

**Location**: Core 1  
**Priority**: LOW (2)  
**Stack**: 4 KB

**Public Methods**:

```cpp
void setState(LEDState state, uint8_t priority = 0);
void setPattern(LEDPattern pattern);
void setColor(LEDColor color);
LEDState getCurrentState();
```

**Example**:

```cpp
extern LEDTask* ledTask;

void indicateError() {
    ledTask->setState(LEDState::ERROR, 255);  // High priority
}
```

---

### AnalysisTask

Performs network analysis and diagnostics.

**Location**: Core 1  
**Priority**: VERY_LOW (1)  
**Stack**: 8 KB

**Public Methods**:

```cpp
void startChannelAnalysis();
void startLatencyTest(const char* host);
bool isAnalysisRunning();
```

---

## Examples

### Complete Task Implementation

```cpp
#include "task_base.h"
#include "queue_manager.h"
#include "mutex_manager.h"

class MyCustomTask : public TaskBase {
private:
    int counter;

public:
    MyCustomTask()
        : TaskBase("CustomTask", 4096, TaskPriority::PRIORITY_MEDIUM, 1),
          counter(0) {}

protected:
    void setup() override {
        Serial.println("MyCustomTask: Initializing");
        counter = 0;
    }

    void loop() override {
        // Check for commands
        CommandRequest cmd;
        if (receiveCommand(cmd, 100)) {
            handleCommand(cmd);
        }

        // Do periodic work
        counter++;
        if (counter % 100 == 0) {
            Serial.printf("Counter: %d\n", counter);
        }

        // Yield CPU
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    void cleanup() override {
        Serial.println("MyCustomTask: Cleaning up");
    }

    void handleCommand(const CommandRequest& cmd) {
        // Protected operation
        MutexGuard guard(wifiMutex, 100);
        if (!guard.acquired()) {
            Serial.println("Mutex timeout");
            return;
        }

        // Process command
        Serial.printf("Processing command: %d\n", cmd.type);
    }
};

// Usage
void setup() {
    Serial.begin(115200);

    if (!initializeRTOS()) {
        Serial.println("RTOS init failed");
        return;
    }

    MyCustomTask* task = new MyCustomTask();
    if (!task->start()) {
        Serial.println("Task start failed");
        delete task;
    }
}
```

---

### Queue Communication Pattern

```cpp
// Sender Task
void senderLoop() {
    CommandRequest cmd;
    cmd.type = CommandRequest::WIFI_SCAN;
    cmd.requestId = millis();
    cmd.timestamp = millis();

    if (sendCommand(cmd, 100)) {
        Serial.println("Command sent");
    } else {
        Serial.println("Queue full, retrying...");
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Receiver Task
void receiverLoop() {
    CommandRequest cmd;

    // Wait up to 1 second for command
    if (receiveCommand(cmd, 1000)) {
        processCommand(cmd);

        // Send response via status queue
        SystemStatus status;
        status.timestamp = millis();
        sendSystemStatus(status, 100);
    }
}
```

---

### Mutex Protection Pattern

```cpp
// Method 1: Manual lock/unlock
void manualMutexExample() {
    if (lockWiFiMutex(100)) {
        WiFi.begin(ssid, password);
        unlockWiFiMutex();
    } else {
        Serial.println("Timeout");
    }
}

// Method 2: RAII guard (preferred)
void guardMutexExample() {
    MutexGuard guard(wifiMutex, 100);
    if (!guard.acquired()) {
        Serial.println("Timeout");
        return;
    }

    // Mutex automatically released on return
    WiFi.begin(ssid, password);
}
```

---

### Priority Change Example

```cpp
void urgentOperation(TaskBase* task) {
    // Save original priority
    TaskPriority original = task->getPriority();

    // Boost priority for urgent work
    task->setPriority(TaskPriority::PRIORITY_HIGH);

    // Do urgent work
    performUrgentOperation();

    // Restore original priority
    task->setPriority(original);
}
```

---

### Statistics Monitoring

```cpp
void monitorTaskHealth() {
    TaskStatistics stats = myTask->getStatistics();

    // Check stack usage
    if (stats.stackHighWaterMark < 512) {
        Serial.println("WARNING: Stack usage high!");
    }

    // Check loop performance
    if (stats.maxLoopTimeMs > 100) {
        Serial.println("WARNING: Slow loop detected");
    }

    // Check system health
    RTOSStatistics rtosStats = getRTOSStatistics();
    if (rtosStats.freeHeapSize < 50000) {
        Serial.println("WARNING: Low memory!");
    }
}
```

---

## Best Practices

### Queue Operations

1. **Always use timeouts**: Never use `portMAX_DELAY` in production code
2. **Check return values**: Always check if send/receive succeeded
3. **Handle full queues**: Implement backoff or retry logic
4. **Keep messages small**: Large messages waste memory
5. **Use peek sparingly**: Peek doesn't remove items, can cause starvation

### Mutex Operations

1. **Use RAII guards**: Prefer `MutexGuard` over manual lock/unlock
2. **Keep critical sections short**: Hold mutexes briefly
3. **Consistent ordering**: Always acquire mutexes in same order
4. **Never block in critical section**: No delays or blocking calls
5. **Same task unlock**: Task that locks must unlock

### Task Design

1. **Single responsibility**: Each task has one clear purpose
2. **Yield CPU**: Always use `vTaskDelay()` in loops
3. **Monitor stack**: Check `stackHighWaterMark` regularly
4. **Handle errors**: Tasks should be resilient
5. **Clean shutdown**: Implement proper cleanup

### Performance

1. **Right-size stacks**: Too large wastes RAM, too small risks overflow
2. **Appropriate priorities**: Match priority to importance
3. **Minimize context switches**: Group related work
4. **Use pinning wisely**: Pin network tasks to Core 0
5. **Monitor statistics**: Track performance over time

---

## API Versioning

**Current Version**: 4.1.0

**Version History**:

- 4.1.0: Initial RTOS implementation with full API
- 4.0.x: Pre-RTOS synchronous architecture

**Compatibility**: This API is stable for 4.x releases. Breaking changes will increment major version.

---

## See Also

- [RTOS Architecture Guide](RTOS_ARCHITECTURE.md)
- [Migration Guide](../user-guides/RTOS_MIGRATION_GUIDE.md)
- [Tutorial: Creating a New Task](RTOS_TUTORIAL_NEW_TASK.md)
- [Tutorial: Using Queues](RTOS_TUTORIAL_QUEUES.md)
- [Tutorial: Debugging RTOS](RTOS_TUTORIAL_DEBUGGING.md)
- [FAQ](../user-guides/RTOS_FAQ.md)

---

_Last Updated: 2025-10-18_  
_Version: 4.1.0_  
_Status: Production Ready_
