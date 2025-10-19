# Tutorial: Using Queues for Communication

**Difficulty**: Intermediate  
**Time**: 20-30 minutes  
**Prerequisites**: Completed "Creating a New Task" tutorial

## What You'll Learn

- Queue-based inter-task communication patterns
- Producer-consumer pattern
- Request-response pattern
- Event broadcasting
- Queue monitoring and debugging

## Queue Communication Patterns

### Pattern 1: Producer-Consumer

**Use Case**: One task produces data, another consumes it

```cpp
// Producer Task
class SensorTask : public TaskBase {
protected:
    void loop() override {
        SensorData data = readSensor();
        if (sendSensorData(data, 100)) {
            Serial.println("Data sent");
        } else {
            Serial.println("Queue full");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
};

// Consumer Task
class ProcessorTask : public TaskBase {
protected:
    void loop() override {
        SensorData data;
        if (receiveSensorData(data, 1000)) {
            processData(data);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
};
```

### Pattern 2: Request-Response

**Use Case**: Task sends request, waits for response

```cpp
// Requester
void sendRequest() {
    Request req;
    req.id = millis();
    req.type = REQUEST_STATUS;
    
    sendRequestQueue(req, 100);
    
    // Wait for response
    Response resp;
    while (receiveResponseQueue(resp, 5000)) {
        if (resp.requestId == req.id) {
            // Got our response
            processResponse(resp);
            break;
        }
    }
}

// Responder Task
void loop() override {
    Request req;
    if (receiveRequestQueue(req, 100)) {
        Response resp = handleRequest(req);
        resp.requestId = req.id;
        sendResponseQueue(resp, 100);
    }
}
```

### Pattern 3: Event Broadcasting

**Use Case**: One event sent to multiple listeners

```cpp
// Broadcaster
void broadcastEvent(EventType type) {
    Event event;
    event.type = type;
    event.timestamp = millis();
    
    // Send to multiple queues
    sendToLEDQueue(event, 10);
    sendToWebQueue(event, 10);
    sendToLogQueue(event, 10);
}

// Each listener has its own queue
void LEDTask::loop() override {
    Event event;
    if (receiveFromLEDQueue(event, 100)) {
        updateLEDForEvent(event);
    }
}
```

### Pattern 4: Priority Queue (Simulation)

**Use Case**: Urgent messages processed first

```cpp
// High priority queue
QueueHandle_t urgentQueue;   // Size: 5
QueueHandle_t normalQueue;   // Size: 20

// Sender chooses queue by priority
void sendMessage(Message msg, bool urgent) {
    if (urgent) {
        sendToUrgentQueue(msg, 100);
    } else {
        sendToNormalQueue(msg, 100);
    }
}

// Receiver checks urgent queue first
void loop() override {
    Message msg;
    
    // Check urgent queue first
    if (receiveFromUrgentQueue(msg, 0)) {
        processMessage(msg);
        return;
    }
    
    // Then check normal queue
    if (receiveFromNormalQueue(msg, 100)) {
        processMessage(msg);
    }
}
```

## Complete Example: Data Pipeline

Create a data processing pipeline: Sensor → Filter → Logger

```cpp
// Data structures
struct RawData {
    float value;
    uint32_t timestamp;
};

struct FilteredData {
    float filtered;
    float raw;
    uint32_t timestamp;
};

// Queues
QueueHandle_t rawDataQueue;
QueueHandle_t filteredDataQueue;

// Task 1: Sensor Reader
class SensorReaderTask : public TaskBase {
public:
    SensorReaderTask() 
        : TaskBase("SensorReader", 4096, TaskPriority::PRIORITY_HIGH, 1) {}
    
protected:
    void setup() override {
        pinMode(SENSOR_PIN, INPUT);
    }
    
    void loop() override {
        RawData data;
        data.value = analogRead(SENSOR_PIN) * (3.3 / 4095.0);
        data.timestamp = millis();
        
        if (!xQueueSend(rawDataQueue, &data, pdMS_TO_TICKS(10))) {
            Serial.println("Raw queue full!");
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));  // 10 Hz sampling
    }
    
    void cleanup() override {}
};

// Task 2: Data Filter
class DataFilterTask : public TaskBase {
public:
    DataFilterTask() 
        : TaskBase("DataFilter", 4096, TaskPriority::PRIORITY_MEDIUM, 1),
          alpha(0.1) {}  // Filter coefficient
    
protected:
    void setup() override {
        lastFiltered = 0.0;
    }
    
    void loop() override {
        RawData raw;
        if (xQueueReceive(rawDataQueue, &raw, pdMS_TO_TICKS(100))) {
            // Apply low-pass filter
            float filtered = alpha * raw.value + (1 - alpha) * lastFiltered;
            lastFiltered = filtered;
            
            FilteredData output;
            output.filtered = filtered;
            output.raw = raw.value;
            output.timestamp = raw.timestamp;
            
            xQueueSend(filteredDataQueue, &output, pdMS_TO_TICKS(10));
        }
    }
    
    void cleanup() override {}
    
private:
    float alpha;
    float lastFiltered;
};

// Task 3: Data Logger
class DataLoggerTask : public TaskBase {
public:
    DataLoggerTask() 
        : TaskBase("DataLogger", 4096, TaskPriority::PRIORITY_LOW, 1) {}
    
protected:
    void setup() override {
        // Initialize SD card or file system
    }
    
    void loop() override {
        FilteredData data;
        if (xQueueReceive(filteredDataQueue, &data, pdMS_TO_TICKS(100))) {
            // Log to file
            MutexGuard guard(serialMutex, 50);
            if (guard.acquired()) {
                Serial.printf("[%lu] Raw: %.2f, Filtered: %.2f\n",
                            data.timestamp, data.raw, data.filtered);
            }
        }
    }
    
    void cleanup() override {}
};

// Main setup
void setup() {
    Serial.begin(115200);
    
    // Create queues
    rawDataQueue = xQueueCreate(10, sizeof(RawData));
    filteredDataQueue = xQueueCreate(10, sizeof(FilteredData));
    
    // Create and start tasks
    SensorReaderTask* sensor = new SensorReaderTask();
    DataFilterTask* filter = new DataFilterTask();
    DataLoggerTask* logger = new DataLoggerTask();
    
    sensor->start();
    filter->start();
    logger->start();
}
```

