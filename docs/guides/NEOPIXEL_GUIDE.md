# ESP32 WiFi Utility - NeoPixel Status Display

This document describes the NeoPixel LED status display functionality available for the Adafruit Feather ESP32-S3 TFT board.

## Overview

The ESP32 WiFi Utility now includes conditional compilation support for NeoPixel LEDs specifically for the Adafruit Feather ESP32-S3 TFT board. The NeoPixel provides rich, colorful status indication for various WiFi operations and system states.

## Hardware Configuration

### NeoPixel Pin Configuration
- **Pin**: GPIO 33 (default for Feather ESP32-S3 TFT)
- **Type**: Single WS2812B NeoPixel LED
- **Brightness**: 50/255 (adjustable via `NEOPIXEL_BRIGHTNESS`)

### Conditional Compilation
The NeoPixel functionality is enabled only for the Feather board using the `USE_NEOPIXEL` flag:

```ini
[env:adafruit_feather_esp32s3_tft]
build_flags = 
    -DUSE_NEOPIXEL=1
lib_deps = 
    adafruit/Adafruit NeoPixel@^1.12.0
```

## Status Color Scheme

### 🌈 System States

| State | Color | Pattern | Description |
|-------|-------|---------|-------------|
| **Startup** | Purple → Green | Solid | Initialization sequence |
| **Idle Mode** | Blue | Smooth Pulse | System ready, WiFi idle |
| **Station Mode (Ready)** | Green | Solid | Station mode active, ready to scan |
| **Scanning** | Cyan | Fast Blink | WiFi network scanning in progress |
| **Access Point Mode** | Orange | Medium Blink | AP mode active |
| **Connecting** | Yellow | Alternating Blink | Attempting WiFi connection |
| **Connected** | Green | Solid (1 sec) | Successfully connected |
| **Connection Failed** | Red | Solid (2 sec) | Connection attempt failed |
| **Error/Off** | Red | Solid | Error state or WiFi disabled |

### 🎨 Interactive Feedback

| Command | Color | Duration | Description |
|---------|-------|----------|-------------|
| `scan on` | Cyan | 300ms flash | Scanning enabled |
| `scan off` | White | 300ms flash | Scanning disabled |
| `scan on` (error) | Red | 500ms flash | Command error (wrong mode) |

## Features

### ✨ Visual Enhancements

1. **Startup Animation**
   - Purple flash on initialization
   - Green confirmation when ready
   - Clear visual boot sequence

2. **Smooth Animations**
   - Breathing pulse effect for idle state
   - Smooth color transitions
   - Non-blocking animation timing

3. **Connection Feedback**
   - Real-time connection progress indication
   - Clear success/failure feedback
   - Visual confirmation of network commands

### 🔧 Technical Implementation

**Conditional Compilation:**
```cpp
#ifdef USE_NEOPIXEL
    // NeoPixel-specific code
    setNeoPixelColor(r, g, b);
#else
    // Standard LED code
    digitalWrite(LED_PIN, state);
#endif
```

**Key Functions:**
- `setNeoPixelColor()` - Set RGB color
- `setNeoPixelStatus()` - Automatic status display
- `neoPixelBlink()` - Blinking patterns
- `neoPixelPulse()` - Breathing effects

## Build Commands

### Standard ESP32 (Regular LED)
```bash
pio run -e esp32dev
```

### Feather ESP32-S3 TFT (NeoPixel)
```bash
pio run -e adafruit_feather_esp32s3_tft
```

### Build Both
```bash
pio run
```

## Memory Usage

| Board | RAM Usage | Flash Usage |
|-------|-----------|-------------|
| ESP32 Dev | 45,496 bytes (13.9%) | 805,077 bytes (61.4%) |
| Feather S3 TFT | 57,200 bytes (17.5%) | 783,093 bytes (54.3%) |

The NeoPixel support adds minimal overhead while providing significant visual enhancement.

## Usage Examples

### Basic WiFi Operations with Visual Feedback

```
> mode station     # LED: Green (ready)
> scan on          # LED: Cyan flash → Cyan blinking (scanning)
> scan off         # LED: White flash → Green (ready)
> connect MyWiFi pass123  # LED: Yellow blinking → Green (success) or Red (failure)
> mode ap          # LED: Orange blinking (AP active)
```

### Status Monitoring

The NeoPixel automatically reflects the current system state:
- **Blue pulse**: System is idle and ready
- **Green solid**: Station mode, ready for commands
- **Cyan blink**: Actively scanning for networks
- **Orange blink**: Access Point mode is active
- **Yellow blink**: Attempting to connect to WiFi

## Troubleshooting

### Common Issues

1. **NeoPixel Not Working**
   - Verify `USE_NEOPIXEL=1` flag is set
   - Check pin 33 connection
   - Ensure Adafruit NeoPixel library is installed

2. **Compilation Errors**
   - Missing NeoPixel library: `pio pkg install -l "adafruit/Adafruit NeoPixel@^1.12.0"`
   - Wrong environment: Use `adafruit_feather_esp32s3_tft` target

3. **Brightness Issues**
   - Adjust `NEOPIXEL_BRIGHTNESS` in config.h (0-255)
   - Check power supply if LED is dim

### Pin Configuration

If using a different NeoPixel pin, update `config.h`:
```cpp
#define NEOPIXEL_PIN 33  // Change to your pin
```

## Compatibility

- ✅ **Standard ESP32**: Uses built-in LED (pin 2)
- ✅ **Feather ESP32-S3 TFT**: Uses NeoPixel (pin 33)
- ✅ **Conditional Compilation**: No code bloat on standard boards
- ✅ **Same Functionality**: All WiFi features work identically
- ✅ **Library Management**: Automatic dependency handling per environment

The NeoPixel integration provides professional-grade visual feedback while maintaining full compatibility with standard ESP32 development boards.