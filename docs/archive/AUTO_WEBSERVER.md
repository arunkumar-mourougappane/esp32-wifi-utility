# Automatic Web Server Management

## 🤖 Overview

The ESP32 WiFi Utility now features **automatic web server lifecycle management** on the Feather ESP32-S3 TFT. The web server automatically starts when entering network modes and stops when exiting, providing seamless access to the web interface without manual commands.

## ✨ Automatic Behavior

### 🚀 Web Server Auto-Start

The web server **automatically starts** in the following scenarios:

#### 1. **Entering AP Mode**
```
Command: mode ap
Result:
  ✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 123456789
  IP Address: 192.168.4.1
  🌐 Starting web server...
  ✅ Web server is ready at: http://192.168.4.1
```

#### 2. **Entering AP Mode with Custom SSID**
```
Command: mode ap MyNetwork MyPassword123
Result:
  ✓ Custom Access Point mode activated
  SSID: MyNetwork
  Password: MyPassword123
  IP Address: 192.168.4.1
  🌐 Starting web server...
  ✅ Web server is ready at: http://192.168.4.1
```

#### 3. **Connecting to WiFi Network**
```
Command: connect MyHomeWiFi password123
Result:
  🔗 Connecting to 'MyHomeWiFi'...
  ✓ Connected to 'MyHomeWiFi'
  IP Address: 192.168.1.100
  Gateway: 192.168.1.1
  DNS: 192.168.1.1
  🌐 Starting web server...
  ✅ Web server is ready at: http://192.168.1.100
```

#### 4. **Entering Station Mode (Ready for Connection)**
```
Command: mode station
Result:
  ✓ Station mode activated - Ready to scan for networks
  Use 'scan on' to start scanning
  🌐 Starting web server...
  ⚠️  Web server will start after WiFi connection
```
*Note: In station mode without connection, web server waits until `connect` succeeds*

### 🛑 Web Server Auto-Stop

The web server **automatically stops** in the following scenarios:

#### 1. **Disconnecting from WiFi**
```
Command: disconnect
Result:
  🌐 Stopping web server...
  ✅ Web server stopped
  ✓ Disconnected from 'MyHomeWiFi'
```

#### 2. **Entering Idle Mode**
```
Command: mode idle
Result:
  🌐 Stopping web server...
  ✅ Web server stopped
  🟡 Device set to IDLE mode - Ready for commands
```

#### 3. **Disabling WiFi**
```
Command: wifi off
Result:
  🌐 Stopping web server...
  ✅ Web server stopped
  ✓ WiFi disabled
```

#### 4. **Switching Between Modes**
When switching from one mode to another (e.g., AP → Station), the web server stops and restarts automatically:
```
Current: AP Mode (web server at 192.168.4.1)
Command: mode station
Result:
  🌐 Stopping web server...      ← Stops in AP mode
  ✅ Web server stopped
  ✓ Station mode activated
  🌐 Starting web server...      ← Starts in Station mode
  ⚠️  Web server will start after WiFi connection
```

## 🎯 Key Benefits

### User Experience
- ✅ **No Manual Commands**: Web server starts/stops automatically
- ✅ **Seamless Access**: Always available when WiFi is active
- ✅ **Clean Transitions**: Proper cleanup when changing modes
- ✅ **Clear Feedback**: Serial console shows web server status

### System Reliability
- ✅ **Resource Management**: Server stops when not needed
- ✅ **Memory Cleanup**: Proper deallocation on mode changes
- ✅ **No Conflicts**: Server resets between mode transitions
- ✅ **Graceful Shutdown**: Proper disconnection handling

## 📋 Mode-Specific Behavior

### Access Point Mode (AP)

| Trigger | Web Server State | Access URL |
|---------|-----------------|------------|
| `mode ap` | ✅ Auto-starts immediately | `http://192.168.4.1` |
| `mode ap SSID PASS` | ✅ Auto-starts immediately | `http://192.168.4.1` |
| Switch to other mode | 🛑 Auto-stops | N/A |
| `wifi off` | 🛑 Auto-stops | N/A |
| `mode idle` | 🛑 Auto-stops | N/A |

**Access Point Example:**
```bash
> mode ap
✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 123456789
  IP Address: 192.168.4.1
🌐 Starting web server...
✅ Web server is ready at: http://192.168.4.1

# Connect phone to ESP32-WiFiScanner
# Open browser to http://192.168.4.1
# Access all web features immediately
```

### Station Mode (Connected)