## Queue Monitoring

Monitor queue health to prevent overflow:

```cpp
void monitorQueues() {
    Serial.println("\n=== Queue Status ===");
    
    // Command queue
    uint32_t cmdCount = uxQueueMessagesWaiting(commandQueue);
    uint32_t cmdSpaces = uxQueueSpacesAvailable(commandQueue);
    Serial.printf("Command Queue: %lu/%lu (%.0f%% full)\n",
                  cmdCount, cmdCount + cmdSpaces,
                  100.0 * cmdCount / (cmdCount + cmdSpaces));
    
    // WiFi event queue
    uint32_t wifiCount = uxQueueMessagesWaiting(wifiEventQueue);
    uint32_t wifiSpaces = uxQueueSpacesAvailable(wifiEventQueue);
    Serial.printf("WiFi Queue: %lu/%lu (%.0f%% full)\n",
                  wifiCount, wifiCount + wifiSpaces,
                  100.0 * wifiCount / (wifiCount + wifiSpaces));
}

// Call periodically
void loop() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck >= 10000) {
        lastCheck = millis();
        monitorQueues();
    }
}
```

## Best Practices

### 1. Choose Appropriate Queue Size

```cpp
// Too small - frequent overflows
QueueHandle_t tooSmall = xQueueCreate(2, sizeof(Data));

// Too large - wastes memory
QueueHandle_t tooLarge = xQueueCreate(1000, sizeof(Data));

// Just right - based on production rate and consumption rate
// Formula: size = (production_rate / consumption_rate) * safety_factor
QueueHandle_t justRight = xQueueCreate(10, sizeof(Data));
```

### 2. Always Use Timeouts

```cpp
// BAD - can deadlock
xQueueSend(queue, &data, portMAX_DELAY);

// GOOD - handles timeout
if (xQueueSend(queue, &data, pdMS_TO_TICKS(100)) != pdTRUE) {
    // Handle timeout - queue full
    Serial.println("Queue send timeout");
}
```

### 3. Handle Queue Full Conditions

```cpp
void robustSend(QueueHandle_t queue, Data* data) {
    // Try to send
    if (xQueueSend(queue, data, 0) == pdTRUE) {
        return;  // Success
    }
    
    // Queue full - try dropping oldest
    Data old;
    if (xQueueReceive(queue, &old, 0) == pdTRUE) {
        // Dropped oldest, try again
        xQueueSend(queue, data, 0);
        Serial.println("Dropped old data");
    }
}
```

### 4. Use Peek for Lookahead

```cpp
void checkNextMessage() {
    Message msg;
    
    // Peek at next message without removing
    if (xQueuePeek(queue, &msg, 0) == pdTRUE) {
        if (msg.priority == URGENT) {
            // Process immediately
            xQueueReceive(queue, &msg, 0);
            handleUrgent(msg);
        } else {
            // Process later
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
```

## Troubleshooting

### Queue Overflow

**Symptom**: `xQueueSend()` returns pdFALSE

**Solutions**:
1. Increase queue size
2. Increase consumer priority
3. Add multiple consumers
4. Drop old messages
5. Reduce producer rate

### Queue Starvation

**Symptom**: `xQueueReceive()` always returns pdFALSE

**Solutions**:
1. Verify producer is running
2. Check for deadlocks
3. Increase producer priority
4. Reduce consumer delay

### Memory Issues

**Symptom**: Queue creation fails

**Solution**:
```cpp
QueueHandle_t queue = xQueueCreate(10, sizeof(LargeStruct));
if (queue == NULL) {
    Serial.printf("Free heap: %lu\n", esp_get_free_heap_size());
    // Reduce queue size or struct size
}
```

## Next Steps

- Try [Tutorial: Debugging RTOS](RTOS_TUTORIAL_DEBUGGING.md)
- Review [API Reference](RTOS_API_REFERENCE.md)
- Read [Architecture Guide](RTOS_ARCHITECTURE.md)

---

*Last Updated: 2025-10-18*  
*Version: 4.1.0*
