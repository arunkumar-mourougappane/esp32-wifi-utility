# RTOS Architecture Guide

**Version**: 4.1.0  
**Date**: 2025-10-18  
**Status**: Production

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Task Structure](#task-structure)
4. [Queue Architecture](#queue-architecture)
5. [Synchronization Primitives](#synchronization-primitives)
6. [CPU Core Assignments](#cpu-core-assignments)
7. [Memory Management](#memory-management)
8. [Design Decisions](#design-decisions)
9. [Performance Characteristics](#performance-characteristics)

## Overview

The ESP32 WiFi Utility implements a comprehensive FreeRTOS-based architecture that enables concurrent,
responsive, and maintainable operation. The system leverages ESP32's dual-core architecture to
efficiently manage WiFi operations, command processing, web server, and LED control simultaneously.

### Key Features

- **Dual-Core Task Distribution**: Core 0 for system-critical tasks, Core 1 for application tasks
- **Queue-Based Communication**: Type-safe, thread-safe inter-task messaging
- **Priority-Based Scheduling**: 5-level priority system ensures responsive operation
- **Resource Protection**: Mutexes protect shared resources and configuration
- **Event-Driven Architecture**: Asynchronous command execution and event propagation
- **Memory Efficient**: Optimized stack sizes and minimal heap fragmentation

### Architecture Principles

1. **Separation of Concerns**: Each task has a single, well-defined responsibility
2. **Loose Coupling**: Tasks communicate only through queues, not direct function calls
3. **Fail-Safe Design**: Tasks can fail and restart without affecting system stability
4. **Observable Behavior**: All state changes propagate through events
5. **Testability**: Components designed for independent testing

## System Architecture

### High-Level Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                         ESP32 WiFi Utility                          │
│                         RTOS Architecture                           │
└─────────────────────────────────────────────────────────────────────┘

┌──────────────────────── Core 0 (Protocol CPU) ────────────────────────┐
│                                                                       │
│  ┌──────────────────┐         ┌──────────────────┐                    │
│  │   WiFi Manager   │         │   Web Server     │                    │
│  │  Priority: HIGH  │         │ Priority: MEDIUM │                    │
│  │   Task (8KB)     │         │   Task (12KB)    │                    │
│  └─────────┬────────┘         └─────────┬────────┘                    │
│            │                            │                             │
│            │ WiFiEvent Queue            │ HTTP Requests               │
│            └────────────┬───────────────┘                             │
└─────────────────────────┼─────────────────────────────────────────────┘
                          │
                          │ System Events
                          │
┌──────────────────────── Core 1 (Application CPU) ──────────────────────┐
│                                                                        │
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐      │
│  │  Command Task    │  │   LED Task       │  │  Analysis Task   │      │
│  │Priority: HIGHEST │  │ Priority: LOW    │  │Priority: V.LOW   │      │
│  │   Task (6KB)     │  │   Task (4KB)     │  │   Task (8KB)     │      │
│  └─────────┬────────┘  └─────────┬────────┘  └──────────────────┘      │
│            │                     │                                     │
│            │ Command Queue       │ LED State Queue                     │
│            └─────────┬───────────┘                                     │
└──────────────────────┼─────────────────────────────────────────────────┘
                       │
                       │ Status Queue
                       ▼
              ┌─────────────────┐
              │ Config Mutex    │
              │ System Mutex    │
              └─────────────────┘
```

### Communication Flow

```
User Input (Serial/Web)
         │
         ▼
  ┌──────────────┐
  │ Command Task │ ──────────► Command Queue (10 items)
  └──────────────┘                    │
         │                            ▼
         │                   ┌─────────────────┐
         │                   │  WiFi Manager   │
         │                   │    Processes    │
         │                   │    Commands     │
         │                   └─────────────────┘
         │                            │
         │                            ▼
         │                   WiFi Event Queue (15 items)
         │                            │
         │                            ▼
         │                   ┌─────────────────┐
         │                   │   LED Task      │ ◄── LED State Queue (5 items)
         │                   │ Updates Visual  │
         │                   └─────────────────┘
         │
         ▼
  Status Queue (10 items)
         │
         ▼
  ┌──────────────┐
  │  Web Server  │ ──────────► HTTP Response
  └──────────────┘
```

## Task Structure

### Task Base Class

All tasks inherit from `TaskBase`, providing common functionality:

```cpp
class TaskBase {
protected:
    TaskHandle_t taskHandle;
    uint32_t stackSize;
    TaskPriority priority;
    BaseType_t coreId;
    TaskState state;

    virtual void setup() = 0;    // One-time initialization
    virtual void loop() = 0;     // Continuous execution
    virtual void cleanup() = 0;  // Shutdown cleanup

public:
    bool start();                // Create and start task
    void stop();                 // Request task stop
    void suspend();              // Suspend task execution
    void resume();               // Resume suspended task
    bool isRunning();           // Check task state
};
```

### Task Priority Levels

```cpp
enum class TaskPriority {
    PRIORITY_VERY_LOW  = 1,  // Analysis, background tasks
    PRIORITY_LOW       = 2,  // LED control, non-critical UI
    PRIORITY_MEDIUM    = 3,  // Web server, general processing
    PRIORITY_HIGH      = 4,  // WiFi manager, network operations
    PRIORITY_HIGHEST   = 5   // Command interface, user input
};
```

**Priority Assignment Rationale**:

- **HIGHEST**: Command interface - immediate user responsiveness
- **HIGH**: WiFi manager - network operations are time-sensitive
- **MEDIUM**: Web server - balanced responsiveness
- **LOW**: LED control - visual feedback can tolerate slight delays
- **VERY_LOW**: Analysis - background, no user-facing impact

### Task Inventory

| Task Name     | Core | Priority     | Stack | Purpose                               |
| ------------- | ---- | ------------ | ----- | ------------------------------------- |
| CommandTask   | 1    | HIGHEST (5)  | 6KB   | Process user commands from serial/web |
| WiFiTask      | 0    | HIGH (4)     | 8KB   | Manage WiFi connections and scanning  |
| WebServerTask | 0    | MEDIUM (3)   | 12KB  | Handle HTTP requests and responses    |
| LEDTask       | 1    | LOW (2)      | 4KB   | Control LED animations and states     |
| AnalysisTask  | 1    | VERY_LOW (1) | 8KB   | Perform network analysis (optional)   |

### Task Lifecycle

```
  [CREATED]
      │
      │ start()
      ▼
  [STARTING] ──► setup() ──► [RUNNING]
      │                          │
      │                          │ loop() continuously
      │                          │
      │ Error                    │ stop() or error
      │                          ▼
      └──────────────────► [STOPPING] ──► cleanup() ──► [STOPPED]
                                                              │
                                                              │ restart()
                                                              └──► [STARTING]
```

## Queue Architecture

### Queue Types and Purposes

#### 1. Command Queue

```cpp
QueueHandle_t commandQueue;  // 10 items, sizeof(CommandRequest)

struct CommandRequest {
    enum CommandType {
        WIFI_SCAN,
        WIFI_CONNECT,
        WIFI_DISCONNECT,
        WIFI_STATUS,
        STATUS_REQUEST,
        CONFIG_READ,
        CONFIG_WRITE
    };

    CommandType type;
    uint32_t requestId;
    char data[128];
    uint32_t timestamp;
};
```

**Flow**: CommandTask → WiFiTask/AnalysisTask
**Purpose**: User commands requiring WiFi or system actions

#### 2. WiFi Event Queue

```cpp
QueueHandle_t wifiEventQueue;  // 15 items, sizeof(WiFiEvent)

struct WiFiEvent {
    enum EventType {
        SCAN_STARTED,
        SCAN_COMPLETE,
        CONNECT_STARTED,
        CONNECT_SUCCESS,
        CONNECT_FAILED,
        DISCONNECT,
        IP_CHANGED
    };

    EventType type;
    char ssid[33];
    int8_t rssi;
    uint32_t timestamp;
};
```

**Flow**: WiFiTask → LEDTask/WebServerTask/StatusQueue
**Purpose**: WiFi state changes and events propagation

#### 3. Status Queue

```cpp
QueueHandle_t statusQueue;  // 10 items, sizeof(SystemStatus)

struct SystemStatus {
    bool wifiConnected;
    bool apMode;
    int connectedClients;
    uint32_t freeHeap;
    uint32_t uptime;
    uint32_t timestamp;
};
```

**Flow**: All Tasks → WebServerTask
**Purpose**: System state updates for status reporting

#### 4. LED State Queue

```cpp
QueueHandle_t ledStateQueue;  // 5 items, sizeof(LEDStateRequest)

struct LEDStateRequest {
    LEDState state;
    LEDPattern pattern;
    uint8_t priority;
    LEDColor color;
};
```

**Flow**: WiFiTask/CommandTask → LEDTask
**Purpose**: Visual feedback control

### Queue Sizing Rationale

| Queue     | Size | Reasoning                                                      |
| --------- | ---- | -------------------------------------------------------------- |
| Command   | 10   | Typical user command rate ~1/sec, 10-second buffer adequate    |
| WiFiEvent | 15   | Multiple rapid events during scan/connect, needs larger buffer |
| Status    | 10   | Periodic updates every 5s, allows 50s of backlog               |
| LEDState  | 5    | Visual changes infrequent, small buffer sufficient             |

### Queue Timeout Strategies

- **Blocking Send** (infinite timeout): Never used to avoid deadlocks
- **Short Timeout** (10-100ms): Standard for most operations
- **Poll** (0ms): Used in loops for non-blocking checks
- **Long Timeout** (1000ms): Used for critical operations

## Synchronization Primitives

### Mutex Inventory

#### 1. Config Mutex

```cpp
SemaphoreHandle_t configMutex;  // Protects configuration data
```

**Protected Resources**:

- WiFi credentials
- AP settings
- System configuration
- Persistent storage access

**Usage Pattern**:

```cpp
if (lockConfigMutex(100)) {
    // Access configuration
    unlockConfigMutex();
} else {
    // Handle timeout
}
```

#### 2. System State Mutex

```cpp
SemaphoreHandle_t systemMutex;  // Protects runtime state
```

**Protected Resources**:

- Connection status
- Client count
- Runtime flags
- Shared buffers

### Deadlock Prevention

**Rules**:

1. Always use timeouts (never wait indefinitely)
2. Acquire mutexes in consistent order
3. Release mutexes in reverse order
4. Keep critical sections minimal
5. Never call blocking functions while holding mutex

**Mutex Acquisition Order**:

```
configMutex → systemMutex
(Never reverse this order)
```

## CPU Core Assignments

### ESP32 Dual-Core Architecture

- **Core 0 (Protocol CPU)**: System services, WiFi stack, Bluetooth
- **Core 1 (Application CPU)**: User application code

### Task Pinning Strategy

#### Core 0 - System & Network Tasks

```cpp
WiFiTask:       Core 0, Priority HIGH
WebServerTask:  Core 0, Priority MEDIUM
```

**Rationale**: WiFi and web server benefit from co-location with WiFi stack on Core 0,
reducing context switches and improving network performance.

#### Core 1 - Application Tasks

```cpp
CommandTask:    Core 1, Priority HIGHEST
LEDTask:        Core 1, Priority LOW
AnalysisTask:   Core 1, Priority VERY_LOW
```

**Rationale**: User-facing tasks on Core 1 keep application responsive while
Core 0 handles protocol processing.

### Core Load Distribution

```
Core 0: ~30-40% average load
├─ WiFi Stack:  15-25%
├─ WiFi Task:   10-15%
└─ Web Server:  5-10%

Core 1: ~20-30% average load
├─ Command Task: 5-10%
├─ LED Task:     2-5%
└─ Analysis:     10-15% (when active)

Idle: 30-50% (dynamic, available for spikes)
```

## Memory Management

### Memory Layout

```
ESP32 Memory Map:
┌────────────────────────────────────┐
│  Internal RAM: 520 KB total        │
├────────────────────────────────────┤
│  DRAM0: 320 KB (used by FreeRTOS)  │
│  ├─ Task Stacks                    │
│  │  ├─ WiFiTask:      8 KB         │
│  │  ├─ WebServerTask: 12 KB        │
│  │  ├─ CommandTask:   6 KB         │
│  │  ├─ LEDTask:       4 KB         │
│  │  └─ AnalysisTask:  8 KB         │
│  │     Total Stacks:  38 KB        │
│  ├─ Queue Storage:     ~2 KB       │
│  ├─ Heap:              ~220 KB     │
│  └─ System Reserved:   ~60 KB      │
└────────────────────────────────────┘
```

### Stack Size Calculations

**Formula**: `Required = Peak Usage × Safety Factor`

| Task      | Peak   | Safety | Allocated |
| --------- | ------ | ------ | --------- |
| WiFi      | 5.2 KB | 1.5×   | 8 KB      |
| WebServer | 9.5 KB | 1.25×  | 12 KB     |
| Command   | 4.8 KB | 1.25×  | 6 KB      |
| LED       | 2.9 KB | 1.4×   | 4 KB      |
| Analysis  | 6.1 KB | 1.3×   | 8 KB      |

**Safety Factors**:

- High variability tasks: 1.5×
- Library-heavy tasks: 1.4×
- Well-understood tasks: 1.25×

### Heap Management

**Current Usage** (typical):

- Task creation: ~500 bytes per task
- Queue creation: ~200 bytes per queue
- Runtime allocations: ~10-20 KB
- WiFi stack: ~40-50 KB
- Free heap (steady state): ~150-180 KB

**Heap Fragmentation Prevention**:

1. Pre-allocate fixed-size buffers where possible
2. Use stack allocation for temporary data
3. Avoid frequent malloc/free cycles
4. Use memory pools for repetitive allocations

### Memory Monitoring

```cpp
// Periodic monitoring in each task
UBaseType_t stackHighWater = uxTaskGetStackHighWaterMark(NULL);
if (stackHighWater < 512) {
    // Stack usage concerning, log warning
}

// System-wide monitoring
size_t freeHeap = esp_get_free_heap_size();
if (freeHeap < 50000) {
    // Low memory, trigger cleanup
}
```

## Design Decisions

### Why Queue-Based Communication?

**Alternatives Considered**:

1. Direct function calls
2. Global shared variables
3. Event groups
4. Callbacks

**Queue Benefits**:

- ✅ Thread-safe by design
- ✅ Decouples tasks completely
- ✅ Natural backpressure mechanism
- ✅ Easy to test and debug
- ✅ Supports priority and timeout

### Why Dual-Core Task Distribution?

**Single-Core Alternative**: All tasks on Core 1

**Dual-Core Benefits**:

- ✅ WiFi task near WiFi stack (Core 0)
- ✅ Reduced context switch overhead
- ✅ Better cache locality
- ✅ Isolation of network from application
- ✅ ~20% throughput improvement

### Why Priority-Based Scheduling?

**Alternative**: All tasks equal priority (time-slicing)

**Priority Benefits**:

- ✅ Guaranteed user responsiveness
- ✅ Network operations preempt background tasks
- ✅ Critical tasks never starved
- ✅ Predictable latency
- ✅ Better matches task importance

### Why 10ms Loop Delays?

Tasks use `vTaskDelay(pdMS_TO_TICKS(10))` in loops.

**Rationale**:

- Prevents task starvation
- Allows scheduler to run
- 100 Hz update rate sufficient for most operations
- Balances responsiveness vs efficiency
- Lower delays waste CPU, higher delays reduce responsiveness

## Performance Characteristics

### Latency Measurements

| Operation         | Typical    | Max    | Target  |
| ----------------- | ---------- | ------ | ------- |
| Command to Action | 2-5 ms     | 10 ms  | <10 ms  |
| WiFi Event to LED | 5-8 ms     | 15 ms  | <20 ms  |
| Queue Send        | 200-500 μs | 1 ms   | <1 ms   |
| Mutex Lock        | 20-50 μs   | 100 μs | <100 μs |
| Task Switch       | 100-300 μs | 1 ms   | <1 ms   |
| HTTP Request      | 10-30 ms   | 100 ms | <100 ms |

### Throughput Measurements

| Metric             | Value   | Notes                                |
| ------------------ | ------- | ------------------------------------ |
| Commands/sec       | 150-300 | Limited by WiFi operations, not RTOS |
| WiFi Events/sec    | 50-100  | During active scanning               |
| Status Updates/sec | 10-20   | Configurable rate                    |
| LED Updates/sec    | 60      | Fixed 60 FPS animation rate          |

### CPU Utilization

**Idle System** (connected, no activity):

- Core 0: 15-20% (WiFi keep-alive)
- Core 1: 5-10% (LED animation, monitoring)

**Active Scanning**:

- Core 0: 40-60% (WiFi scanning, web server)
- Core 1: 10-15% (command processing, LED updates)

**Heavy Load** (scanning + web requests):

- Core 0: 60-80% (near maximum)
- Core 1: 20-30% (moderate)

### Memory Usage

**Startup** (after initialization):

- Free heap: ~200 KB
- Largest free block: ~100 KB

**Steady State**:

- Free heap: ~170-180 KB
- Fragmentation: <5%

**Under Load**:

- Free heap: ~150-160 KB
- Temporary allocations: ~20-30 KB

## Scalability and Future Expansion

### Adding New Tasks

**Process**:

1. Derive from `TaskBase`
2. Implement `setup()`, `loop()`, `cleanup()`
3. Choose appropriate priority and core
4. Allocate adequate stack (measure + safety factor)
5. Create queue for communication (if needed)
6. Register with RTOSManager

**Capacity**:

- Current: 5 tasks
- Maximum practical: ~15-20 tasks
- Limited by: Memory, scheduling overhead, complexity

### Queue System Expansion

**Adding New Queue**:

1. Define message structure
2. Create queue in `QueueManager`
3. Document flow and purpose
4. Add send/receive helpers
5. Update architecture documentation

### Performance Tuning

**When to Tune**:

- Task latency exceeds requirements
- CPU utilization consistently >80%
- Free heap drops below 100 KB
- Stack high-water mark < 20% of allocated

**Tuning Strategies**:

1. Adjust task priorities
2. Modify loop delays
3. Optimize critical sections
4. Reduce stack sizes (carefully)
5. Move tasks between cores

## Conclusion

The ESP32 WiFi Utility RTOS architecture provides a robust, scalable foundation for concurrent
operations. The dual-core task distribution, queue-based communication, and priority scheduling
ensure responsive, reliable operation under varying loads.

**Key Strengths**:

- Clean separation of concerns
- Predictable performance
- Easy to extend and maintain
- Well-tested and documented
- Efficient resource usage

**For More Information**:

- [RTOS API Reference](RTOS_API_REFERENCE.md)
- [Migration Guide](../user-guides/RTOS_MIGRATION_GUIDE.md)
- [Tutorial: Creating a New Task](RTOS_TUTORIAL_NEW_TASK.md)
- [Tutorial: Using Queues](RTOS_TUTORIAL_QUEUES.md)
- [Test Results](../../test/RTOS_TEST_RESULTS.md)

---

_Last Updated: 2025-10-18_  
_Version: 4.1.0_  
_Status: Production Ready_
