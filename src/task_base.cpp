#include "task_base.h"

#ifdef USE_RTOS

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ==========================================
// TASK BASE CLASS IMPLEMENTATION
// ==========================================

TaskBase::TaskBase(const char* taskName, uint32_t stackSize, TaskPriority priority, BaseType_t coreId)
    : taskHandle(NULL)
    , name(taskName)
    , stackSizeBytes(stackSize)
    , taskPriority(priority)
    , cpuCore(coreId)
    , currentState(TaskState::NOT_CREATED)
    , stopRequested(false)
    , loopStartTimeMs(0)
    , lastStatsUpdateMs(0) {
    
    // Initialize statistics
    stats.name = name;
    stats.state = currentState;
    stats.priority = taskPriority;
    stats.stackSize = stackSizeBytes;
    stats.stackHighWaterMark = stackSizeBytes;
    stats.loopCount = 0;
    stats.totalRunTimeMs = 0;
    stats.lastRunTimeMs = 0;
    stats.maxLoopTimeMs = 0;
    stats.avgLoopTimeMs = 0;
}

TaskBase::~TaskBase() {
    if (taskHandle != NULL) {
        stop(5000);
    }
}

bool TaskBase::start() {
    if (currentState != TaskState::NOT_CREATED && currentState != TaskState::STOPPED) {
        Serial.printf("[Task:%s] Already started or starting\n", name);
        return false;
    }
    
    Serial.printf("[Task:%s] Creating task (stack: %u bytes, priority: %d, core: %d)\n",
                 name, stackSizeBytes, static_cast<int>(taskPriority), cpuCore);
    
    currentState = TaskState::CREATED;
    stopRequested = false;
    
    BaseType_t result;
    
    if (cpuCore == tskNO_AFFINITY) {
        // Create without core affinity
        result = xTaskCreate(
            taskFunction,
            name,
            stackSizeBytes / sizeof(StackType_t),  // Convert bytes to words
            this,
            static_cast<UBaseType_t>(taskPriority),
            &taskHandle
        );
    } else {
        // Create with core affinity
        result = xTaskCreatePinnedToCore(
            taskFunction,
            name,
            stackSizeBytes / sizeof(StackType_t),  // Convert bytes to words
            this,
            static_cast<UBaseType_t>(taskPriority),
            &taskHandle,
            cpuCore
        );
    }
    
    if (result != pdPASS) {
        Serial.printf("[Task:%s] ERROR: Failed to create task\n", name);
        currentState = TaskState::ERROR;
        return false;
    }
    
    currentState = TaskState::RUNNING;
    Serial.printf("[Task:%s] Task created and started\n", name);
    return true;
}

bool TaskBase::stop(uint32_t timeoutMs) {
    if (currentState != TaskState::RUNNING && currentState != TaskState::SUSPENDED) {
        Serial.printf("[Task:%s] Not running\n", name);
        return true;
    }
    
    Serial.printf("[Task:%s] Stopping task...\n", name);
    currentState = TaskState::STOPPING;
    stopRequested = true;
    
    // Wait for task to acknowledge stop
    unsigned long startMs = millis();
    while (currentState != TaskState::STOPPED && (millis() - startMs < timeoutMs)) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    if (currentState != TaskState::STOPPED) {
        Serial.printf("[Task:%s] WARNING: Task did not stop within timeout\n", name);
        // Force delete the task
        if (taskHandle != NULL) {
            vTaskDelete(taskHandle);
            taskHandle = NULL;
        }
        currentState = TaskState::STOPPED;
        return false;
    }
    
    Serial.printf("[Task:%s] Task stopped\n", name);
    return true;
}

bool TaskBase::suspend() {
    if (taskHandle == NULL || currentState != TaskState::RUNNING) {
        return false;
    }
    
    vTaskSuspend(taskHandle);
    currentState = TaskState::SUSPENDED;
    Serial.printf("[Task:%s] Task suspended\n", name);
    return true;
}

bool TaskBase::resume() {
    if (taskHandle == NULL || currentState != TaskState::SUSPENDED) {
        return false;
    }
    
    vTaskResume(taskHandle);
    currentState = TaskState::RUNNING;
    Serial.printf("[Task:%s] Task resumed\n", name);
    return true;
}

bool TaskBase::isRunning() const {
    return currentState == TaskState::RUNNING;
}

TaskState TaskBase::getState() const {
    return currentState;
}

TaskHandle_t TaskBase::getHandle() const {
    return taskHandle;
}

const char* TaskBase::getName() const {
    return name;
}

TaskPriority TaskBase::getPriority() const {
    return taskPriority;
}

void TaskBase::setPriority(TaskPriority newPriority) {
    taskPriority = newPriority;
    if (taskHandle != NULL) {
        vTaskPrioritySet(taskHandle, static_cast<UBaseType_t>(newPriority));
    }
}