| Trigger | Web Server State | Access URL |
|---------|-----------------|------------|
| `mode station` | ⏳ Waits for connection | N/A |
| `connect SSID PASS` | ✅ Auto-starts after success | `http://<DHCP-IP>` |
| `disconnect` | 🛑 Auto-stops | N/A |
| Switch to other mode | 🛑 Auto-stops | N/A |

**Station Mode Example:**
```bash
> mode station
✓ Station mode activated - Ready to scan for networks
🌐 Starting web server...
⚠️  Web server will start after WiFi connection

> connect MyHomeWiFi password123
🔗 Connecting to 'MyHomeWiFi'...
✓ Connected to 'MyHomeWiFi'
  IP Address: 192.168.1.100
🌐 Starting web server...
✅ Web server is ready at: http://192.168.1.100

# Open browser to http://192.168.1.100
# Access web interface on local network
```

### Idle Mode

| Trigger | Web Server State | Access URL |
|---------|-----------------|------------|
| `mode idle` | 🛑 Stopped | N/A |
| Boot/Reset | 🛑 Stopped (default) | N/A |
| `wifi off` | 🛑 Stopped | N/A |

## 🔧 Technical Implementation

### Code Architecture

**Location:** `src/wifi_manager.cpp`

**Key Functions Modified:**

1. **`startStationMode()`**
```cpp
#ifdef USE_NEOPIXEL
  // Automatically start web server in station mode
  Serial.println("🌐 Starting web server...");
  if (startWebServer()) {
    Serial.println("✅ Web server is ready at: " + getWebServerURL());
  } else {
    Serial.println("⚠️  Web server will start after WiFi connection");
  }
#endif
```

2. **`startAccessPoint()`** (both overloads)
```cpp
#ifdef USE_NEOPIXEL
  // Automatically start web server in AP mode
  Serial.println("🌐 Starting web server...");
  if (startWebServer()) {
    Serial.println("✅ Web server is ready at: " + getWebServerURL());
  }
#endif
```

3. **`connectToNetwork()`**
```cpp
#ifdef USE_NEOPIXEL
  // Automatically start web server after successful connection
  if (!isWebServerRunning()) {
    Serial.println("🌐 Starting web server...");
    if (startWebServer()) {
      Serial.println("✅ Web server is ready at: " + getWebServerURL());
    }
  }
#endif
```

4. **`stopWiFi()`**
```cpp
#ifdef USE_NEOPIXEL
  // Stop web server before disabling WiFi
  if (isWebServerRunning()) {
    Serial.println("🌐 Stopping web server...");
    stopWebServer();
  }
#endif
```

5. **`setIdleMode()`**
```cpp
#ifdef USE_NEOPIXEL
  // Stop web server before setting idle mode
  if (isWebServerRunning()) {
    Serial.println("🌐 Stopping web server...");
    stopWebServer();
  }
#endif
```

6. **`disconnectFromNetwork()`**
```cpp
#ifdef USE_NEOPIXEL
  // Stop web server before disconnecting
  if (isWebServerRunning()) {
    Serial.println("🌐 Stopping web server...");
    stopWebServer();
  }
#endif
```

### Conditional Compilation

All automatic web server management is wrapped in `#ifdef USE_NEOPIXEL`:
- **Feather ESP32-S3 TFT**: Full automatic web server support
- **ESP32dev**: No web server code compiled (saves memory)

### Header Inclusion

**File:** `src/wifi_manager.cpp`
```cpp
#ifdef USE_NEOPIXEL
#include "web_server.h"
#endif
```

Only includes web server header when building for Feather board.

## 🚀 Usage Workflows

### Workflow 1: Quick AP Testing
```bash
# Boot device
> mode ap

# Automatic sequence:
✓ Access Point mode activated
🌐 Starting web server...
✅ Web server is ready at: http://192.168.4.1

# Connect device to ESP32-WiFiScanner
# Open http://192.168.4.1 in browser
# Immediately access all features
```

### Workflow 2: Home Network Integration
```bash
# Boot device
> mode station
> scan on
> connect MyHomeWiFi password123

# Automatic sequence:
🔗 Connecting to 'MyHomeWiFi'...
✓ Connected to 'MyHomeWiFi'
  IP Address: 192.168.1.100
🌐 Starting web server...
✅ Web server is ready at: http://192.168.1.100

# Access from any device on local network
# Open http://192.168.1.100 in browser
```

### Workflow 3: Mode Switching
```bash
# Currently in AP mode with web server running
Current: http://192.168.4.1 ✅ Active

> mode station
# Automatic cleanup and restart:
🌐 Stopping web server...
✅ Web server stopped
✓ Station mode activated
🌐 Starting web server...
⚠️  Web server will start after WiFi connection

> connect HomeWiFi pass123
# Automatic start after connection:
✓ Connected to 'HomeWiFi'
🌐 Starting web server...
✅ Web server is ready at: http://192.168.1.105
```

