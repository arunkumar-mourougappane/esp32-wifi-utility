# Tutorial: Debugging RTOS Issues

**Difficulty**: Intermediate  
**Time**: 20-30 minutes  
**Prerequisites**: Basic RTOS knowledge

## Common RTOS Issues

### 1. Stack Overflow

**Symptoms**:
- Random crashes
- "Stack canary watchpoint triggered"
- Task stops responding

**Debug**:
```cpp
void checkStackUsage() {
    TaskStatistics stats = myTask->getStatistics();
    float usedPercent = 100.0 * (stats.stackSize - stats.stackHighWaterMark) / stats.stackSize;
    
    Serial.printf("Stack: %lu/%lu bytes (%.1f%% used)\n",
                  stats.stackSize - stats.stackHighWaterMark,
                  stats.stackSize,
                  usedPercent);
    
    if (usedPercent > 90.0) {
        Serial.println("⚠️  WARNING: Stack almost full!");
    }
}
```

**Fix**: Increase stack size in task constructor

### 2. Watchdog Timeout

**Symptoms**:
- "Task watchdog got triggered" message
- System reboots
- Task name shown in error

**Debug**:
```cpp
void loop() override {
    unsigned long start = millis();
    
    doWork();
    
    unsigned long duration = millis() - start;
    if (duration > 1000) {
        Serial.printf("WARNING: Loop took %lu ms\n", duration);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));  // Must yield!
}
```

**Fix**: Add vTaskDelay() calls, break up long operations

### 3. Memory Leak

**Symptoms**:
- Free heap decreasing over time
- Eventually crashes with out-of-memory

**Debug**:
```cpp
void monitorMemory() {
    static uint32_t minHeap = 0xFFFFFFFF;
    uint32_t freeHeap = esp_get_free_heap_size();
    
    if (freeHeap < minHeap) {
        minHeap = freeHeap;
        Serial.printf("New minimum heap: %lu bytes\n", minHeap);
    }
    
    if (freeHeap < 50000) {
        Serial.println("⚠️  LOW MEMORY!");
    }
}
```

**Fix**: Check for missing delete, unclosed files, growing queues

### 4. Deadlock

**Symptoms**:
- System hangs
- No serial output
- Specific tasks stop

**Debug**:
```cpp
void detectDeadlock() {
    MutexStatistics stats = getMutexStatistics(wifiMutex);
    
    if (stats.currentlyLocked) {
        unsigned long lockDuration = millis() - stats.lockTimeMs;
        if (lockDuration > 5000) {
            Serial.printf("⚠️  Possible deadlock! Mutex locked for %lu ms\n", 
                         lockDuration);
            Serial.printf("Owner task: %p\n", stats.ownerTask);
        }
    }
}
```

**Fix**: Always use timeouts, acquire mutexes in same order

### 5. Priority Inversion

**Symptoms**:
- High priority task blocked
- Low priority task running instead
- Unpredictable timing

**Debug**:
```cpp
void checkPriorityInversion() {
    // High priority task should preempt low priority
    TaskStatistics high = highPriorityTask->getStatistics();
    TaskStatistics low = lowPriorityTask->getStatistics();
    
    if (high.avgLoopTimeMs > 100 && low.avgLoopTimeMs < 20) {
        Serial.println("⚠️  Possible priority inversion");
    }
}
```

**Fix**: Use priority inheritance mutexes (default in FreeRTOS)

## Debugging Tools

### 1. Task Statistics

```cpp
void printAllTaskStats() {
    Serial.println("\n=== All Task Statistics ===");
    
    TaskStatistics stats;
    
    if (commandTask) {
        stats = commandTask->getStatistics();
        printTaskStats("Command", stats);
    }
    
    if (wifiTask) {
        stats = wifiTask->getStatistics();
        printTaskStats("WiFi", stats);
    }
    
    // ... repeat for all tasks
}

void printTaskStats(const char* name, const TaskStatistics& stats) {
    Serial.printf("\n%s Task:\n", name);
    Serial.printf("  State: %d\n", (int)stats.state);
    Serial.printf("  Priority: %d\n", (int)stats.priority);
    Serial.printf("  Stack: %lu/%lu (%.1f%% used)\n",
                  stats.stackSize - stats.stackHighWaterMark,
                  stats.stackSize,
                  100.0 * (stats.stackSize - stats.stackHighWaterMark) / stats.stackSize);
    Serial.printf("  Loops: %lu\n", stats.loopCount);
    Serial.printf("  Avg/Max Loop: %lu/%lu ms\n",
                  stats.avgLoopTimeMs, stats.maxLoopTimeMs);
}
```

### 2. FreeRTOS Runtime Stats

```cpp
void printRuntimeStats() {
    char buffer[1024];
    vTaskGetRunTimeStats(buffer);
    Serial.println("\n=== FreeRTOS Runtime Stats ===");
    Serial.println(buffer);
}
```

### 3. Queue Monitoring

