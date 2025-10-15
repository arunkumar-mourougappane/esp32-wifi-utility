# Reset Command Implementation

## Overview

Added a new `reset` command to the ESP32 WiFi Utility that allows users to restart the ESP32  
device gracefully from the command prompt.

## Command Syntax

- `reset` - Restart the ESP32 device
- `restart` - Alias for the reset command

## Functionality

The reset command performs a graceful shutdown of all active services before restarting the device:

### Shutdown Sequence:
1. **WiFi Services**: Stops Access Point or disconnects from WiFi network
2. **iPerf Services**: Cleanly shuts down any running performance tests
3. **Latency Analysis**: Stops latency and jitter testing
4. **Channel Monitoring**: Disables channel congestion monitoring
5. **LED Control**: Turns off all LEDs (both standard and NeoPixel)
6. **System Reset**: Performs hardware reset using `esp_restart()`

### Example Usage:
```
🟡 ESP32> reset

⚠️  SYSTEM RESET INITIATED
==========================================
🔄 Preparing for board reset...
🛑 Stopping active services...
   - Stopping Access Point
   - Stopping iPerf services  
   - Stopping latency analysis
   - Stopping channel monitoring
   - Turning off NeoPixel
✅ All services stopped
🔄 Restarting ESP32 in 2 seconds...
==========================================

[Device restarts and shows initialization sequence]
```

## Technical Implementation

### Files Modified:
- **`src/command_interface.cpp`**: Added reset command handling and implementation
- **`include/command_interface.h`**: Added function declaration

### Key Features:
- **Graceful Shutdown**: All services are properly stopped before reset
- **Status Feedback**: Clear messages show shutdown progress
- **Dual Board Support**: Works on both ESP32dev and Feather ESP32-S3 TFT
- **Help Integration**: Command appears in help documentation

### Safety Measures:
- 2-second delay before reset to ensure serial output is sent
- Serial buffer flush to prevent message loss
- Proper cleanup of all active connections and services

## Benefits

1. **Clean Restart**: Ensures all services stop gracefully
2. **Troubleshooting**: Easy way to restart device when issues occur
3. **User Friendly**: Simple command instead of physical button press
4. **Professional**: Follows embedded system best practices
5. **Safe**: No risk of corrupting active connections

## Hardware Compatibility

- ✅ **ESP32 Development Board**: Full support
- ✅ **Adafruit Feather ESP32-S3 TFT**: Full support with NeoPixel handling
- ✅ **All WiFi Modes**: Works in IDLE, Station, AP, and OFF modes
- ✅ **All Services**: Properly handles iPerf, latency testing, and channel analysis

The reset command provides a professional and safe way to restart the ESP32 device from the  
command interface, ensuring all services are properly shut down before the hardware reset occurs.