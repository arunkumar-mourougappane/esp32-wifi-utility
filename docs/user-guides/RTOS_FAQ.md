# RTOS Frequently Asked Questions (FAQ)

**Version**: 4.1.0  
**Last Updated**: 2025-10-18

## General Questions

### What is RTOS?

RTOS (Real-Time Operating System) is a lightweight operating system designed for embedded systems. 
FreeRTOS provides task scheduling, inter-task communication, and synchronization primitives.

### Why use RTOS instead of Arduino's simple loop()?

**RTOS Advantages**:
- Multiple tasks run concurrently
- Responsive to user input (no blocking)
- Better CPU utilization (dual-core)
- Easier to manage complex applications
- Built-in synchronization and communication

### Is v4.x compatible with v3.x?

**Yes, mostly**. Serial commands work identically, but:
- Commands execute asynchronously (non-blocking)
- Custom code may need updates for concurrent operation
- See [Migration Guide](RTOS_MIGRATION_GUIDE.md) for details

## Task Questions

### How many tasks can I create?

**Theoretical**: Limited by memory  
**Practical**: 15-20 tasks maximum

Each task consumes:
- Stack memory (4-12 KB typical)
- Task control block (~500 bytes)
- Scheduling overhead

### What priority should my task have?

**Priority Guidelines**:
- **HIGHEST (5)**: User input, critical timing
- **HIGH (4)**: Network operations, time-sensitive
- **MEDIUM (3)**: General processing, web server
- **LOW (2)**: UI updates, non-critical
- **VERY_LOW (1)**: Background, analysis

**Rule**: Match priority to importance and timing requirements

### How much stack does my task need?

**Formula**: `Required Stack = Peak Usage × Safety Factor (1.25-1.5)`

**Measure usage**:
```cpp
TaskStatistics stats = myTask->getStatistics();
Serial.printf("Stack used: %lu/%lu bytes\n",
              stats.stackSize - stats.stackHighWaterMark,
              stats.stackSize);
```

**Typical Sizes**:
- Simple tasks: 2-4 KB
- Network tasks: 6-8 KB
- Web server: 10-12 KB

### Can tasks share variables?

**Yes, but carefully**:

**Wrong** (race condition):
```cpp
int counter = 0;  // Shared

// Task A
counter++;

// Task B
counter++;
```

**Right** (mutex protected):
```cpp
int counter = 0;
SemaphoreHandle_t counterMutex;

// Task A
xSemaphoreTake(counterMutex, 100);
counter++;
xSemaphoreGive(counterMutex);

// Task B
xSemaphoreTake(counterMutex, 100);
counter++;
xSemaphoreGive(counterMutex);
```

**Better** (avoid sharing):
```cpp
// Use queues instead
struct CounterMsg { int delta; };
// Tasks send messages instead of sharing variable
```

### Which CPU core should my task run on?

**Core 0** (Protocol CPU):
- WiFi operations
- Network stack
- Protocol processing

**Core 1** (Application CPU):
- User interface
- Application logic
- Non-network tasks

**No Affinity** (tskNO_AFFINITY):
- Let scheduler decide
- Good for balanced workload

### Why does my task keep crashing?

**Common Causes**:

1. **Stack overflow**: Increase stack size
2. **Watchdog timeout**: Add vTaskDelay() calls
3. **Null pointer**: Check all pointers
4. **Heap exhaustion**: Reduce memory usage
5. **Recursive calls**: Avoid or limit depth

**Debug**:
```cpp
TaskStatistics stats = myTask->getStatistics();
if (stats.state == TaskState::ERROR) {
    Serial.println("Task crashed!");
    // Check logs before crash
}
```

## Queue Questions

### How big should my queue be?

**Formula**: `Size = (Production Rate / Consumption Rate) × Safety Factor`

**Example**:
- Producer: 10 items/sec
- Consumer: 12 items/sec
- Size = (10/12) × 2 = 2 items (round up to 5 for safety)

**Monitor utilization**:
```cpp
uint32_t count = uxQueueMessagesWaiting(queue);
uint32_t total = count + uxQueueSpacesAvailable(queue);
float util = 100.0 * count / total;

if (util > 80.0) {
    Serial.println("Queue almost full - increase size");
}
```

### What happens when queue is full?

Depends on timeout:

**No timeout** (0):
```cpp
xQueueSend(queue, &item, 0);  // Returns immediately
// Returns pdFALSE if full
```

**With timeout**:
```cpp
xQueueSend(queue, &item, pdMS_TO_TICKS(100));
// Waits up to 100ms for space
// Returns pdFALSE if still full
```

**Infinite timeout** (NOT RECOMMENDED):
```cpp
xQueueSend(queue, &item, portMAX_DELAY);
// Waits forever - can deadlock!
```

### Can multiple tasks send to same queue?

**Yes!** Queues are thread-safe. Multiple producers and consumers are fine.

```cpp
// Task A sends
xQueueSend(sharedQueue, &dataA, 100);

// Task B sends
xQueueSend(sharedQueue, &dataB, 100);

// Task C receives
xQueueReceive(sharedQueue, &data, 100);
```

### Should I use queue or global variable?

**Use Queue**:
- ✅ Thread-safe automatically
- ✅ Decouples tasks
- ✅ Built-in flow control
- ✅ Can buffer multiple items

**Use Global** (with mutex):
- Configuration values
- Status flags
- Small, infrequently updated data

## Mutex Questions

### When do I need a mutex?

**Need Mutex**:
- Shared variables modified by multiple tasks
- Hardware resources (Serial, SPI, I2C)
- WiFi library calls
- File system operations

**Don't Need Mutex**:
- Task-local variables
- Queue operations (already thread-safe)
- Atomic operations
- Read-only data

### What's the difference between mutex and semaphore?