### Workflow 4: Power Saving
```bash
# Currently connected with web server running
Current: http://192.168.1.100 ✅ Active

> mode idle
# Automatic cleanup:
🌐 Stopping web server...
✅ Web server stopped
🟡 Device set to IDLE mode

# Web server stopped, WiFi off, minimal power consumption
```

## 📊 Memory Impact

### Build Statistics

**Before Automatic Management:**
```
Flash: 72.6%  (1,046,789 bytes)
RAM:   19.1%  (62,632 bytes)
```

**After Automatic Management:**
```
Flash: 72.7%  (1,047,529 bytes)
RAM:   19.1%  (62,632 bytes)
Impact: +740 bytes flash (+0.05%)
        No RAM increase
```

**Memory Breakdown:**
- Additional code: ~500 bytes
- Additional strings: ~240 bytes
- Conditional compilation ensures no impact on ESP32dev

## 🎯 User Benefits Summary

### Before (Manual Management)
```bash
> mode ap
✓ Access Point mode activated
> webserver start                    ← Manual command needed
✅ Web server started
```

### After (Automatic Management)
```bash
> mode ap
✓ Access Point mode activated
🌐 Starting web server...             ← Automatic
✅ Web server is ready at: http://192.168.4.1
```

**Benefits:**
- ✅ **3 fewer commands** for typical workflow
- ✅ **Immediate access** to web interface
- ✅ **No forgotten steps** - always available when needed
- ✅ **Cleaner transitions** between modes
- ✅ **Better UX** for beginners

## ⚠️ Important Notes

### Web Server Availability

1. **Station Mode Without Connection:**
   - Web server initialization attempted but waits for connection
   - Shows warning: "⚠️  Web server will start after WiFi connection"
   - Automatically starts when `connect` succeeds

2. **AP Mode:**
   - Web server starts immediately (always has IP)
   - Accessible at fixed IP: `192.168.4.1`

3. **Idle/Off Modes:**
   - Web server completely stopped
   - Resources freed for other operations

### Serial Feedback

All automatic operations provide clear serial feedback:
- 🌐 "Starting web server..." - Initiation message
- ✅ "Web server is ready at: [URL]" - Success message
- 🛑 "Stopping web server..." - Shutdown message
- ⚠️ "Web server will start after WiFi connection" - Waiting state

## 🔮 Future Enhancements

### Potential Additions

1. **Auto-Restart on IP Change:**
   - Restart web server if DHCP renews different IP
   - Update QR codes with new URL

2. **mDNS Support:**
   - Access via `http://esp32-wifi.local` instead of IP
   - Easier discovery on local networks

3. **Connection Recovery:**
   - Auto-restart web server after WiFi reconnection
   - Handle temporary network outages

4. **Status LED Integration:**
   - NeoPixel colors indicate web server state
   - Visual feedback for server availability

## ✅ Testing Checklist

- [x] Web server starts in AP mode
- [x] Web server starts after WiFi connection
- [x] Web server stops on disconnect
- [x] Web server stops on mode idle
- [x] Web server stops on wifi off
- [x] Web server restarts on mode switch
- [x] Serial messages display correctly
- [x] ESP32dev builds without web server code
- [x] No memory leaks on start/stop cycles
- [x] Custom AP SSID triggers auto-start
- [x] URLs displayed correctly for each mode

## 📚 Related Documentation

- **Web Server Features**: See `INTERACTIVE_IPERF_WEB.md`
- **Signal Strength Indicators**: See `SIGNAL_STRENGTH_INDICATORS.md`
- **Command Reference**: See main `README.md`
- **API Documentation**: See `include/web_server.h`

## 🎊 Conclusion

The automatic web server management feature provides a **seamless, user-friendly experience** that eliminates manual steps while maintaining full control and reliability. Users can focus on network testing and analysis without worrying about web server lifecycle management.

**Key Achievements:**
- ✅ Fully automatic start/stop behavior
- ✅ Clear serial feedback for all operations
- ✅ Proper resource management and cleanup
- ✅ Minimal memory impact (+740 bytes)
- ✅ No code bloat on ESP32dev platform
- ✅ Seamless mode transitions
- ✅ Enhanced user experience

---

**Version:** 2.1.0+auto-webserver  
**Date:** October 15, 2025  
**Flash Impact:** +740 bytes (+0.05%)  
**Feature:** Automatic web server lifecycle management  
**Platform:** Feather ESP32-S3 TFT only (conditional compilation)
