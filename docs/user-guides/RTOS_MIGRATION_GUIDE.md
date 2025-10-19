# RTOS Migration Guide

**Version**: 4.1.0  
**Date**: 2025-10-18  
**Audience**: Developers migrating from v3.x to v4.x

## Table of Contents

1. [Overview](#overview)
2. [What Changed](#what-changed)
3. [Behavioral Changes](#behavioral-changes)
4. [Async Command Execution](#async-command-execution)
5. [Concurrent Operations](#concurrent-operations)
6. [Timing Considerations](#timing-considerations)
7. [API Changes](#api-changes)
8. [Migration Steps](#migration-steps)
9. [Troubleshooting](#troubleshooting)
10. [Performance Tuning](#performance-tuning)

## Overview

Version 4.x introduces a comprehensive FreeRTOS-based architecture, transforming the ESP32 WiFi
Utility from synchronous, blocking operations to asynchronous, concurrent task-based processing.

### Why Migrate?

**Benefits of RTOS Architecture**:

- ✅ Concurrent WiFi operations and web server
- ✅ Responsive user interface (no blocking)
- ✅ Smooth LED animations during operations
- ✅ Better CPU utilization (dual-core)
- ✅ Improved reliability and error recovery
- ✅ Easier to extend with new features

**Migration Effort**: Low to Medium

- No changes required for basic usage
- Serial commands work identically
- Web interface operates the same
- Advanced users may need to update custom code

### Backward Compatibility

**What Still Works**:

- All serial commands
- Command syntax unchanged
- Web interface URL structure
- Configuration format
- Saved credentials

**What Changed**:

- Command execution is now asynchronous
- Multiple operations can run concurrently
- Timing of events may differ slightly
- Custom integrations may need updates

## What Changed

### Architecture Transformation

**Version 3.x (Synchronous)**:

```
User Input → Command Handler → WiFi Operation (BLOCKS) → Response
              ↓
         Everything waits
```

**Version 4.x (Asynchronous)**:

```
User Input → Command Queue → Command Task → WiFi Task
                                  ↓              ↓
                              Immediate       Executes
                               Return        Async

LED Task, Web Server, Analysis all run concurrently
```

### Key Architectural Changes

| Aspect         | v3.x       | v4.x             |
| -------------- | ---------- | ---------------- |
| Execution      | Blocking   | Asynchronous     |
| Concurrency    | None       | Full             |
| WiFi + Web     | Sequential | Concurrent       |
| LED Updates    | Blocking   | 60 FPS smooth    |
| Core Usage     | Single     | Dual-core        |
| Error Recovery | Reboot     | Task restart     |
| Extensibility  | Difficult  | Easy (new tasks) |

## Behavioral Changes

### 1. Command Execution is Asynchronous

**v3.x Behavior**:

```cpp
// Command blocks until complete
scan               // Blocks for 3-5 seconds
// Returns when scan done
```

**v4.x Behavior**:

```cpp
// Command returns immediately
scan               // Returns in <10ms
// Scan executes in background
// Events notify completion
```

**What This Means**:

- Commands return faster
- Can issue multiple commands
- Must wait for events, not command return
- Status updates are asynchronous

**Example Change**:

**v3.x Code**:

```cpp
void doScanAndConnect() {
    scan();  // Waits for scan
    // Scan complete, results available
    connect("MyWiFi", "password");  // Waits for connection
    // Connection complete
    Serial.println("Connected!");
}
```

**v4.x Code**:

```cpp
void doScanAndConnect() {
    // Issue scan command (returns immediately)
    CommandRequest scanCmd;
    scanCmd.type = CommandRequest::WIFI_SCAN;
    sendCommand(scanCmd, 100);

    // Wait for scan complete event
    WiFiEvent event;
    while (receiveWiFiEvent(event, 5000)) {
        if (event.type == WiFiEvent::SCAN_COMPLETE) {
            break;
        }
    }

    // Now issue connect command
    CommandRequest connectCmd;
    connectCmd.type = CommandRequest::WIFI_CONNECT;
    // ... fill in details
    sendCommand(connectCmd, 100);
}
```

### 2. Multiple Operations Can Run Simultaneously

**v3.x**: Only one operation at a time

```
WiFi Scan → Wait → Connect → Wait → Serve Web Page
```

**v4.x**: Multiple operations concurrently

```
WiFi Scan ─────────────────┐
                           ├─→ All running simultaneously
Web Server ────────────────┤
                           │
LED Animation ─────────────┘
```

**What This Means**:

- Web server responsive during WiFi operations
- LED animations smooth during scanning
- Can scan while connected
- Background analysis doesn't block commands

### 3. Events Replace Polling

**v3.x Pattern**:

```cpp
// Poll for status
while (true) {
    if (WiFi.status() == WL_CONNECTED) {
        break;
    }
    delay(100);
}
```

**v4.x Pattern**:

```cpp
// Wait for event
WiFiEvent event;
if (receiveWiFiEvent(event, 10000)) {
    if (event.type == WiFiEvent::CONNECTED) {
        // Connection successful
    }
}
```

**Benefits**:

- No busy-waiting
- CPU available for other tasks
- Events carry data (SSID, RSSI, etc.)
- More efficient

### 4. LED Behavior Improved

**v3.x**:

- LED updates during delays only
- Slow, stuttering animations
- Blocks during network operations

**v4.x**:

- Dedicated LED task
- Smooth 60 FPS animations
- Never blocks
- Multiple patterns and states

### 5. Error Handling Enhanced

**v3.x**:

- Error = reboot or hang
- No recovery
- Lost all state

**v4.x**:

- Tasks can restart independently
- System continues on task failure
- Graceful degradation
- Health monitoring

## Async Command Execution

### Understanding Async Flow

**Synchronous (v3.x)**:

```
1. Issue command
2. Wait for completion ← YOU ARE HERE until done
3. Get result
4. Continue
```

**Asynchronous (v4.x)**:

```
1. Issue command ← Returns immediately
2. Continue other work
3. Event notification arrives
4. Handle result
```

### Command Queue Depth

Commands are queued (depth: 10):

```cpp
sendCommand(cmd1, 100);  // Queued position 1
sendCommand(cmd2, 100);  // Queued position 2
sendCommand(cmd3, 100);  // Queued position 3
// ... up to 10 commands
sendCommand(cmd11, 100); // FAILS - queue full
```

**Best Practice**: Check queue count before sending burst of commands:

```cpp
if (getCommandQueueCount() < 8) {
    // Safe to send more commands
    sendCommand(cmd, 100);
}
```

### Command Priorities

Commands are FIFO (first-in-first-out), but you can implement priority:

**High Priority Command**:

```cpp
// Clear queue and send urgent command
while (getCommandQueueCount() > 0) {
    CommandRequest dummy;
    receiveCommand(dummy, 0);  // Drain queue
}
sendCommand(urgentCmd, 100);  // Now first in queue
```

**Low Priority Command**:

```cpp
// Wait for queue to be mostly empty
while (getCommandQueueCount() > 3) {
    vTaskDelay(pdMS_TO_TICKS(100));
}
sendCommand(lowPriorityCmd, 100);
```

## Concurrent Operations

### What Can Run Simultaneously

**Concurrent Operations (v4.x)**:

1. WiFi scanning/connecting (Core 0)
2. Web server handling requests (Core 0)
3. Command processing (Core 1)
4. LED animations (Core 1)
5. Network analysis (Core 1)

### Resource Sharing

**Shared Resources** require protection:

**WiFi State** (protected by `wifiMutex`):

```cpp
// WRONG - no protection
WiFi.begin(ssid, password);

// RIGHT - mutex protected
if (lockWiFiMutex(100)) {
    WiFi.begin(ssid, password);
    unlockWiFiMutex();
}
```

**Configuration** (protected by `configMutex`):

```cpp
MutexGuard guard(configMutex, 100);
if (guard.acquired()) {
    readConfiguration();
}
```

### Thread-Safe Operations

**Always Thread-Safe**:

- Queue operations (send/receive)
- Mutex operations (lock/unlock)
- FreeRTOS task functions
- Serial.print (when using serialMutex)

**Never Thread-Safe** (without protection):

- WiFi library calls
- File system access
- Shared variables
- Arduino String operations

### Deadlock Prevention

**Rules to Avoid Deadlock**:

1. **Always use timeouts**:

   ```cpp
   // WRONG
   lockWiFiMutex(portMAX_DELAY);  // Can deadlock

   // RIGHT
   if (!lockWiFiMutex(100)) {
      return false;  // Handle timeout
   }
   ```

2. **Acquire in consistent order**:

   ```cpp
   // WRONG - can deadlock
   Task A: lockA() → lockB()
   Task B: lockB() → lockA()

   // RIGHT - consistent order
   Task A: lockA() → lockB()
   Task B: lockA() → lockB()
   ```

3. **Keep critical sections short**:

   ```cpp
   // WRONG - holds mutex too long
   lockWiFiMutex(100);
   WiFi.begin();        // Blocks for seconds!
   delay(5000);         // Very bad!
   unlockWiFiMutex();

   // RIGHT - minimal holding time
   lockWiFiMutex(100);
   WiFi.begin();
   unlockWiFiMutex();
   // Wait outside critical section
   ```

## Timing Considerations

### Command Latency

**v3.x Latency**: Variable, blocking

- Simple command: 1-10ms
- WiFi scan: 3-5 seconds (blocks)
- Connection: 5-10 seconds (blocks)

**v4.x Latency**: Predictable, non-blocking

- Command accepted: <10ms (all commands)
- Command execution: asynchronous
- Event notification: <20ms after event

### Event Propagation Time

Typical times from event to notification:

| Event          | Propagation Time |
| -------------- | ---------------- |
| WiFi connected | 5-10ms           |
| Scan complete  | 10-20ms          |
| Disconnect     | 5-10ms           |
| Web request    | 20-50ms          |

### Loop Delays

Tasks use 10ms delays:

```cpp
void loop() override {
    doWork();
    vTaskDelay(pdMS_TO_TICKS(10));  // 100 Hz update rate
}
```

**What This Means**:

- Maximum event processing delay: ~10ms
- Multiple events processed per second
- No task starvation
- Efficient CPU usage

### Watchdog Timers

**v3.x**: Single watchdog, 8 second timeout

**v4.x**: Per-core watchdogs

- Core 0 watchdog: 5 seconds
- Core 1 watchdog: 5 seconds
- Task watchdog: configurable

**Implications**:

- Don't block for >5 seconds in any task
- Use `vTaskDelay()` in long loops
- Watchdog fed automatically by FreeRTOS

## API Changes

### Removed Functions

Functions removed in v4.x:

| v3.x Function         | v4.x Alternative            | Reason       |
| --------------------- | --------------------------- | ------------ |
| `blockingConnect()`   | `sendCommand(WIFI_CONNECT)` | Asynchronous |
| `blockingScan()`      | `sendCommand(WIFI_SCAN)`    | Asynchronous |
| `waitForConnection()` | Receive `WiFiEvent`         | Event-driven |
| `pollStatus()`        | Receive `SystemStatus`      | Queue-based  |

### Modified Functions

Functions with changed signatures:

**v3.x**:

```cpp
bool connectWiFi(String ssid, String password);  // Blocks
```

**v4.x**:

```cpp
// Non-blocking, queued
CommandRequest cmd;
cmd.type = CommandRequest::WIFI_CONNECT;
cmd.argument = ssid;
cmd.secondArgument = password;
sendCommand(cmd, 100);
```

### New Functions

New v4.x APIs:

**RTOS Core**:

- `initializeRTOS()`
- `shutdownRTOS()`
- `checkRTOSHealth()`
- `getRTOSStatistics()`

**Queue Management**:

- `sendCommand()`, `receiveCommand()`
- `sendWiFiEvent()`, `receiveWiFiEvent()`
- `sendSystemStatus()`, `receiveSystemStatus()`

**Mutex Management**:

- `lockWiFiMutex()`, `unlockWiFiMutex()`
- `lockWebServerMutex()`, `unlockWebServerMutex()`
- `MutexGuard` class

**Task Management**:

- `TaskBase::start()`, `stop()`
- `TaskBase::suspend()`, `resume()`
- `TaskBase::getStatistics()`

## Migration Steps

### Step 1: Update Build Configuration

**platformio.ini changes**:

```ini
[env:esp32dev_rtos]
platform = espressif32
board = esp32dev
framework = arduino
build_flags =
    -DUSE_RTOS           ; Enable RTOS mode
    -DCORE_DEBUG_LEVEL=3
lib_deps =
    # ... your existing dependencies
```

### Step 2: Replace Blocking Calls

**Before (v3.x)**:

```cpp
void loop() {
    String cmd = Serial.readStringUntil('\n');
    if (cmd == "scan") {
        doBlockingScan();  // Blocks
    }
}
```

**After (v4.x)**:

```cpp
void loop() {
    String cmd = Serial.readStringUntil('\n');
    if (cmd == "scan") {
        CommandRequest req;
        req.type = CommandRequest::WIFI_SCAN;
        sendCommand(req, 100);  // Non-blocking
    }
}
```

### Step 3: Replace Status Polling

**Before (v3.x)**:

```cpp
while (WiFi.status() != WL_CONNECTED) {
    delay(100);
}
```

**After (v4.x)**:

```cpp
WiFiEvent event;
while (receiveWiFiEvent(event, 10000)) {
    if (event.type == WiFiEvent::CONNECTED) {
        break;
    }
}
```

### Step 4: Add Mutex Protection

**Before (v3.x)**:

```cpp
WiFi.begin(ssid, password);
```

**After (v4.x)**:

```cpp
MutexGuard guard(wifiMutex, 100);
if (guard.acquired()) {
    WiFi.begin(ssid, password);
}
```

### Step 5: Update Custom Tasks

If you created custom tasks, derive from `TaskBase`:

**Before (v3.x)**:

```cpp
void myTaskFunction() {
    while (true) {
        doWork();
        delay(100);
    }
}

void setup() {
    xTaskCreate(myTaskFunction, "MyTask", 4096, NULL, 1, NULL);
}
```

**After (v4.x)**:

```cpp
class MyTask : public TaskBase {
public:
    MyTask() : TaskBase("MyTask", 4096, TaskPriority::PRIORITY_MEDIUM) {}

protected:
    void setup() override { /* init */ }
    void loop() override {
        doWork();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    void cleanup() override { /* cleanup */ }
};

void setup() {
    initializeRTOS();
    MyTask* task = new MyTask();
    task->start();
}
```

### Step 6: Test Thoroughly

**Test Checklist**:

- [ ] All serial commands work
- [ ] WiFi scanning works
- [ ] WiFi connection works
- [ ] AP mode works
- [ ] Web interface accessible
- [ ] LED animations smooth
- [ ] No crashes/reboots
- [ ] Memory usage acceptable
- [ ] Performance meets requirements

## Troubleshooting

### Commands Not Executing

**Symptom**: Commands queued but not processing

**Causes**:

1. RTOS not initialized
2. Command task not started
3. Queue full

**Solutions**:

```cpp
// Check RTOS state
if (!isRTOSRunning()) {
    initializeRTOS();
}

// Check queue
uint32_t count = getCommandQueueCount();
Serial.printf("Queue depth: %lu\n", count);
if (count >= 10) {
    Serial.println("Queue full!");
}

// Check task
if (commandTask && !commandTask->isRunning()) {
    commandTask->start();
}
```

### Task Stack Overflow

**Symptom**: Crash, reboot, or "Stack canary watchpoint" error

**Cause**: Task stack too small

**Solutions**:

```cpp
// Check stack usage
TaskStatistics stats = myTask->getStatistics();
if (stats.stackHighWaterMark < 512) {
    Serial.println("Stack almost full!");
    // Increase stack size in task constructor
}

// Increase stack size
MyTask() : TaskBase("MyTask", 8192, ...) {}  // Was 4096
```

### Watchdog Timeout

**Symptom**: "Task watchdog got triggered" message and reboot

**Cause**: Task blocked for >5 seconds

**Solutions**:

```cpp
// WRONG - blocks too long
for (int i = 0; i < 1000000; i++) {
    doWork();
}

// RIGHT - periodic yields
for (int i = 0; i < 1000000; i++) {
    doWork();
    if (i % 1000 == 0) {
        vTaskDelay(pdMS_TO_TICKS(1));  // Yield periodically
    }
}
```

### Memory Leaks

**Symptom**: Free heap decreasing over time

**Diagnosis**:

```cpp
void checkMemory() {
    RTOSStatistics stats = getRTOSStatistics();
    Serial.printf("Free heap: %lu (min: %lu)\n",
                  stats.freeHeapSize,
                  stats.minFreeHeapSize);

    if (stats.freeHeapSize < 50000) {
        Serial.println("LOW MEMORY!");
    }
}
```

**Solutions**:

1. Check for unclosed resources (files, connections)
2. Look for growing queues
3. Verify `delete` matches `new`
4. Use RAII patterns (automatic cleanup)

### Mutex Deadlock

**Symptom**: System hangs, tasks stop responding

**Diagnosis**:

```cpp
MutexStatistics stats = getMutexStatistics(wifiMutex);
if (stats.currentlyLocked &&
    (millis() - stats.lockTimeMs) > 5000) {
    Serial.println("Possible deadlock!");
    Serial.printf("Locked by task: %p\n", stats.ownerTask);
}
```

**Solutions**:

1. Always use timeouts (not portMAX_DELAY)
2. Check mutex acquisition order
3. Keep critical sections minimal
4. Use `MutexGuard` for automatic release

### Slow Performance

**Symptom**: Operations slower than expected

**Diagnosis**:

```cpp
// Check task performance
TaskStatistics stats = myTask->getStatistics();
Serial.printf("Avg loop time: %lu ms (max: %lu ms)\n",
              stats.avgLoopTimeMs, stats.maxLoopTimeMs);

// Check CPU load
// High CPU load on one core?
// Check task pinning and priorities
```

**Solutions**:

1. Adjust task priorities
2. Balance work across cores
3. Reduce loop delay times
4. Optimize critical paths
5. Use profiling to find bottlenecks

## Performance Tuning

### Optimizing Task Priorities

**Default Priorities**:

```
CommandTask:    HIGHEST (5)  - User responsiveness
WiFiTask:       PRIORITY_HIGH (4)
WebServerTask:  PRIORITY_MEDIUM (3)
LEDTask:        PRIORITY_LOW (2)
AnalysisTask:   PRIORITY_VERY_LOW (1)
```

**When to Adjust**:

- Web server slow → Increase WebServerTask priority
- LED stutters → Increase LEDTask priority
- Commands delayed → Increase CommandTask priority (already highest)

**How to Adjust**:

```cpp
myTask->setPriority(TaskPriority::PRIORITY_HIGH);
```

### Optimizing Stack Sizes

**Current Allocations**:

```
WiFiTask:       8 KB
WebServerTask:  12 KB
CommandTask:    6 KB
LEDTask:        4 KB
AnalysisTask:   8 KB
```

**Optimization Process**:

```cpp
// 1. Measure actual usage
TaskStatistics stats = myTask->getStatistics();
uint32_t used = myTask->stackSize - stats.stackHighWaterMark;
Serial.printf("Stack used: %lu of %lu bytes\n", used, myTask->stackSize);

// 2. Calculate optimal size
uint32_t optimal = used * 1.25;  // 25% safety margin

// 3. Update task constructor
MyTask() : TaskBase("MyTask", optimal, ...) {}
```

### Optimizing Queue Sizes

**Current Sizes**:

```
commandQueue:       10 items
wifiEventQueue:     20 items
statusQueue:        15 items
```

**When to Adjust**:

- Queue full frequently → Increase size
- Queue rarely >30% full → Decrease size

**Monitoring**:

```cpp
uint32_t count = getCommandQueueCount();
uint32_t capacity = 10;
float utilization = (float)count / capacity * 100.0;
Serial.printf("Queue utilization: %.1f%%\n", utilization);
```

### Core Assignment Optimization

**Default Assignment**:

```
Core 0: WiFiTask, WebServerTask
Core 1: CommandTask, LEDTask, AnalysisTask
```

**When to Adjust**:

- One core consistently >80% → Move tasks
- Network latency high → Pin WiFi to Core 0 (already done)
- UI sluggish → Pin UI tasks to Core 1 (already done)

**Monitoring Load**:

```cpp
// Use ESP32's built-in monitoring
vTaskGetRunTimeStats(buffer);
Serial.println(buffer);
```

## Conclusion

The RTOS architecture in v4.x provides significant improvements in responsiveness, concurrency,
and reliability. While the migration requires understanding asynchronous patterns and proper
resource protection, the benefits are substantial.

**Key Takeaways**:

- Commands execute asynchronously (non-blocking)
- Multiple operations run concurrently
- Events replace polling
- Mutex protection required for shared resources
- Proper error handling prevents system failures

**Next Steps**:

1. Review the [RTOS Architecture Guide](../technical/RTOS_ARCHITECTURE.md)
2. Study the [API Reference](../technical/RTOS_API_REFERENCE.md)
3. Try the [Creating a Task Tutorial](../technical/RTOS_TUTORIAL_NEW_TASK.md)
4. Read the [FAQ](RTOS_FAQ.md) for common questions

**Support**:

- Check [GitHub Issues](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/issues)
- Review [Test Results](../../test/RTOS_TEST_RESULTS.md)
- Consult [Debugging Tutorial](../technical/RTOS_TUTORIAL_DEBUGGING.md)

---

_Last Updated: 2025-10-18_  
_Version: 4.1.0_  
_Status: Production Ready_