TaskStatistics TaskBase::getStatistics() {
    updateStatistics();
    return stats;
}

void TaskBase::printStatistics() {
    updateStatistics();
    
    Serial.printf("\n=== Task Statistics: %s ===\n", stats.name);
    
    // State
    Serial.print("State: ");
    switch (stats.state) {
        case TaskState::NOT_CREATED:
            Serial.println("NOT_CREATED");
            break;
        case TaskState::CREATED:
            Serial.println("CREATED");
            break;
        case TaskState::RUNNING:
            Serial.println("RUNNING");
            break;
        case TaskState::SUSPENDED:
            Serial.println("SUSPENDED");
            break;
        case TaskState::STOPPING:
            Serial.println("STOPPING");
            break;
        case TaskState::STOPPED:
            Serial.println("STOPPED");
            break;
        case TaskState::ERROR:
            Serial.println("ERROR");
            break;
    }
    
    // Priority
    Serial.printf("Priority: %d", static_cast<int>(stats.priority));
    switch (stats.priority) {
        case TaskPriority::IDLE:
            Serial.println(" (IDLE)");
            break;
        case TaskPriority::PRIORITY_LOW:
            Serial.println(" (LOW)");
            break;
        case TaskPriority::PRIORITY_MEDIUM:
            Serial.println(" (MEDIUM)");
            break;
        case TaskPriority::PRIORITY_HIGH:
            Serial.println(" (HIGH)");
            break;
        case TaskPriority::REALTIME:
            Serial.println(" (REALTIME)");
            break;
    }
    
    // Stack
    Serial.printf("Stack: %u bytes allocated\n", stats.stackSize);
    Serial.printf("Stack High Water Mark: %u bytes (%.1f%% used)\n",
                 stats.stackHighWaterMark,
                 ((stats.stackSize - stats.stackHighWaterMark) * 100.0) / stats.stackSize);
    
    // Runtime
    Serial.printf("Loop Count: %u\n", stats.loopCount);
    Serial.printf("Total Runtime: %lu ms (%.2f seconds)\n",
                 stats.totalRunTimeMs, stats.totalRunTimeMs / 1000.0);
    Serial.printf("Last Loop Time: %lu ms\n", stats.lastRunTimeMs);
    Serial.printf("Average Loop Time: %lu ms\n", stats.avgLoopTimeMs);
    Serial.printf("Max Loop Time: %lu ms\n", stats.maxLoopTimeMs);
    
    Serial.println("==============================\n");
}

void TaskBase::resetStatistics() {
    stats.loopCount = 0;
    stats.totalRunTimeMs = 0;
    stats.lastRunTimeMs = 0;
    stats.maxLoopTimeMs = 0;
    stats.avgLoopTimeMs = 0;
}

void TaskBase::taskDelay(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

bool TaskBase::isStopRequested() const {
    return stopRequested;
}

void TaskBase::yield() {
    taskYIELD();
}

void TaskBase::taskFunction(void* parameter) {
    TaskBase* task = static_cast<TaskBase*>(parameter);
    
    Serial.printf("[Task:%s] Task function started\n", task->name);
    
    // Call setup
    task->setup();
    
    unsigned long taskStartMs = millis();
    
    // Main task loop
    while (!task->stopRequested) {
        task->loopStartTimeMs = millis();
        
        // Call derived class loop
        task->loop();
        
        // Update statistics
        task->stats.loopCount++;
        unsigned long loopTime = millis() - task->loopStartTimeMs;
        task->stats.lastRunTimeMs = loopTime;
        task->stats.totalRunTimeMs = millis() - taskStartMs;
        
        if (loopTime > task->stats.maxLoopTimeMs) {
            task->stats.maxLoopTimeMs = loopTime;
        }
        
        if (task->stats.loopCount > 0) {
            task->stats.avgLoopTimeMs = task->stats.totalRunTimeMs / task->stats.loopCount;
        }
        
        // Warn if loop took too long (> 1 second)
        if (loopTime > 1000) {
            Serial.printf("[Task:%s] WARNING: Loop took %lums\n", task->name, loopTime);
        }
    }
    
    // Call cleanup
    task->cleanup();
    
    // Mark as stopped
    task->currentState = TaskState::STOPPED;
    Serial.printf("[Task:%s] Task function exiting\n", task->name);
    
    // Delete task
    task->taskHandle = NULL;
    vTaskDelete(NULL);
}

void TaskBase::updateStatistics() {
    stats.state = currentState;
    stats.priority = taskPriority;
    
    // Update stack high water mark
    if (taskHandle != NULL) {
        UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(taskHandle);
        stats.stackHighWaterMark = highWaterMark * sizeof(StackType_t);
    }
}

#endif // USE_RTOS
