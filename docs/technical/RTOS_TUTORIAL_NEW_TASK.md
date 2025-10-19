# Tutorial: Creating a New RTOS Task

**Difficulty**: Intermediate  
**Time**: 30-45 minutes  
**Prerequisites**: Basic C++ knowledge, familiarity with Arduino ESP32

## What You'll Learn

- How to create a custom task using `TaskBase`
- Proper task lifecycle management
- Queue-based communication
- Mutex usage for resource protection
- Task monitoring and debugging

## Tutorial Overview

We'll create a "TemperatureMonitorTask" that:
1. Reads a temperature sensor periodically
2. Sends alerts via queue when temperature is too high
3. Protects serial output with mutex
4. Runs on Core 1 with medium priority

## Step 1: Create Header File

Create `include/temperature_task.h`:

```cpp
#pragma once

#include "task_base.h"
#include "queue_manager.h"
#include "mutex_manager.h"

// Temperature alert structure
struct TemperatureAlert {
    float temperature;
    uint32_t timestamp;
    bool isWarning;  // true = warning, false = critical
};

// Temperature monitor task
class TemperatureMonitorTask : public TaskBase {
public:
    TemperatureMonitorTask();
    
    // Configuration
    void setThresholds(float warning, float critical);
    void setSensor Pin(uint8_t pin);
    
    // Status
    float getLastTemperature() const;
    bool isAlertActive() const;
    
protected:
    // TaskBase interface
    void setup() override;
    void loop() override;
    void cleanup() override;
    
private:
    // Configuration
    uint8_t sensorPin;
    float warningThreshold;
    float criticalThreshold;
    uint32_t readIntervalMs;
    
    // State
    float lastTemperature;
    bool alertActive;
    unsigned long lastReadMs;
    
    // Helper methods
    float readTemperature();
    void checkThresholds(float temp);
    void sendAlert(float temp, bool isWarning);
};

// Global instance
extern TemperatureMonitorTask* temperatureTask;

// Alert queue (add to your queue_manager.h)
extern QueueHandle_t temperatureAlertQueue;

// Queue helper functions
bool sendTemperatureAlert(const TemperatureAlert& alert, TickType_t timeout);
bool receiveTemperatureAlert(TemperatureAlert& alert, TickType_t timeout);
```

## Step 2: Implement the Task

Create `src/temperature_task.cpp`:

```cpp
#include "temperature_task.h"
#include <Arduino.h>

// Global instance
TemperatureMonitorTask* temperatureTask = nullptr;

// Alert queue
QueueHandle_t temperatureAlertQueue = nullptr;

// Queue helpers
bool sendTemperatureAlert(const TemperatureAlert& alert, TickType_t timeout) {
    if (!temperatureAlertQueue) return false;
    return xQueueSend(temperatureAlertQueue, &alert, pdMS_TO_TICKS(timeout)) == pdTRUE;
}

bool receiveTemperatureAlert(TemperatureAlert& alert, TickType_t timeout) {
    if (!temperatureAlertQueue) return false;
    return xQueueReceive(temperatureAlertQueue, &alert, pdMS_TO_TICKS(timeout)) == pdTRUE;
}

// Constructor
TemperatureMonitorTask::TemperatureMonitorTask()
    : TaskBase("TempMonitor", 
               4096,                              // 4KB stack
               TaskPriority::PRIORITY_MEDIUM,     // Medium priority
               1),                                 // Core 1
      sensorPin(34),                              // Default ADC pin
      warningThreshold(75.0),                     // 75°C warning
      criticalThreshold(85.0),                    // 85°C critical
      readIntervalMs(1000),                       // Read every second
      lastTemperature(0.0),
      alertActive(false),
      lastReadMs(0)
{
}

// Configuration methods
void TemperatureMonitorTask::setThresholds(float warning, float critical) {
    warningThreshold = warning;
    criticalThreshold = critical;
}

void TemperatureMonitorTask::setSensorPin(uint8_t pin) {
    sensorPin = pin;
}

// Status methods
float TemperatureMonitorTask::getLastTemperature() const {
    return lastTemperature;
}

bool TemperatureMonitorTask::isAlertActive() const {
    return alertActive;
}

// Task lifecycle
void TemperatureMonitorTask::setup() {
    // Create alert queue
    temperatureAlertQueue = xQueueCreate(5, sizeof(TemperatureAlert));
    if (!temperatureAlertQueue) {
        if (lockSerialMutex(100)) {
            Serial.println("ERROR: Failed to create temperature alert queue");
            unlockSerialMutex();
        }
        requestStop();
        return;
    }
    
    // Configure sensor pin
    pinMode(sensorPin, INPUT);
    
    // Initial read
    lastTemperature = readTemperature();
    
    if (lockSerialMutex(100)) {
        Serial.println("Temperature Monitor: Initialized");
        Serial.printf("  Sensor Pin: %d\n", sensorPin);
        Serial.printf("  Warning Threshold: %.1f°C\n", warningThreshold);
        Serial.printf("  Critical Threshold: %.1f°C\n", criticalThreshold);
        unlockSerialMutex();
    }
}

void TemperatureMonitorTask::loop() {
    unsigned long now = millis();
    
    // Check if it's time to read
    if (now - lastReadMs >= readIntervalMs) {
        lastReadMs = now;
        
        // Read temperature
        float temp = readTemperature();
        lastTemperature = temp;
        
        // Check thresholds
        checkThresholds(temp);
        
        // Periodic status (every 10 seconds)
        static unsigned long lastStatusMs = 0;
        if (now - lastStatusMs >= 10000) {
            lastStatusMs = now;
            
            if (lockSerialMutex(50)) {
                Serial.printf("Temperature: %.1f°C\n", temp);
                unlockSerialMutex();
            }
        }
    }
    
    // Yield CPU
    vTaskDelay(pdMS_TO_TICKS(100));
}

void TemperatureMonitorTask::cleanup() {
    // Delete queue
    if (temperatureAlertQueue) {
        vQueueDelete(temperatureAlertQueue);
        temperatureAlertQueue = nullptr;
    }
    
    if (lockSerialMutex(100)) {
        Serial.println("Temperature Monitor: Stopped");
        unlockSerialMutex();
    }
}

// Helper methods
float TemperatureMonitorTask::readTemperature() {
    // Read ADC value (0-4095)
    int adcValue = analogRead(sensorPin);
    
    // Convert to voltage (0-3.3V)
    float voltage = adcValue * (3.3 / 4095.0);
    
    // Convert to temperature (example for TMP36: 10mV/°C, 500mV offset)
    // Adjust formula for your specific sensor!
    float tempC = (voltage - 0.5) * 100.0;
    
    return tempC;
}

void TemperatureMonitorTask::checkThresholds(float temp) {
    bool wasCritical = alertActive;
    
    if (temp >= criticalThreshold) {
        // Critical temperature
        if (!alertActive) {
            alertActive = true;
            sendAlert(temp, false);  // Critical alert
            
            if (lockSerialMutex(100)) {
                Serial.printf("⚠️  CRITICAL: Temperature %.1f°C exceeds %.1f°C!\n", 
                            temp, criticalThreshold);
                unlockSerialMutex();
            }
        }
    } else if (temp >= warningThreshold) {
        // Warning temperature
        if (!wasCritical) {  // Only send if not previously critical
            sendAlert(temp, true);  // Warning alert
            
            if (lockSerialMutex(100)) {
                Serial.printf("⚠️  WARNING: Temperature %.1f°C exceeds %.1f°C\n", 
                            temp, warningThreshold);
                unlockSerialMutex();
            }
        }
        alertActive = false;  // Not critical
    } else {
        // Normal temperature
        if (alertActive || wasCritical) {
            if (lockSerialMutex(100)) {
                Serial.printf("✓ Temperature normal: %.1f°C\n", temp);
                unlockSerialMutex();
            }
        }
        alertActive = false;
    }
}

void TemperatureMonitorTask::sendAlert(float temp, bool isWarning) {
    TemperatureAlert alert;
    alert.temperature = temp;
    alert.timestamp = millis();
    alert.isWarning = isWarning;
    
    if (!sendTemperatureAlert(alert, 100)) {
        if (lockSerialMutex(50)) {
            Serial.println("WARNING: Alert queue full!");
            unlockSerialMutex();
        }
    }
}
```

## Step 3: Integrate into main.cpp

Add to your `main.cpp`:

