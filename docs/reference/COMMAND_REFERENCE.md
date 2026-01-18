# Command Reference

Complete reference guide for all ESP32 WiFi Utility serial commands.

## Table of Contents

- [Command Reference](#command-reference)
  - [Table of Contents](#table-of-contents)
  - [Mode Commands](#mode-commands)
    - [AP Security Types](#ap-security-types)
    - [Examples](#examples)
  - [Access Point Configuration](#access-point-configuration)
    - [Parameters](#parameters)
    - [Examples](#examples-1)
  - [Station Configuration](#station-configuration)
    - [Parameters](#parameters-1)
    - [Security Preference Behavior](#security-preference-behavior)
    - [Examples](#examples-2)
  - [Scanning Commands](#scanning-commands)
    - [Examples](#examples-3)
  - [Channel Analysis Commands](#channel-analysis-commands)
    - [Examples](#examples-4)
  - [Network Performance \& Latency](#network-performance--latency)
    - [Examples](#examples-5)
  - [Network Connection Commands](#network-connection-commands)
    - [Security Parameters](#security-parameters)
    - [Examples](#examples-6)
  - [Network Performance Testing](#network-performance-testing)
  - [Information Commands](#information-commands)
    - [Examples](#examples-7)
  - [Web Server Commands](#web-server-commands)
    - [Examples](#examples-8)
  - [Client Management Commands](#client-management-commands)
    - [Client Identification Methods](#client-identification-methods)
    - [Examples](#examples-9)
    - [Security \& Usage Notes](#security--usage-notes)
  - [Command Line Features](#command-line-features)
    - [🎯 Interactive Prompt Format](#-interactive-prompt-format)
    - [Typing Features](#typing-features)
  - [Related Documentation](#related-documentation)

---

## Mode Commands

| Command                                     | Description                                             |
| ------------------------------------------- | ------------------------------------------------------- |
| `mode idle`                                 | Set device to idle/standby mode (default startup state) |
| `mode station`                              | Switch to station mode for WiFi scanning                |
| `mode ap`                                   | Start Access Point mode with default settings           |
| `mode ap <ssid> <password> [security]`      | Start Access Point with custom SSID and security type   |
| `mode off`                                  | Disable WiFi completely                                 |

### AP Security Types

The optional security parameter accepts:

- `open` - Open network (no password required)
- `wpa2` - WPA2-PSK (default, recommended)
- `wpa3` - WPA3-PSK (maximum security, ESP32-S2/S3/C3+ only)
- `mixed` - WPA2/WPA3 Mixed mode (compatibility mode)

### Examples

```bash
mode ap MyHotspot MyPassword123           # WPA2 (default)
mode ap MyHotspot MyPassword123 wpa3      # WPA3 only
mode ap MyHotspot MyPassword123 mixed     # WPA2/WPA3 mixed
mode ap OpenNetwork "" open               # Open network (no password)
```

---

## Access Point Configuration

Persistent configuration storage for Access Point settings (introduced in v4.2.0).

| Command                                                          | Description                                       |
| ---------------------------------------------------------------- | ------------------------------------------------- |
| `ap save`                                                        | Save current AP settings (must be in AP mode)     |
| `ap save <ssid> <password> [channel] [security] [autostart]`    | Save custom AP configuration                       |
| `ap load`                                                        | Load and apply saved AP configuration             |
| `ap show`                                                        | Display saved or default AP configuration         |
| `ap clear`                                                       | Clear saved AP configuration                      |

### Parameters

- **ssid**: Network name (1-32 characters)
- **password**: Password (8-63 characters for WPA2/WPA3, empty "" for open)
- **channel**: WiFi channel 1-13 (optional, default: 1)
- **security**: Security type (optional, default: `wpa2`)
  - `open` - No password required
  - `wpa2` - WPA2-PSK (recommended)
  - `wpa3` - WPA3-PSK (ESP32-S2/S3/C3+ only)
  - `mixed` - WPA2/WPA3 Mixed mode
- **autostart**: Enable auto-start on boot - `yes`/`no`, `true`/`false`, `1`/`0` (optional, default: yes)

### Examples

```bash
ap save                                           # Save current AP settings
ap save MyHotspot SecurePass123 6 wpa2 yes       # WPA2 network on channel 6
ap save MyHotspot SecurePass123 11 wpa3 yes      # WPA3 network (secure)
ap save "Guest WiFi" "" 1 open yes               # Open network (no password)
ap save MyHotspot SecurePass123 6 mixed no       # Mixed mode, no auto-start
ap show                                          # View saved configuration
ap load                                          # Load saved config
mode ap                                          # Start AP with loaded settings
ap clear                                         # Remove saved configuration
```

---

## Station Configuration

Persistent configuration storage for Station mode WiFi credentials (introduced in v4.2.0).

| Command                                                   | Description                                     |
| --------------------------------------------------------- | ----------------------------------------------- |
| `station save`                                            | Save current connection (must be connected)     |
| `station save <ssid> <password> [security] [autoconnect]` | Save custom station configuration               |
| `station load`                                            | Load and display saved station configuration    |
| `station show`                                            | Display saved station config (password masked)  |
| `station clear`                                           | Clear saved station configuration               |

### Parameters

- **ssid**: Network name to connect to (1-32 characters)
- **password**: Network password (0-63 characters, empty "" for open networks)
- **security**: Security preference (optional, default: `auto`)
  - `auto` - Accept any security type (default)
  - `wpa3prefer` - Prefer WPA3, fallback to WPA2 if unavailable
  - `wpa3only` - Require WPA3 (reject WPA2 and lower)
  - `wpa2min` - Minimum WPA2 (reject WEP/Open networks)
  - `wpa2only` - Require exactly WPA2 (reject WPA3, WEP, Open)
- **autoconnect**: Auto-connect on boot - `yes`/`no`, `true`/`false`, `1`/`0` (optional, default: yes)

### Security Preference Behavior

| Preference    | WPA3 Network | WPA2 Network | WEP/Open Network |
|---------------|--------------|--------------|------------------|
| `auto`        | ✓ Connect    | ✓ Connect    | ✓ Connect        |
| `wpa3prefer`  | ✓ Connect    | ✓ Fallback   | ✗ Reject         |
| `wpa3only`    | ✓ Connect    | ✗ Reject     | ✗ Reject         |
| `wpa2min`     | ✓ Connect    | ✓ Connect    | ✗ Reject         |
| `wpa2only`    | ✗ Reject     | ✓ Connect    | ✗ Reject         |

### Examples

```bash
station save                                          # Save current connection
station save HomeNetwork WiFiPass123 auto yes        # Save with any security
station save HomeNetwork WiFiPass123 wpa3prefer yes  # Prefer WPA3
station save HomeNetwork WiFiPass123 wpa3only yes    # Require WPA3
station save HomeNetwork WiFiPass123 wpa2min yes     # Minimum WPA2 security
station save "Public WiFi" "" auto no                # Open network, no auto-connect
station show                                         # View saved config (password hidden)
station load                                         # Load saved config
mode station && connect HomeNetwork WiFiPass123      # Connect manually
station clear                                        # Remove saved configuration
```

---

## Scanning Commands

Control WiFi network scanning in Station mode.

| Command          | Description                                     |
| ---------------- | ----------------------------------------------- |
| `scan on`        | Start automatic WiFi network scanning           |
| `scan off`       | Stop WiFi scanning                              |
| `scan now`       | Immediate detailed scan with enhanced analysis  |
| `scan info <id>` | Show comprehensive details for specific network |

### Examples

```bash
scan on          # Enable automatic scanning
scan now         # Perform immediate detailed scan
scan info 1      # Show details for network ID 1
scan off         # Disable scanning
```

---

## Channel Analysis Commands

Professional spectrum analysis and channel optimization (introduced in v3.1.0).

| Command                 | Description                                                 |
| ----------------------- | ----------------------------------------------------------- |
| `channel scan`          | Quick spectrum analysis of all 2.4GHz channels (<5 seconds) |
| `channel scan quick`    | Fast 5-second channel congestion scan                       |
| `channel scan detailed` | Comprehensive 30-second spectrum analysis                   |
| `channel monitor start` | Begin continuous background channel monitoring              |
| `channel monitor stop`  | End background monitoring                                   |
| `channel recommend`     | AI-powered optimal channel recommendations with rationale   |
| `channel export`        | Export scan data to JSON format for external analysis       |
| `channel status`        | Show current monitoring status and recent scan results      |
| `channel help`          | Display comprehensive channel analysis command reference    |

### Examples

```bash
channel scan                # Quick 5-second channel analysis
channel scan detailed       # Comprehensive 30-second analysis
channel recommend           # Get AI-powered channel recommendations
channel monitor start       # Start background monitoring
channel status             # Check monitoring status
channel export             # Export data as JSON
channel monitor stop        # Stop monitoring
```

💡 **Professional Analysis**: The channel analysis system provides enterprise-grade network diagnostics with AI-powered recommendations. See [Channel Analysis Guide](user-guides/CHANNEL_GUIDE.md) for complete details.

---

## Network Performance & Latency

Advanced latency testing and jitter analysis (introduced in v3.1.0).

| Command                  | Description                                     |
| ------------------------ | ----------------------------------------------- |
| `latency test <host>`    | Comprehensive latency test with jitter analysis |
| `latency monitor <host>` | Continuous latency monitoring with statistics   |
| `latency report`         | Generate detailed network performance report    |
| `latency help`           | Display latency analysis command reference      |

### Examples

```bash
latency test 8.8.8.8           # Test latency to Google DNS
latency test google.com        # Test using hostname
latency monitor 192.168.1.1    # Monitor gateway latency
latency report                 # Generate performance report
```

💡 **Network Performance**: See [Latency Testing Guide](user-guides/LATENCY_GUIDE.md) for detailed usage and interpretation.

---

## Network Connection Commands

Connect to and disconnect from WiFi networks in Station mode.

| Command                                   | Description                                          |
| ----------------------------------------- | ---------------------------------------------------- |
| `connect <ssid> <password> [security]`    | Connect to a WiFi network with security preferences  |
| `disconnect`                              | Disconnect from current WiFi network                 |

### Security Parameters

Optional security parameter for `connect` command:

- `auto` - Accept any security type (default)
- `wpa3prefer` - Prefer WPA3, fallback to WPA2
- `wpa3only` - Require WPA3 only
- `wpa2min` - Minimum WPA2 (reject WEP/Open)
- `wpa2only` - Require exactly WPA2

### Examples

```bash
connect HomeNetwork MyPassword                    # Auto-negotiate security
connect HomeNetwork MyPassword wpa3prefer        # Prefer WPA3
connect HomeNetwork MyPassword wpa3only          # Require WPA3
connect "Coffee Shop" "" auto                    # Open network
disconnect                                       # Disconnect from network
```

---

## Network Performance Testing

iPerf network bandwidth and performance analysis.

| Command        | Description                      |
| -------------- | -------------------------------- |
| `iperf`        | Show iPerf performance test help |
| `iperf status` | Show current iPerf test status   |

💡 **Network Performance Testing**: The iPerf functionality provides comprehensive network bandwidth and performance analysis with both TCP and UDP protocols. See [iPerf Testing Guide](user-guides/IPERF_GUIDE.md) for detailed usage, configuration options, and examples.

---

## Information Commands

Display device status and network information.

| Command      | Description                                                  |
| ------------ | ------------------------------------------------------------ |
| `status`     | Show current device status and statistics                    |
| `ap info`    | Show detailed Access Point information (AP mode only)        |
| `ap clients` | List all connected clients with MAC addresses (AP mode only) |
| `qr`         | Display QR code for easy mobile connection (AP mode only)    |
| `clear`      | Clear console screen and redisplay header                    |
| `help`       | Display all available commands                               |

### Examples

```bash
status        # Display comprehensive device status
ap info       # Show AP details (when in AP mode)
ap clients    # List connected clients (when in AP mode)
qr            # Display QR code for mobile connection
help          # Show command help
clear         # Clear screen
```

---

## Web Server Commands

Browser-based interface control (available on both ESP32dev and Feather boards).

| Command            | Description                                     |
| ------------------ | ----------------------------------------------- |
| `webserver start`  | Launch web server for browser-based interface   |
| `webserver stop`   | Stop the web server                             |
| `webserver status` | Check server status and get access URL          |
| `webserver`        | Display web server help and feature information |

### Examples

```bash
webserver start       # Start the web interface
webserver status      # Get access URL and status
webserver stop        # Stop web server
```

💡 **Web Interface Access**: Once started, access the web interface at the displayed IP address (e.g., `http://192.168.4.1` in AP mode or your device IP in Station mode). The web interface provides real-time monitoring, interactive WiFi scanning with clickable network details, and comprehensive channel analysis through a beautiful mobile-responsive dashboard.

---

## Client Management Commands

Manage connected clients in Access Point mode.

| Command        | Description                                                         |
| -------------- | ------------------------------------------------------------------- |
| `deauth <ID>`  | Disconnect client by ID number (e.g., `deauth 1`, `deauth 2`)       |
| `deauth <MAC>` | Disconnect client by MAC address (e.g., `deauth AA:BB:CC:DD:EE:FF`) |
| `deauth all`   | Disconnect all connected clients from the Access Point              |

### Client Identification Methods

The `deauth` command accepts clients in these formats:

- **Client ID**: `1`, `2`, `3` (easiest method - use numbers from `ap clients` list)
- **MAC with colons**: `AA:BB:CC:DD:EE:FF`
- **MAC without colons**: `AABBCCDDEEFF`
- **Case insensitive**: both uppercase and lowercase MAC addresses work

### Examples

```bash
ap clients                    # List connected clients
deauth 1                      # Disconnect client ID 1
deauth AA:BB:CC:DD:EE:FF     # Disconnect by MAC address
deauth all                    # Disconnect all clients
```

### Security & Usage Notes

- **Purpose**: Deauthentication is useful for testing, access control, and network management
- **Legal Notice**: Only use on networks you own or have explicit permission to test
- **Automatic Reconnection**: Most devices will automatically attempt to reconnect after deauthentication
- **Range**: Deauthentication only affects devices connected to your ESP32 Access Point

---

## Command Line Features

### 🎯 Interactive Prompt Format

The command prompt dynamically shows the current device state:

| Prompt      | Mode             | Description                        |
| ----------- | ---------------- | ---------------------------------- |
| `🟡 ESP32>` | **Idle**         | Device ready, WiFi disabled        |
| `🔍 ESP32>` | **Station**      | Ready to scan or actively scanning |
| `📡 ESP32>` | **Access Point** | WiFi hotspot active                |
| `🔴 ESP32>` | **Off**          | WiFi completely disabled           |

### Typing Features

- **Real-time echo**: Characters appear as you type
- **Backspace support**: Use Backspace or Delete to correct mistakes
- **Command execution**: Press Enter to execute commands
- **Screen clearing**: Use `clear` command to clean up terminal output
- **Error feedback**: Invalid commands show helpful error messages

---

## Related Documentation

- [Usage Examples](USAGE_EXAMPLES.md) - Practical examples for all commands
- [Quick Start Guide](../../README.md#quick-start) - Getting started
- [Features Guide](../features/FEATURES.md) - Complete feature documentation
- [Security Configuration](../security/SECURITY_CONFIGURATION.md) - Security setup guide
- [Channel Analysis Guide](../guides/CHANNEL_GUIDE.md) - Spectrum analysis
- [Latency Testing Guide](../guides/LATENCY_GUIDE.md) - Network performance
- [iPerf Guide](../guides/IPERF_GUIDE.md) - Bandwidth testing
