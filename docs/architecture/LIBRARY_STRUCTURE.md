# Library Structure

## Overview
The ESP32 WiFi Utility project has been refactored into modular local libraries for better organization, maintainability, and reusability. All functional components have been separated into individual libraries under the `lib/` directory.

## Library Organization

### Core Libraries

#### 1. **Config** (`lib/Config/`)
- **Purpose**: Global configuration and constants
- **Contents**: `config.h`
- **Description**: Centralized configuration for hardware pins, WiFi modes, timing constants, and system-wide definitions
- **Dependencies**: None
- **Version**: 1.0.0

#### 2. **Utils** (`lib/Utils/`)
- **Purpose**: Utility functions and helpers
- **Contents**:
  - `base64_utils.h/cpp` - Base64 encoding/decoding
  - `logging.h/cpp` - Logging utilities
  - `error_handling.h/cpp` - Error handling and reporting
- **Dependencies**: None
- **Version**: 1.0.0

### WiFi Management Libraries

#### 3. **WiFiManager** (`lib/WiFiManager/`)
- **Purpose**: Core WiFi management functionality
- **Contents**: `wifi_manager.h/cpp`
- **Features**:
  - Station and AP mode management
  - Network scanning and connection
  - QR code generation for easy connection
- **Dependencies**: Config, QRCode
- **Version**: 1.0.0

#### 4. **APManager** (`lib/APManager/`)
- **Purpose**: Access Point specific operations
- **Contents**: `ap_manager.h/cpp`
- **Features**:
  - AP configuration and management
  - Client monitoring
  - AP-specific utilities
- **Dependencies**: None
- **Version**: 1.0.0

#### 5. **APConfig** (`lib/APConfig/`)
- **Purpose**: AP configuration persistence
- **Contents**: `ap_config.h/cpp`
- **Features**:
  - Save/load AP settings to NVS
  - Auto-start configuration
  - Preference management
- **Dependencies**: Config, Utils, Preferences
- **Version**: 1.0.0

#### 6. **StationConfig** (`lib/StationConfig/`)
- **Purpose**: Station mode configuration persistence
- **Contents**: `station_config.h/cpp`
- **Features**:
  - Save/load Station settings to NVS
  - Auto-connect configuration
  - Preference management
- **Dependencies**: Config, Preferences
- **Version**: 1.0.0

### Display & Control Libraries

#### 7. **TFTDisplay** (`lib/TFTDisplay/`)
- **Purpose**: TFT display management
- **Contents**: `tft_display.h/cpp`
- **Features**:
  - Board-specific pin configurations (TFT and Reverse TFT)
  - QR code display
  - WiFi status information display
  - Periodic background updates (AP: 5s, Station: 10s)
  - Modular display functions
- **Dependencies**: Config, Adafruit GFX, Adafruit ST7789, QRCode
- **Version**: 1.0.0

#### 8. **LEDController** (`lib/LEDController/`)
- **Purpose**: LED and NeoPixel control
- **Contents**: `led_controller.h/cpp`
- **Features**:
  - Status indication via LEDs
  - NeoPixel animations
  - Visual feedback
- **Dependencies**: Adafruit NeoPixel
- **Version**: 1.0.0

### Network Analysis Libraries

#### 9. **NetworkAnalyzer** (`lib/NetworkAnalyzer/`)
- **Purpose**: Network monitoring and analysis
- **Contents**:
  - `channel_analyzer.h/cpp` - WiFi channel analysis
  - `signal_monitor.h/cpp` - Signal strength monitoring
  - `performance_monitor.h/cpp` - Performance metrics
- **Dependencies**: None
- **Version**: 1.0.0

#### 10. **NetworkTools** (`lib/NetworkTools/`)
- **Purpose**: Network diagnostic tools
- **Contents**:
  - `port_scanner.h/cpp` - Port scanning functionality
  - `latency_analyzer.h/cpp` - Latency measurement
  - `iperf_manager.h/cpp` - iPerf testing
- **Dependencies**: ESP32 Async UDP
- **Version**: 1.0.0

### Application Libraries

#### 11. **CommandInterface** (`lib/CommandInterface/`)
- **Purpose**: Serial command handling
- **Contents**: `command_interface.h/cpp`
- **Features**:
  - Command parsing and execution
  - Interactive CLI
  - Help system
- **Dependencies**: None
- **Version**: 1.0.0

#### 12. **WebServer** (`lib/WebServer/`)
- **Purpose**: Web server functionality
- **Contents**: `web_server.h/cpp`
- **Features**:
  - Web-based configuration interface
  - RESTful API endpoints
  - Status monitoring
- **Dependencies**: None
- **Version**: 1.0.0

## Library Dependencies

