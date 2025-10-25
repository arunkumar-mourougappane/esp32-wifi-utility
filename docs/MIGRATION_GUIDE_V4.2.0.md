# Migration Guide: v4.1.0 → v4.2.0

## Quick Start

Version 4.2.0 is **100% backward compatible**. Existing code will continue to work without any changes.

However, to take advantage of the new features, follow this guide.

---

## New Features to Adopt

### 1. Replace Error Returns with Result<T>

**Old Pattern:**
```cpp
bool connectWiFi(const char* ssid) {
    if (!ssid) {
        Serial.println("ERROR: Invalid SSID");
        return false;
    }
    return WiFi.begin(ssid);
}
```

**New Pattern:**
```cpp
#include "error_handling.h"

Result<void> connectWiFi(const char* ssid) {
    if (!ssid) {
        return {ErrorCode::WIFI_INVALID_SSID, "SSID cannot be null"};
    }
    
    if (!WiFi.begin(ssid)) {
        return {ErrorCode::WIFI_CONNECT_FAILED, "Connection failed"};
    }
    
    return {};  // Success
}

// Usage:
auto result = connectWiFi("MyNetwork");
if (!result) {
    LOG_ERROR(TAG_WIFI, "%s", result.getMessage());
}
```

---

### 2. Replace Serial.print with Logging

**Old Pattern:**
```cpp
Serial.println("[WiFi] Connecting...");
Serial.print("[WiFi] ERROR: ");
Serial.println(errorMsg);
```

**New Pattern:**
```cpp
#include "logging.h"

LOG_INFO(TAG_WIFI, "Connecting to %s", ssid);
LOG_ERROR(TAG_WIFI, "Connection failed: %s", errorMsg);
LOG_DEBUG(TAG_WIFI, "RSSI: %d dBm", rssi);
```

---

### 3. Add Performance Monitoring

**Old Pattern:**
```cpp
void scanNetworks() {
    unsigned long start = millis();
    int count = WiFi.scanNetworks();
    Serial.printf("Scan took %lu ms\n", millis() - start);
}
```

**New Pattern:**
```cpp
#include "performance_monitor.h"

void scanNetworks() {
    ScopedTimer timer(g_wifiScanMonitor);
    int count = WiFi.scanNetworks();
}

// Later, print all statistics:
printAllPerformanceStats();
```

---

### 4. Update Queue Structure Usage

**If you're using CommandRequest:**

**Old Pattern:**
```cpp
CommandRequest req;
req.commandString = myString;
req.argument = arg1;
req.secondArgument = arg2;
```

**New Pattern:**
```cpp
CommandRequest req;
req.setCommand(myString.c_str());
req.setArgument(arg1.c_str());
req.setSecondArgument(arg2.c_str());
```

**If you're creating new queue structures:**
- Use `char[]` arrays, not `String`
- Add helper methods like `setXxx(const char*)`
- Make it a POD type (no constructors with side effects)

---

## Configuration Changes

### Use Named Constants

**Old Pattern:**
```cpp
delay(300);  // Magic number!
if (strlen(ssid) > 32) { ... }
```

**New Pattern:**
```cpp
#include "config.h"

delay(SystemConstants::LED_STARTUP_ANIMATION_DELAY_MS);
if (strlen(ssid) > SystemConstants::MAX_SSID_LENGTH) { ... }
```

---

## Testing Your Migration

### 1. Compile Test
```bash
pio run -e esp32dev
```

### 2. Enable Debug Logging
```cpp
void setup() {
    setLogLevel(LogLevel::DEBUG);
    setLogTimestamps(true);
    LOG_INFO(TAG_MAIN, "System starting v4.2.0");
}
```

### 3. Check Performance
```cpp
void loop() {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 60000) {  // Every minute
        printAllPerformanceStats();
        logMemoryStats(TAG_SYSTEM);
        lastPrint = millis();
    }
}
```

---

## Common Migration Patterns

### Pattern 1: Function Returns

```cpp
// Before:
bool myFunction() {
    if (error) return false;
    return true;
}

// After:
Result<void> myFunction() {
    if (error) return {ErrorCode::SPECIFIC_ERROR};
    return {};
}
```

### Pattern 2: Functions with Return Values

```cpp
// Before:
int getValue(bool& success) {
    if (error) {
        success = false;
        return 0;
    }
    success = true;
    return 42;
}

// After:
Result<int> getValue() {
    if (error) return {ErrorCode::SPECIFIC_ERROR};
    return 42;
}
```

### Pattern 3: Error Propagation

```cpp
// Before:
bool doMultipleThings() {
    if (!step1()) return false;
    if (!step2()) return false;
    if (!step3()) return false;
    return true;
}

// After:
Result<void> doMultipleThings() {
    RETURN_IF_ERROR(step1());
    RETURN_IF_ERROR(step2());
    RETURN_IF_ERROR(step3());
    return {};
}
```

---

## Gradual Migration Strategy

You don't have to migrate everything at once! Here's a recommended approach:

### Phase 1: Add Logging (Low Risk)
1. Replace `Serial.println()` with `LOG_XXX()` macros
2. No functional changes, just better output
3. Test: Everything should work the same

### Phase 2: Add Performance Monitoring (Low Risk)
1. Wrap critical functions with `ScopedTimer`
2. No functional changes, just measurement
3. Test: Check stats periodically

### Phase 3: Migrate Error Handling (Medium Risk)
1. Start with leaf functions (lowest level)
2. Work your way up the call stack
3. Test thoroughly after each module

### Phase 4: Fix Queue Structures (High Risk)
1. Only if you've added custom queue types
2. Test immediately after changes
3. Verify no memory corruption

---

## Breaking Changes

**None!** Version 4.2.0 is 100% backward compatible.

All new features are additive. Your existing code will continue to work exactly as before.

---

## Need Help?

### Documentation
- [CODE_IMPROVEMENTS_V4.2.0.md](technical/CODE_IMPROVEMENTS_V4.2.0.md) - Full details
- [IMPROVEMENTS_SUMMARY.md](IMPROVEMENTS_SUMMARY.md) - Quick overview

### Code Examples
- See `src/logging.cpp` for logging patterns
- Check `include/error_handling.h` for Result<T> examples
- Look at `src/performance_monitor.cpp` for monitoring

### Questions?
Open an issue on GitHub or check existing documentation in the `docs/` folder.

---

**Version:** 4.2.0  
**Compatibility:** ✅ 100% Backward Compatible  
**Migration Difficulty:** 🟢 Easy (Optional)
