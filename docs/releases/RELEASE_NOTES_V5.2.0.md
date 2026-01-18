# ESP32 WiFi Utility - Release Notes v5.2.0

**Release Date:** January 16, 2026  
**Release Type:** Feature Release  
**Focus Areas:** TFT Display UI, Code Architecture, User Experience

---

## 🎨 Overview

Version 5.2.0 represents a **complete redesign of the TFT display user interface** with professional branding, intuitive color-coded status indicators, and a modular architecture that significantly improves code maintainability while enhancing the overall user experience.

This release transforms the TFT display from functional status screens into a polished, professional interface that provides instant visual feedback and creates a branded experience from device boot.

---

## ✨ What's New

### 1. Branded Welcome Screen

**Professional First Impression**

The device now boots with a professional branded welcome screen featuring the ESP32 logo and "WiFi Utility" title, replacing the generic "WiFi Tool Starting..." text message.

**Features:**
- Custom ESP32 logo (60×60 pixels) in purple (#8855FF)
- Professional typography with "ESP32" title (size 2 font)
- "WiFi Utility" subtitle (size 1 font)
- 2-second display duration for optimal brand recognition
- Sleek black background for high contrast

**Implementation:**
```cpp
void displayWelcomeScreen() {
    tft.fillScreen(ST77XX_BLACK);
    tft.drawBitmap(logoX, logoY, image_esp32_logo_bits, 60, 60, 0x82F4);
    // Title and subtitle rendering
}
```

**Design Reference:** [Lopaka UI Editor Project](https://lopaka.app/editor/23371/50884)

---

### 2. Color-Coded Status System

**Intuitive Visual Feedback at a Glance**

A comprehensive color-coding system provides instant status recognition without requiring users to read text. Colors are chosen based on psychological associations and industry standards.

**Color Palette:**

| Color | Hex Code | RGB565 | Status | Use Case | Psychology |
|-------|----------|--------|--------|----------|------------|
| 🔵 Blue | #5555EE | 0x55E | Connecting/Idle | Station mode operations | Calm, processing, waiting |
| 🟢 Green | #00FF00 | 0x07E0 | Connected/Active | Success states | Success, go, ready |
| 🔴 Red | #FF2266 | 0xF206 | Disabled | WiFi stopped | Stop, warning, offline |
| 🟡 Yellow | #FFFF00 | 0xFFE0 | Idle (standby) | Mode idle command | Standby, neutral |
| 🟣 Purple | #8855FF | 0x82F4 | Branding | Welcome screen | Premium, technology |

**Status Mapping:**

| WiFi State | Icon Color | Text | Display Mode |
|------------|------------|------|--------------|
| Device Boot | Purple | "ESP32 WiFi Utility" | Welcome |
| AP Initializing | Green | "Initializing Access Point" | Connecting |
| AP Active | Green | Full AP info + QR code | AP Mode |
| Station Connecting | Blue | "Station Mode / Connecting..." | Connecting |
| Station Connected | Green → Station | "Connected!" (1s) → Full info | Station |
| Station Idle (text) | Blue | "Station Mode / Idle" | Idle |
| Mode Idle (symbol) | Yellow | (no text, centered) | Idle Yellow |
| WiFi Disabled | Red | "Disabled!" | Disabled |

---

### 3. Complete Screen Coverage

**9 Distinct Display Modes**

Every WiFi state now has a dedicated, professionally designed screen:

#### **Welcome Screen**
- Shown once during device boot
- Purple ESP32 logo with branding
- Sets professional tone for user experience

#### **Connecting Screen (Blue)**
- WiFi icon in blue (50×50 pixels)
- "Station Mode" header
- "Connecting..." status text
- Vertically centered layout

#### **Connected Confirmation (Green)**
- Green WiFi icon
- "Connected!" message
- 1-second display before transitioning to full info
- Provides immediate positive feedback

#### **Station Mode Info**
- Comprehensive network details
- SSID, IP address, signal strength
- Color-coded RSSI quality bar
- Real-time signal monitoring

#### **Station Idle with Text (Blue)**
- Blue WiFi icon
- "Station Mode" header
- "Idle" status text
- Indicates station mode active but not connected

#### **Mode Idle - Yellow Symbol (No Text)**
- Yellow WiFi icon only
- No text labels
- Vertically centered at y=42
- Used for `mode idle` command
- Minimal, clean design

#### **Initializing Access Point (Green)**
- Green WiFi icon
- "Initializing Access Point" text
- Brief display during AP startup

#### **Access Point Mode**
- Full AP information display
- Network SSID and password
- IP address
- QR code for instant connection
- Client count
- Battery status
- Current time

#### **Disabled Screen (Red)**
- Red WiFi icon
- "Disabled!" message
- Clear indication WiFi is off
- Shown when `mode off` executed

---

### 4. Modular Architecture

**55% Code Reduction Through Smart Design**

The centerpiece of this release is the introduction of a single, flexible `displayWiFiStatusScreen()` function that replaces 6+ duplicated display functions.

**Before (v5.1.0):**
```cpp
// Each screen had its own implementation (~30 lines each)
void displayStationConnectingScreen() {
    tft.fillScreen(ST77XX_BLACK);
    int iconX = (TFT_WIDTH - 50) / 2;
    int iconY = 22;
    tft.drawBitmap(iconX, iconY, image_wifi_1_bits, 50, 50, 0x55E);
    tft.setTextColor(0x73AE);
    // ... 20+ more lines of duplicated code
}
// Similar functions for disabled, idle, etc. (150+ lines total)
```

**After (v5.2.0):**
```cpp
// Single modular function with parameters
void displayWiFiStatusScreen(uint16_t iconColor, uint16_t textColor,
                              const char* line1, const char* line2) {
    tft.fillScreen(ST77XX_BLACK);
    
    // Smart positioning based on text presence
    int iconY = (line1 == nullptr) ? 42 :           // Centered, no text
                (line2 == nullptr) ? 26 :           // One line of text
                22;                                 // Two lines of text
    
    // Draw icon and text
    tft.drawBitmap(iconX, iconY, image_wifi_1_bits, 50, 50, iconColor);
    // Text rendering with automatic centering
}

// Clean, declarative usage
void displayStationConnectingScreen() {
    displayWiFiStatusScreen(0x55E, 0x73AE, "Station Mode", "Connecting...");
}

void displayWiFiDisabledScreen() {
    displayWiFiStatusScreen(0xF206, 0xEF7D, "Disabled!", nullptr);
}
```

**Benefits:**
- **Code Reduction:** 180 lines → 80 lines (55% reduction)
- **Consistency:** All screens guaranteed identical layout behavior
- **Maintainability:** Single function to update for layout changes
- **Flexibility:** Easy to add new status screens with 1 line of code
- **Readability:** Declarative, self-documenting code

**Automatic Vertical Centering:**
- No text: Icon centered at y=42
- One line: Icon at y=26, text below
- Two lines: Icon at y=22, text below

---

### 5. Queue-Based Messaging System

**Non-Blocking FreeRTOS Integration**

All TFT updates now use the existing FreeRTOS queue infrastructure for truly non-blocking, asynchronous display operations.

**New Queue Functions:**
```cpp
bool sendTFTIdle();              // Blue station idle with text
bool sendTFTIdleYellow();        // Yellow idle symbol (no text)
bool sendTFTDisabled();          // Red disabled screen
bool sendTFTConnecting();        // Blue connecting screen
```

**Display Task Integration:**
```cpp
void tftDisplayTask(void *parameter) {
    TFTMessage msg;
    while (true) {
        if (xQueueReceive(tftQueue, &msg, portMAX_DELAY) == pdTRUE) {
            switch (msg.mode) {
                case TFT_MODE_IDLE:
                    displayStationIdleScreen();
                    break;
                case TFT_MODE_IDLE_YELLOW:
                    displayWiFiStatusScreen(0xFFE0, 0x73AE, nullptr, nullptr);
                    break;
                case TFT_MODE_DISABLED:
                    displayWiFiDisabledScreen();
                    break;
                // ... other modes
            }
        }
    }
}
```

**Benefits:**
- Zero blocking in main loop or WiFi operations
- Clean separation between business logic and UI
- Proper task synchronization
- Predictable, reliable display updates

---

### 6. Architecture Cleanup

**Improved Separation of Concerns**

The codebase architecture has been significantly improved with proper module boundaries and responsibilities.

**Before:**
```cpp
// command_interface.cpp - Poor separation
#include "tft_display.h"  // Direct TFT dependency

void handleModeOff() {
    stopWiFi();
    displayWiFiDisabledScreen();  // Direct UI call from command layer
}
```

**After:**
```cpp
// command_interface.cpp - Clean architecture
// No TFT includes

void handleModeOff() {
    requestStopWiFi();  // WiFi task handles coordination
}

// wifi_task.cpp - WiFi task orchestrates both WiFi and UI
case WIFI_CMD_STOP:
    sendTFTDisabled();  // TFT update via queue
    stopWiFi();         // WiFi operations
    break;
```

**Improvements:**
- ✅ Command interface has no TFT dependencies
- ✅ WiFi task orchestrates related WiFi and TFT operations
- ✅ Single responsibility principle maintained
- ✅ Easier testing and maintenance
- ✅ Clear module boundaries

---

### 7. Battery Monitor Conditional Compilation

**Platform-Specific Features**

Battery monitoring is now properly isolated to TFT Feather boards with dedicated macro support.

**New Macro:** `BATTERY_MONITOR_SUPPORT`
- Replaces overloaded use of `USE_NEOPIXEL`
- Semantic naming clearly indicates feature
- Only defined for boards with battery monitor hardware

**Conditional Compilation:**
```cpp
// battery_monitor.h
#ifdef BATTERY_MONITOR_SUPPORT
    bool initializeBatteryMonitor(...);
    float readBatteryVoltage();
    // ... battery monitor API
#endif // BATTERY_MONITOR_SUPPORT
```

**Build Configuration:**
- ✅ `adafruit_feather_esp32s3_tft` - BATTERY_MONITOR_SUPPORT=1
- ✅ `adafruit_feather_esp32s3_reversetft` - BATTERY_MONITOR_SUPPORT=1
- ❌ `esp32dev` - No macro (no battery hardware)

**Benefits:**
- ESP32dev builds without battery monitor libraries
- Clearer code intent with semantic naming
- Reduced flash usage for non-TFT boards
- Proper hardware feature abstraction

---

## 📊 Technical Details

### Memory Usage

**Flash Memory:**
- Total: 1,441,792 bytes
- Used: 1,226,785 bytes (85.1%)
- TFT Display Code: ~8KB
- WiFi Icon Bitmap: 313 bytes (50×50)
- ESP32 Logo Bitmap: 450 bytes (60×60)
- **Total Bitmaps:** 763 bytes

**RAM Usage:**
- Total: 327,680 bytes
- Used: 53,592 bytes (16.4%)
- TFT Task Stack: 4,096 bytes
- TFT Queue: 360 bytes (5 messages × 72 bytes)
- Display Buffers: Minimal (no frame buffer)

**Code Efficiency:**
- Before: ~180 lines (duplicated functions)
- After: ~80 lines (modular architecture)
- **Reduction:** 55% fewer lines of code
- Better cache utilization
- Reduced flash footprint

### Display Modes

**8 Distinct Modes:**
1. `TFT_MODE_OFF` - Display cleared
2. `TFT_MODE_AP` - Access Point with QR code
3. `TFT_MODE_STATION` - Connected station details
4. `TFT_MODE_CONNECTING` - Connecting animation
5. `TFT_MODE_IDLE` - Blue station idle (with text)
6. `TFT_MODE_IDLE_YELLOW` - Yellow idle (no text)
7. `TFT_MODE_DISABLED` - Red disabled indicator
8. `TFT_MODE_STATUS` - General status display

### FreeRTOS Integration

**Display Task:**
- **Task Name:** `TFT_Display`
- **Stack Size:** 4096 bytes
- **Priority:** 1 (low priority)
- **Core Affinity:** Core 1 (App Core)
- **Queue Size:** 5 messages
- **Message Size:** 72 bytes

**Task Behavior:**
- Blocks on queue when idle (zero CPU usage)
- Wakes only when messages arrive
- Independent rendering on Core 1
- No main loop overhead

---

## 🎯 User Experience

### Boot Sequence

**New Flow:**
1. Power on → Welcome Screen (2 seconds)
2. Check saved AP config → Green "Initializing Access Point"
3. OR check saved Station config → Blue "Connecting..."
4. OR no config → Yellow idle symbol

**Visual Feedback:**
- Every state change has immediate visual confirmation
- Color coding provides instant status recognition
- Professional branding establishes quality perception
- Consistent layouts reduce cognitive load

### State Transitions

**AP Mode Flow:**
```
Welcome → "Initializing Access Point" (green) → Full AP display with QR
```

**Station Mode Flow:**
```
Welcome → "Connecting..." (blue) → "Connected!" (green, 1s) → Full station info
```

**Idle Mode Flow:**
```
Welcome → Yellow WiFi symbol (no text, centered)
```

**Disabled Flow:**
```
Any state → "Disabled!" (red, centered)
```

---

## 📚 Documentation

### New Documentation

**TFT_UI_REDESIGN.md** (762 lines)
- Complete UI design guide
- Color psychology explanations
- All 9 screen modes documented with examples
- Modular architecture deep-dive
- Technical implementation details
- Memory usage metrics
- Before/after code comparisons
- User experience flows
- Future enhancement suggestions

### Updated Documentation

- **README.md** - Added v5.2.0 section with feature highlights
- **DOCUMENTATION_INDEX.md** - Updated with TFT UI redesign links
- **docs/README.md** - Added TFT UI Redesign as first core feature

### UI Design Assets

- **Lopaka Editor:** [Project Link](https://lopaka.app/editor/23371/50884)
- **Screenshots:** 5 professional UI screenshots in `docs/images/`
  - `welcome.png` - Branded startup screen
  - `connecting.png` - Blue connecting state
  - `initializing_ap.png` - Green AP initialization
  - `disabled.png` - Red disabled state
  - `ap_mode_qr.png` - Full AP mode with QR

---

## 🔧 Breaking Changes

**None** - This release is fully backward compatible.

All changes are internal improvements to the TFT display system. No API changes, no configuration changes, no behavioral changes to WiFi functionality.

---

## 🐛 Bug Fixes

### Fixed Issues

1. **Static Function Visibility**
   - `displayStationIdleScreen()` was declared static but called externally
   - Removed static declaration to allow external access
   - Enables proper function reuse across modules

2. **Architecture Issues**
   - Command interface had unnecessary TFT dependencies
   - Mode off command didn't show disabled screen
   - Fixed by moving TFT coordination to WiFi task layer

---

## ✅ Testing

### Build Verification

**All environments build successfully:**
- ✅ `esp32dev` - No TFT display (baseline)
- ✅ `adafruit_feather_esp32s3_tft` - Normal TFT
- ✅ `adafruit_feather_esp32s3_reversetft` - Reverse TFT
- ✅ `test` - ESP32dev test environment
- ✅ `test_feather` - Normal TFT test environment
- ✅ `test_feather_reversetft` - Reverse TFT test environment

**Memory Metrics:**
- Flash: 82.6% - 85.1% (within acceptable limits)
- RAM: 16.3% - 16.4% (stable)

### Hardware Testing

**Tested on Adafruit Feather ESP32-S3 Reverse TFT:**
- ✅ Welcome screen displays on boot
- ✅ All color-coded screens render correctly
- ✅ Mode switching updates display properly
- ✅ Station connecting → connected flow works
- ✅ AP mode shows full information and QR code
- ✅ Disabled screen appears on `mode off`
- ✅ Idle modes (blue with text, yellow without) function
- ✅ No visual glitches or timing issues

---

## 📦 Upgrade Guide

### For Existing Users

**No action required.** Simply pull and flash the new firmware:

```bash
git pull origin main
pio run -e adafruit_feather_esp32s3_reversetft --target upload
```

**What You'll Notice:**
1. New branded welcome screen on boot
2. Improved color-coded status indicators
3. Better visual feedback for all WiFi states
4. Professional, polished UI appearance

### For Developers

**If you've customized TFT display code:**

1. **Review `displayWiFiStatusScreen()` function**
   - Consolidates common display logic
   - May replace your custom implementations

2. **Update custom screens to use modular function:**
   ```cpp
   // Instead of duplicating layout code:
   displayWiFiStatusScreen(iconColor, textColor, "Line 1", "Line 2");
   ```

3. **Use new queue functions for TFT updates:**
   ```cpp
   sendTFTIdle();        // Blue idle with text
   sendTFTIdleYellow();  // Yellow idle, no text
   sendTFTDisabled();    // Red disabled
   ```

---

## 🔮 Future Enhancements

### Potential Improvements

**Animation Support:**
- Spinning/pulsing WiFi icon during connection
- Smooth transitions between screens
- Progress indicators for long operations

**Additional Icons:**
- Warning icon for weak signal
- Lock icon for secured networks
- Cloud icon for internet connectivity
- Signal strength visual indicators

**Theme System:**
- Light/dark mode toggle
- User-configurable color schemes
- Custom color definitions

**Touch Support:**
- Tap to cycle information screens
- Swipe gestures for navigation
- Interactive UI elements

**Enhanced Battery Display:**
- Low battery warning screen
- Charging indicator animation
- Power saving mode indication

---

## 📝 Commits in This Release

1. **feat(tft): Refactor WiFi status screens with modular architecture** (64d381c)
   - Created displayWiFiStatusScreen() modular function
   - Refactored connecting, disabled, and idle screens
   - Added blue station idle screen
   - Fixed architecture issues

2. **feat(tft): Add ESP32 WiFi Utility welcome screen** (7a42438)
   - Added branded welcome screen with purple logo
   - Created ESP32 logo bitmap (60×60)
   - Updated startup sequence with 2-second delay

3. **docs(readme): Add v5.2.0 TFT display UI redesign documentation** (b41eaa7)
   - Added v5.2.0 section to main README
   - Documented color-coded system and all screens
   - Included memory metrics and architecture details

4. **docs: Add comprehensive TFT UI redesign documentation** (ac25aa0)
   - Created TFT_UI_REDESIGN.md (762 lines)
   - Updated documentation index and guides
   - Added 5 UI screenshot images
   - Referenced Lopaka design tool

5. **build: Add battery monitor dependencies to test environments** (18be16a)
   - Add Adafruit MAX1704X library to test environments
   - Add Adafruit LC709203F library to test environments
   - Ensures test environments match production dependencies

6. **refactor: Add BATTERY_MONITOR_SUPPORT macro for TFT boards** (5e3fd4b)
   - Created dedicated BATTERY_MONITOR_SUPPORT macro
   - Add conditional compilation to battery monitor files
   - Battery monitor only compiles for TFT Feather boards
   - ESP32dev builds without battery dependencies

---

## 🙏 Acknowledgments

- **Lopaka UI Editor** - For excellent bitmap design and preview tools
- **ESP32 Community** - For FreeRTOS best practices and examples
- **Adafruit** - For excellent TFT display libraries and hardware

---

## 📞 Support

- **Issues:** [GitHub Issues](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/issues)
- **Pull Requests:** [GitHub PR #31](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/pull/31)
- **Documentation:** [Complete Documentation Index](DOCUMENTATION_INDEX.md)
- **TFT UI Guide:** [TFT Display UI Redesign](TFT_UI_REDESIGN.md)

---

**Version:** 5.2.0  
**Release Date:** January 16, 2026  
**Author:** ESP32 WiFi Utility Team  
**License:** MIT
