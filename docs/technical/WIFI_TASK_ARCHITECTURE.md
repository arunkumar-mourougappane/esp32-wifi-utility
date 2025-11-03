# WiFi Command Task Architecture

## Overview

The WiFi Command Task provides a FreeRTOS-based asynchronous architecture for managing WiFi mode switching (AP ↔ Station) without blocking the main loop or web server operations.

## Architecture

### Task Configuration
- **Core Affinity**: Core 1 (App Core)
- **Stack Size**: 8192 bytes (WiFi operations require more stack)
- **Priority**: 2 (Higher than TFT Display task priority 1)
- **Queue Size**: 10 commands
- **Blocking**: `portMAX_DELAY` on queue receive (efficient CPU usage)

### Design Pattern
```
Web Request → Queue Command → Task Processes → WiFi Switches Mode
     ↓              ↓                ↓                  ↓
  Non-blocking  Async Queue    Dedicated Task    Background Switch
```

## Benefits

### 1. **Non-Blocking Web Server**
- Web requests return immediately with confirmation
- No HTTP timeout issues during WiFi mode switches
- Better user experience

### 2. **Thread-Safe Mode Switching**
- All WiFi operations happen in dedicated task
- No race conditions or concurrent WiFi API calls
- Queue-based command serialization

### 3. **Better Error Handling**
- Commands queued independently
- Failed commands don't block others
- Clear logging of task operations

### 4. **Resource Efficiency**
- Task blocks when idle (zero CPU usage)
- Higher priority than UI updates
- Proper stack allocation for WiFi operations

## API Reference

### Initialization

```cpp
bool initWiFiTask()
```
Initialize the WiFi command task and queue. Called during `setup()`.

**Returns**: `true` if successful, `false` on error

### Mode Switching Commands

#### Switch to Access Point Mode

```cpp
bool requestSwitchToAP()
```
Switch to AP mode using saved configuration (or defaults if none saved).

```cpp
bool requestSwitchToAP(const char* ssid, const char* password)
```
Switch to AP mode with specific credentials.

**Parameters**:
- `ssid`: Access Point SSID (max 63 chars)
- `password`: Access Point password (max 63 chars)

**Returns**: `true` if command queued, `false` if queue full or not initialized

#### Switch to Station Mode

```cpp
bool requestSwitchToStation()
```
Switch to Station mode using saved configuration.

```cpp
bool requestSwitchToStation(const char* ssid, const char* password)
```
Switch to Station mode with specific credentials.

**Parameters**:
- `ssid`: Network SSID to connect to (max 63 chars)
- `password`: Network password (max 63 chars)

**Returns**: `true` if command queued, `false` if queue full or not initialized

#### Other Commands

```cpp
bool requestStopWiFi()
```
Stop WiFi completely.

```cpp
bool requestIdleMode()
```
Set WiFi to idle mode (no AP, no Station).

## Command Types

```cpp
enum WiFiCommandType {
    WIFI_CMD_SWITCH_TO_AP,      // Switch to Access Point mode
    WIFI_CMD_SWITCH_TO_STATION, // Switch to Station mode
    WIFI_CMD_STOP,              // Stop WiFi
    WIFI_CMD_IDLE               // Set to idle mode
}
```

## Command Structure

```cpp
struct WiFiCommand {
    WiFiCommandType type;
    char param1[64];  // For SSID or other parameters
    char param2[64];  // For password or other parameters
}
```

## Usage Examples

### Web Server Mode Switch Handler

```cpp
void handleModeSwitch() {
    String mode = webServer->arg("mode");
    
    if (mode == "ap") {
        if (requestSwitchToAP()) {
            webServer->send(200, "text/plain", 
                "Access Point mode switch requested. Check status in a moment.");
        } else {
            webServer->send(500, "text/plain", 
                "Failed to queue AP mode switch request");
        }
    } else if (mode == "station") {
        if (requestSwitchToStation()) {
            webServer->send(200, "text/plain", 
                "Station mode switch requested. Check status in a moment.");
        } else {
            webServer->send(500, "text/plain", 
                "Failed to queue Station mode switch request");
        }
    }
}
```

### Command Interface