```cpp
#include "temperature_task.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize RTOS
    if (!initializeRTOS()) {
        Serial.println("RTOS initialization failed!");
        return;
    }
    
    // Create and start temperature monitor
    temperatureTask = new TemperatureMonitorTask();
    
    // Optional: Configure thresholds
    temperatureTask->setThresholds(70.0, 80.0);  // Warning at 70°C, critical at 80°C
    
    // Optional: Set sensor pin
    temperatureTask->setSensorPin(34);  // ADC1_CH6
    
    // Start the task
    if (!temperatureTask->start()) {
        Serial.println("Failed to start temperature task!");
        delete temperatureTask;
        temperatureTask = nullptr;
    }
    
    Serial.println("Temperature monitoring started");
}

void loop() {
    // Check for temperature alerts
    TemperatureAlert alert;
    if (receiveTemperatureAlert(alert, 0)) {  // Non-blocking check
        if (alert.isWarning) {
            Serial.printf("[%lu] WARNING: %.1f°C\n", alert.timestamp, alert.temperature);
            // Take action (e.g., slow down CPU, enable cooling)
        } else {
            Serial.printf("[%lu] CRITICAL: %.1f°C\n", alert.timestamp, alert.temperature);
            // Take emergency action (e.g., shutdown)
        }
    }
    
    delay(100);
}
```

## Step 4: Build and Test

1. **Compile the code**:
   ```bash
   pio run -e esp32dev_rtos
   ```

2. **Upload to ESP32**:
   ```bash
   pio run -e esp32dev_rtos --target upload
   ```

3. **Monitor output**:
   ```bash
   pio device monitor
   ```

4. **Expected output**:
   ```
   Temperature Monitor: Initialized
   Sensor Pin: 34
   Warning Threshold: 70.0°C
   Critical Threshold: 80.0°C
   Temperature monitoring started
   Temperature: 25.3°C
   Temperature: 25.5°C
   ...
```

## Step 5: Monitor Task Statistics

Add this to see task performance:

```cpp
void printTaskStats() {
    if (temperatureTask) {
        Serial.println("\n=== Temperature Task Statistics ===");
        TaskStatistics stats = temperatureTask->getStatistics();
        
        Serial.printf("State: %d\n", (int)stats.state);
        Serial.printf("Loop Count: %lu\n", stats.loopCount);
        Serial.printf("Stack High Water: %lu bytes (%.1f%% used)\n",
                    stats.stackHighWaterMark,
                    100.0 * (stats.stackSize - stats.stackHighWaterMark) / stats.stackSize);
        Serial.printf("Avg Loop Time: %lu ms\n", stats.avgLoopTimeMs);
        Serial.printf("Max Loop Time: %lu ms\n", stats.maxLoopTimeMs);
    }
}

void loop() {
    static unsigned long lastStatsMs = 0;
    if (millis() - lastStatsMs >= 30000) {  // Every 30 seconds
        lastStatsMs = millis();
        printTaskStats();
    }
    // ... rest of loop
}
```

## Troubleshooting

### Task Doesn't Start

**Problem**: `start()` returns false

**Solutions**:
```cpp
if (!temperatureTask->start()) {
    Serial.println("Task start failed");
    
    // Check heap
    Serial.printf("Free heap: %lu\n", esp_get_free_heap_size());
    if (esp_get_free_heap_size() < 10000) {
        Serial.println("Insufficient memory");
    }
    
    // Check RTOS
    if (!isRTOSRunning()) {
        Serial.println("RTOS not initialized");
    }
}
```

### Stack Overflow

**Problem**: Crash with "Stack canary watchpoint triggered"

**Solution**: Increase stack size in constructor:
```cpp
TemperatureMonitorTask()
    : TaskBase("TempMonitor", 
               8192,  // Increased from 4096
               // ...
```

### Queue Full

**Problem**: Alerts are dropped

**Solution**: Increase queue size or process alerts faster:
```cpp
// In setup()
temperatureAlertQueue = xQueueCreate(10, sizeof(TemperatureAlert));  // Was 5

// Or drain queue more frequently in main loop
while (receiveTemperatureAlert(alert, 0)) {
    processAlert(alert);
}
```

## Best Practices Demonstrated

1. **RAII Initialization**: Queue created in `setup()`, deleted in `cleanup()`
2. **Mutex Protection**: All Serial calls protected
3. **Non-Blocking Operations**: Uses timeouts, never blocks indefinitely
4. **CPU Yielding**: `vTaskDelay()` in loop to allow other tasks
5. **Error Handling**: Checks all return values
6. **Resource Cleanup**: Properly cleans up in `cleanup()`
7. **Configurable**: Thresholds and pins configurable
8. **Observable**: Statistics and status available

## Next Steps

- Try [Tutorial: Using Queues](RTOS_TUTORIAL_QUEUES.md) for advanced queue patterns
- Read [Tutorial: Debugging RTOS](RTOS_TUTORIAL_DEBUGGING.md) for debugging techniques
- Review [API Reference](RTOS_API_REFERENCE.md) for more APIs

---

*Last Updated: 2025-10-18*  
*Version: 4.1.0*