**Mutex** (Mutual Exclusion):
- Binary (locked/unlocked)
- Has owner (task that locked it)
- Priority inheritance
- Must unlock from same task

**Binary Semaphore**:
- Similar to mutex
- No owner concept
- No priority inheritance
- Can signal from any task

**Counting Semaphore**:
- Count > 1
- Used for resource pools
- Example: 3 available buffers

**Use Case**:
```cpp
// Mutual exclusion → Use Mutex
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

// Signaling between tasks → Use Binary Semaphore
SemaphoreHandle_t signal = xSemaphoreCreateBinary();

// Resource pool → Use Counting Semaphore
SemaphoreHandle_t pool = xSemaphoreCreateCounting(5, 5);  // 5 resources
```

### How do I avoid deadlocks?

**Rules**:

1. **Always use timeouts**:
```cpp
if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    // Got mutex
    xSemaphoreGive(mutex);
}
```

2. **Acquire in same order**:
```cpp
// All tasks do: mutexA → mutexB
// Never: mutexB → mutexA
```

3. **Keep critical sections short**:
```cpp
// WRONG
xSemaphoreTake(mutex, 100);
delay(1000);  // Holds mutex too long!
xSemaphoreGive(mutex);

// RIGHT
xSemaphoreTake(mutex, 100);
quickOperation();
xSemaphoreGive(mutex);
```

4. **Use RAII**:
```cpp
MutexGuard guard(mutex, 100);
// Mutex automatically released when guard destructs
```

### Can I call xSemaphoreGive from ISR?

**No!** Use ISR-safe version:

```cpp
// Regular function
xSemaphoreGive(semaphore);

// From ISR
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
xSemaphoreGiveFromISR(semaphore, &xHigherPriorityTaskWoken);
portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
```

## Performance Questions

### Why is my system slow?

**Common Causes**:

1. **Incorrect priorities**: Adjust task priorities
2. **Blocking operations**: Use async alternatives
3. **Mutex contention**: Reduce critical section time
4. **Excessive delays**: Lower vTaskDelay() values
5. **CPU overload**: Optimize or remove tasks

**Debug**:
```cpp
vTaskGetRunTimeStats(buffer);
Serial.println(buffer);
// Shows CPU usage per task
```

### How do I improve response time?

1. **Increase task priority**:
```cpp
myTask->setPriority(TaskPriority::PRIORITY_HIGH);
```

2. **Reduce loop delays**:
```cpp
vTaskDelay(pdMS_TO_TICKS(1));  // Was 10
```

3. **Pin to correct core**:
```cpp
// Network tasks → Core 0
// UI tasks → Core 1
```

4. **Optimize critical paths**:
```cpp
// Move heavy computation out of critical section
prepareData();
xSemaphoreTake(mutex, 100);
quickUpdate();
xSemaphoreGive(mutex);
```

### What's the maximum command throughput?

**Measured**: 150-300 commands/second

**Limited by**:
- WiFi operation time (not RTOS)
- Command queue size (10 items)
- WiFi task processing speed

**Improve**:
- Increase command queue size
- Increase WiFi task priority
- Batch operations

## Memory Questions

### How much RAM does RTOS use?

**FreeRTOS overhead**: ~10-15 KB

**Per task**: Stack size + ~500 bytes

**Per queue**: Size × Item size + ~200 bytes

**Example System** (5 tasks):
```
Task stacks:     38 KB
Task overhead:    2.5 KB
Queues:           2 KB
FreeRTOS:        12 KB
Total:          ~55 KB
```

**Remaining**: ~265 KB for application

### How do I reduce memory usage?

1. **Optimize stack sizes**:
```cpp
// Measure actual usage first!
TaskStatistics stats = myTask->getStatistics();
```

2. **Reduce queue sizes**:
```cpp
// From 20 to 10 items
xQueueCreate(10, sizeof(Data));
```

3. **Use stack allocation**:
```cpp
// HEAP (uses more memory)
char* buffer = new char[1024];
delete[] buffer;

// STACK (faster, less memory)
char buffer[1024];
```

4. **Disable unused features**:
```cpp
// In config.h
#define USE_ANALYSIS_TASK 0  // Saves 8 KB
```

### Can I use String class in tasks?

**Yes, but carefully**. String can cause heap fragmentation.

**Better alternatives**:
```cpp
// Instead of String
String msg = "Hello " + name;

// Use char arrays
char msg[64];
snprintf(msg, sizeof(msg), "Hello %s", name);
```

## Error Handling

### What happens when a task crashes?

**Default behavior**:
- Task stops
- Other tasks continue
- System remains running

**Detection**:
```cpp
if (myTask->getState() == TaskState::ERROR) {
    Serial.println("Task crashed!");
    // Restart or handle error
}
```

### How do I restart a crashed task?

```cpp
if (myTask->getState() == TaskState::ERROR) {
    delete myTask;
    myTask = new MyTask();
    myTask->start();
}
```

### What causes "Stack canary watchpoint triggered"?

**Cause**: Stack overflow

**Fix**:
1. Increase stack size
2. Reduce local variable size
3. Avoid deep recursion
4. Use heap for large buffers

---

## See Also

- [Architecture Guide](../technical/RTOS_ARCHITECTURE.md)
- [API Reference](../technical/RTOS_API_REFERENCE.md)
- [Migration Guide](RTOS_MIGRATION_GUIDE.md)
- [Tutorial: Creating Tasks](../technical/RTOS_TUTORIAL_NEW_TASK.md)
- [Tutorial: Using Queues](../technical/RTOS_TUTORIAL_QUEUES.md)
- [Tutorial: Debugging](../technical/RTOS_TUTORIAL_DEBUGGING.md)

---

*Last Updated: 2025-10-18*  
*Version: 4.1.0*