```cpp
// Switch to AP with specific credentials
if (requestSwitchToAP("MyAP", "password123")) {
    Serial.println("AP mode switch requested");
}

// Switch to Station using saved config
if (requestSwitchToStation()) {
    Serial.println("Station mode switch requested");
}
```

## Task Flow

### 1. **Task Initialization** (Core 1, Priority 2)
```
setup() → initWiFiTask() → Create Queue + Task
```

### 2. **Command Queueing** (Non-blocking)
```
Web/Serial Request → requestSwitchToX() → xQueueSend(0 ticks)
```

### 3. **Task Processing** (Blocking on queue)
```
wifiCommandTask() → xQueueReceive(FOREVER) → Process Command
```

### 4. **Mode Switch Execution**
```
Switch Command → Load Config → Call WiFi Manager → Update TFT
```

## Serial Output

Typical output during mode switch:

```
[Web] Requesting switch to Access Point mode
[Web] ✅ AP mode switch request queued
🎯 WiFi Command Task started on Core 1
[WiFi Task] Processing command: 0
[WiFi Task] Switching to Access Point mode
[WiFi Task] Started AP with saved config: MyESP32_AP
✅ AP Mode displayed via task
[WiFi Task] ✅ Command processed
```

## Performance Characteristics

### Queue Latency
- **Typical**: <1ms from request to queue
- **Processing**: Depends on WiFi operations (100-2000ms)
- **Total**: ~1-2 seconds for complete mode switch

### Memory Usage
- **Queue**: 640 bytes (10 × 64-byte commands)
- **Stack**: 8192 bytes (WiFi operations)
- **Code**: ~2KB additional flash

### Priority Management
```
Priority 2: WiFi Command Task (Mode switching)
Priority 1: TFT Display Task (UI updates)
Priority 0: Loop() (Background tasks)
```

## Error Handling

### Queue Full
If queue is full (10 commands pending), `requestSwitchToX()` returns `false` immediately.

### No Saved Config
Task will use default AP config or report error for Station mode.

### WiFi API Failures
Logged to Serial, but task continues processing next command.

## Integration Points

### Files Modified/Created

**Created**:
- `lib/WiFiManager/wifi_task.h` - Task API declarations
- `lib/WiFiManager/wifi_task.cpp` - Task implementation

**Modified**:
- `lib/WebServer/web_server.cpp` - Updated `handleModeSwitch()`
- `src/main.cpp` - Added `initWiFiTask()` call

### Dependencies

**Required**:
- FreeRTOS (built into ESP32)
- WiFiManager library (existing)
- APConfig library (existing)
- StationConfig library (existing)

**Optional**:
- TFT Display (for mode change visualization)

## Future Enhancements

### Potential Improvements
1. **Priority Queueing**: High-priority commands (e.g., emergency stop)
2. **Command Confirmation**: Callback when command completes
3. **Status Query**: Check if mode switch in progress
4. **Retry Logic**: Automatic retry on WiFi connection failure
5. **Graceful Degradation**: Fallback to AP mode if Station fails

### Extension Points
```cpp
// Add new command types
enum WiFiCommandType {
    WIFI_CMD_SWITCH_TO_AP,
    WIFI_CMD_SWITCH_TO_STATION,
    WIFI_CMD_RECONNECT,          // New: Reconnect current mode
    WIFI_CMD_SCAN_NETWORKS,      // New: Trigger network scan
    // ... more commands
}
```

## Troubleshooting

### Task Not Created
**Symptom**: "❌ Failed to create WiFi command task"
**Cause**: Insufficient memory or Core 1 overloaded
**Solution**: Reduce stack size or check memory usage

### Commands Not Processing
**Symptom**: Mode doesn't switch after request
**Cause**: Task not initialized or crashed
**Solution**: Check Serial output for task logs

### Queue Overflow
**Symptom**: `requestSwitchToX()` returns `false`
**Cause**: Too many pending commands
**Solution**: Increase queue size or rate-limit requests

## See Also

- [TFT Task Architecture](TFT_TASK_ARCHITECTURE.md) - Similar task-based UI updates
- [WiFi Manager API](lib/WiFiManager/wifi_manager.h) - Underlying WiFi functions
- [FreeRTOS Documentation](docs/technical/RTOS_ARCHITECTURE.md) - RTOS principles