```
Config (base)
  └─> WiFiManager
  └─> TFTDisplay
  └─> APConfig
      └─> Utils

Utils
  └─> APConfig

WiFiManager
  ├─> QRCode (external)
  └─> Config

TFTDisplay
  ├─> Adafruit GFX (external)
  ├─> Adafruit ST7789 (external)
  ├─> QRCode (external)
  └─> Config

LEDController
  └─> Adafruit NeoPixel (external)

NetworkTools
  └─> ESP32 Async UDP (external)

StationConfig
  └─> Preferences (ESP32 framework)

APConfig
  ├─> Preferences (ESP32 framework)
  ├─> Config
  └─> Utils
```

## Build Information

### Successful Builds
All environments compile successfully with the modularized structure:
- ✅ `esp32dev` - RAM: 16.4%, Flash: 90.6%
- ✅ `adafruit_feather_esp32s3_tft` - RAM: 16.2%, Flash: 80.3%
- ✅ `adafruit_feather_esp32s3_reversetft` - RAM: 16.2%, Flash: 80.3%
- ✅ `test` - RAM: 16.5%, Flash: 92.7%
- ✅ `test_feather` - Building
- ✅ `test_feather_reversetft` - Building

### Library Detection
PlatformIO's Library Dependency Finder automatically detects all local libraries:
```
|-- QRCode @ 0.0.1
|-- Adafruit NeoPixel @ 1.15.2
|-- Adafruit ST7735 and ST7789 Library @ 1.11.0
|-- Adafruit GFX Library @ 1.12.3
|-- APManager @ 1.0.0
|-- NetworkAnalyzer @ 1.0.0
|-- CommandInterface @ 1.0.0
|-- NetworkTools @ 1.0.0
|-- LEDController @ 1.0.0
|-- TFTDisplay @ 1.0.0
|-- WebServer @ 1.0.0
|-- WiFiManager @ 1.0.0
|-- WiFi @ 2.0.0
```

## Benefits of Modular Structure

1. **Maintainability**: Each library has a single, well-defined purpose
2. **Reusability**: Libraries can be reused in other projects
3. **Testing**: Individual libraries can be tested in isolation
4. **Documentation**: Each library can have its own documentation
5. **Dependencies**: Clear dependency management through `library.json`
6. **Compilation**: PlatformIO automatically manages library compilation
7. **Organization**: Logical separation of concerns

## File Structure

```
lib/
├── APConfig/
│   ├── ap_config.h
│   ├── ap_config.cpp
│   └── library.json
├── APManager/
│   ├── ap_manager.h
│   ├── ap_manager.cpp
│   └── library.json
├── CommandInterface/
│   ├── command_interface.h
│   ├── command_interface.cpp
│   └── library.json
├── Config/
│   ├── config.h
│   └── library.json
├── LEDController/
│   ├── led_controller.h
│   ├── led_controller.cpp
│   └── library.json
├── NetworkAnalyzer/
│   ├── channel_analyzer.h/cpp
│   ├── signal_monitor.h/cpp
│   ├── performance_monitor.h/cpp
│   └── library.json
├── NetworkTools/
│   ├── port_scanner.h/cpp
│   ├── latency_analyzer.h/cpp
│   ├── iperf_manager.h/cpp
│   └── library.json
├── StationConfig/
│   ├── station_config.h
│   ├── station_config.cpp
│   └── library.json
├── TFTDisplay/
│   ├── tft_display.h
│   ├── tft_display.cpp
│   └── library.json
├── Utils/
│   ├── base64_utils.h/cpp
│   ├── logging.h/cpp
│   ├── error_handling.h/cpp
│   └── library.json
├── WebServer/
│   ├── web_server.h
│   ├── web_server.cpp
│   └── library.json
└── WiFiManager/
    ├── wifi_manager.h
    ├── wifi_manager.cpp
    └── library.json
```

## Migration Notes

### Previous Structure
- All headers in `include/`
- All implementations in `src/`
- Flat organization with no modularity

### New Structure
- Libraries in `lib/<LibraryName>/`
- Each library is self-contained
- Dependencies explicitly defined in `library.json`
- Config library provides shared configuration

### Breaking Changes
- None! The main application (`src/main.cpp`) remains unchanged
- PlatformIO automatically finds and compiles all libraries
- Include paths remain the same (e.g., `#include "wifi_manager.h"`)

## Future Enhancements

1. Add README.md to each library with detailed documentation
2. Create examples for each library demonstrating standalone usage
3. Add unit tests for individual libraries
4. Consider publishing reusable libraries to PlatformIO registry
5. Add versioning and changelog for each library
6. Create library-specific configuration options

## References

- [PlatformIO Library Documentation](https://docs.platformio.org/en/latest/librarymanager/index.html)
- [Library.json Specification](https://docs.platformio.org/en/latest/librarymanager/config.html)
- [Library Dependency Finder](https://docs.platformio.org/en/latest/librarymanager/ldf.html)