```cpp
void printQueueStats() {
    Serial.println("\n=== Queue Status ===");
    
    printQueueInfo("Command", commandQueue, 10);
    printQueueInfo("WiFiEvent", wifiEventQueue, 20);
    printQueueInfo("Status", statusQueue, 15);
}

void printQueueInfo(const char* name, QueueHandle_t queue, uint32_t size) {
    if (!queue) return;
    
    uint32_t count = uxQueueMessagesWaiting(queue);
    uint32_t spaces = uxQueueSpacesAvailable(queue);
    float utilization = 100.0 * count / size;
    
    Serial.printf("%s: %lu/%lu (%.0f%% full)\n", name, count, size, utilization);
    
    if (utilization > 80.0) {
        Serial.printf("  ⚠️  WARNING: Queue almost full!\n");
    }
}
```

### 4. Heap Tracking

```cpp
class HeapTracker {
public:
    void snapshot(const char* label) {
        uint32_t free = esp_get_free_heap_size();
        uint32_t min = esp_get_minimum_free_heap_size();
        
        Serial.printf("[%s] Heap: %lu free, %lu min\n", label, free, min);
        
        if (lastFree > 0) {
            int32_t delta = free - lastFree;
            if (delta < 0) {
                Serial.printf("  📉 Decreased by %ld bytes\n", -delta);
            } else if (delta > 0) {
                Serial.printf("  📈 Increased by %ld bytes\n", delta);
            }
        }
        
        lastFree = free;
    }
    
private:
    uint32_t lastFree = 0;
};

// Usage
HeapTracker tracker;
tracker.snapshot("Before operation");
performOperation();
tracker.snapshot("After operation");
```

### 5. Custom Assertions

```cpp
#define ASSERT_RTOS(condition, message) \
    do { \
        if (!(condition)) { \
            Serial.printf("ASSERTION FAILED: %s\n", message); \
            Serial.printf("  File: %s, Line: %d\n", __FILE__, __LINE__); \
            printSystemState(); \
            while(1) { delay(1000); } \
        } \
    } while(0)

// Usage
ASSERT_RTOS(xQueueSend(queue, &data, 100) == pdTRUE, "Queue send failed");
ASSERT_RTOS(lockWiFiMutex(100), "Mutex timeout");
```

## Debugging Workflow

### Step 1: Identify the Problem

```cpp
void diagnosticCheck() {
    Serial.println("\n=== System Diagnostic ===");
    
    // Check RTOS state
    Serial.printf("RTOS State: %d\n", (int)getRTOSState());
    if (!isRTOSRunning()) {
        Serial.println("❌ RTOS not running!");
        return;
    }
    
    // Check heap
    uint32_t freeHeap = esp_get_free_heap_size();
    Serial.printf("Free Heap: %lu bytes\n", freeHeap);
    if (freeHeap < 50000) {
        Serial.println("⚠️  Low memory!");
    }
    
    // Check all tasks
    checkAllTasks();
    
    // Check all queues
    checkAllQueues();
    
    // Check all mutexes
    checkAllMutexes();
}
```

### Step 2: Collect Information

Enable verbose logging:

```cpp
#define DEBUG_TASK_LIFECYCLE 1
#define DEBUG_QUEUE_OPERATIONS 1
#define DEBUG_MUTEX_OPERATIONS 1

#if DEBUG_TASK_LIFECYCLE
    Serial.printf("[%s] Task started\n", taskName);
#endif

#if DEBUG_QUEUE_OPERATIONS
    Serial.printf("[Queue] Send %s: %s\n", queueName, success ? "OK" : "FAIL");
#endif
```

### Step 3: Isolate the Issue

Disable tasks one at a time:

```cpp
// Disable non-essential tasks
if (ledTask) {
    ledTask->stop();
}

if (analysisTask) {
    analysisTask->stop();
}

// Test if issue persists
```

### Step 4: Fix and Verify

Add monitoring after fix:

```cpp
void verifyFix() {
    // Run for 10 minutes
    unsigned long start = millis();
    while (millis() - start < 600000) {
        // Check health every 10 seconds
        if ((millis() - start) % 10000 == 0) {
            if (!checkRTOSHealth()) {
                Serial.println("❌ Health check failed!");
                printSystemState();
            }
        }
        delay(100);
    }
    
    Serial.println("✓ Verification passed!");
}
```

## Best Practices for Debugging

1. **Add logging early**: Don't wait for problems
2. **Use assertions**: Catch errors at source
3. **Monitor continuously**: Track trends over time
4. **Keep statistics**: Stack, heap, loop times
5. **Test edge cases**: Queue full, low memory, high load

## See Also

- [Architecture Guide](RTOS_ARCHITECTURE.md)
- [API Reference](RTOS_API_REFERENCE.md)
- [Migration Guide](../user-guides/RTOS_MIGRATION_GUIDE.md)
- [FAQ](../user-guides/RTOS_FAQ.md)

---

*Last Updated: 2025-10-18*  
*Version: 4.1.0*
