# Usage Examples

Practical examples demonstrating ESP32 WiFi Utility commands and features.

## Table of Contents

- [Usage Examples](#usage-examples)
  - [Table of Contents](#table-of-contents)
  - [Example 1: Interactive Command Prompt](#example-1-interactive-command-prompt)
  - [Example 2: Device Startup (Idle Mode)](#example-2-device-startup-idle-mode)
  - [Example 3: Enhanced WiFi Network Scanner](#example-3-enhanced-wifi-network-scanner)
  - [Example 4: Access Point Mode with QR Code](#example-4-access-point-mode-with-qr-code)
  - [Example 5: Custom Access Point Configuration](#example-5-custom-access-point-configuration)
  - [Example 6: Status Monitoring \& Mode Switching](#example-6-status-monitoring--mode-switching)
  - [Example 7: QR Code Command](#example-7-qr-code-command)
  - [Example 8: AP Client Management](#example-8-ap-client-management)
  - [Example 9: Network Connection and iPerf Testing](#example-9-network-connection-and-iperf-testing)
  - [Related Documentation](#related-documentation)

---

## Example 1: Interactive Command Prompt

```text
🟡 Device in IDLE mode - Ready for commands
Type 'help' for available commands

🟡 ESP32> help
📋 AVAILABLE COMMANDS:
┌─────────────────┬──────────────────────────────────────┐
│ Command         │ Description                          │
├─────────────────┼──────────────────────────────────────┤
│ mode idle       │ Set device to idle/standby mode      │
│ mode station    │ Enable station mode for scanning     │
│ mode ap         │ Start as Access Point                │
└─────────────────┴──────────────────────────────────────┘

🟡 ESP32> mode ap
✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 12345678
  IP Address: 192.168.4.1

📡 ESP32> status
📊 CURRENT STATUS:
──────────────────
WiFi Mode: 📡 Access Point
Connected Clients: 0

📡 ESP32> mode station
✓ Station mode activated - Ready to scan for networks

🔍 ESP32> scan on
✓ WiFi scanning ENABLED

🔍 ESP32> clear
==========================================
       ESP32 WiFi Scanner & AP
==========================================
🔍 STATION mode - Scanning active
==========================================

🔍 ESP32> _
```

---

## Example 2: Device Startup (Idle Mode)

```text
🟡 Device in IDLE mode - Ready for commands
Type 'help' for available commands

🟡 ESP32> status
📊 CURRENT STATUS:
──────────────────
WiFi Mode: 🟡 Idle (Ready)
Scanning: Disabled
LED Status: Slow Pulse (every 2s)
Free Heap: 298756 bytes

> help
📋 AVAILABLE COMMANDS:
┌─────────────────┬──────────────────────────────────────┐
│ Command         │ Description                          │
├─────────────────┼──────────────────────────────────────┤
│ mode idle       │ Set device to idle/standby mode      │
│ mode station    │ Enable station mode for scanning     │
│ mode ap         │ Start as Access Point                │
│ mode off        │ Disable WiFi completely              │
└─────────────────┴──────────────────────────────────────┘
```

---

## Example 3: Enhanced WiFi Network Scanner

```text
> mode station
✓ Station mode activated - Ready to scan for networks

> scan now
🔍 === WiFi Network Scanner === 🔍
Scanning for available networks...
✅ Discovered 5 networks:

╔════╤═══════════════════════════╤══════╤════╤═══════════════════╤═════════╤═══════════════════╗
║ ID │         SSID              │ RSSI │ CH │    Encryption     │ Quality │      BSSID        ║
╠════╪═══════════════════════════╪══════╪════╪═══════════════════╪═════════╪═══════════════════╣
║  1 │ HomeWiFi                  │  -42 │  6 │ 🔒 WPA2           │ 🟢  95% │ AA:BB:CC:DD:EE:FF ║
║  2 │ OfficeNet                 │  -67 │ 11 │ 🔒 WPA2           │ 🟡  65% │ 11:22:33:44:55:66 ║
║  3 │ CafeGuest                 │  -78 │  1 │ 🔓 Open           │ 🟠  35% │ 22:33:44:55:66:77 ║
║  4 │ Neighbor5G                │  -52 │ 36 │ 🔐 WPA3           │ 🟢  85% │ 33:44:55:66:77:88 ║
║  5 │ <Hidden Network>          │  -85 │  9 │ 🔒 WPA2           │ 🔴  25% │ 44:55:66:77:88:99 ║
╚════╧═══════════════════════════╧══════╧════╧═══════════════════╧═════════╧═══════════════════╝

📈 Network Summary:
├─ 🔓 Open: 1  🔒 WEP: 0  🔒 WPA2: 3  🔐 WPA3: 1
├─ 📶 Strong signals (>-60dBm): 2
├─ 📱 Weak signals (<-80dBm): 1
├─ 📡 Most congested channel: 6 (2 networks)
└─ 💡 Recommended channels for AP: 1, 6, 11 (least interference)

> scan info 1
📡 === Detailed Network Information === 📡
┌─────────────────────────────────────────────────────────┐
│ 🏷️  Network Name: HomeWiFi                             │
├─────────────────────────────────────────────────────────┤
│ 🔗 BSSID (MAC):  AA:BB:CC:DD:EE:FF                     │
│ 📶 Signal (RSSI): -42 dBm                              │
│ 📊 Signal Quality: 95% (Excellent) 🟢🟢🟢🟢              │
│ 📻 Channel:      6 (2.4GHz)                            │
│ 🚦 Congestion:   Light (2 networks on this channel)    │
│ 🔐 Security:     🔒 WPA2 Personal                      │
│ 🛡️  Security Level: 🟢 Good                            │
├─────────────────────────────────────────────────────────┤
│ 💡 Connection Analysis:                              │
│ 📶 Excellent signal strength for stable connection   │
│  Est. Speed:   50-150 Mbps (802.11n)               │
└─────────────────────────────────────────────────────────┘
💡 To connect: connect "HomeWiFi" <password>
```

---

## Example 4: Access Point Mode with QR Code

```text
> mode ap
✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 12345678
  IP Address: 192.168.4.1
  Use 'ap info' for detailed information

═══════════════════════════════════════════════════════════
                    AP CONNECTION QR CODE
═══════════════════════════════════════════════════════════
Scan this QR code with your mobile device to connect:

██████████████  ██  ██  ██████████████
██          ██  ████    ██          ██
██  ██████  ██  ██  ██  ██  ██████  ██
██  ██████  ██    ██    ██  ██████  ██
██  ██████  ██  ██      ██  ██████  ██
██          ██    ██    ██          ██
██████████████  ██  ██  ██████████████
                ██  ██
██████  ████████    ████  ██    ██████
      ██    ██  ██  ██████  ██    ████
██  ██████    ████  ██    ████      ██
████████  ██  ██    ██  ████████  ████
████    ██  ██    ████  ██        ████
                ██    ██  ████  ██████
██████████████  ██████    ████    ████
██          ██    ██    ████    ██  ██
██  ██████  ██  ██████████████  ██████
██  ██████  ██    ██      ██████  ████
██  ██████  ██  ████    ████  ████  ██
██          ██  ██    ██    ██
██████████████  ██    ████████  ██  ██

Network Information:
  SSID: ESP32-WiFiScanner
  Security: WPA
  Password: 12345678
═══════════════════════════════════════════════════════════

> ap info
📡 ACCESS POINT INFORMATION:
─────────────────────────────
SSID: ESP32-WiFiScanner
Password: 12345678
IP Address: 192.168.4.1
MAC Address: 24:6F:28:XX:XX:XX
Connected Clients: 2
Channel: 1
```

---

## Example 5: Custom Access Point Configuration

```text
> mode ap "MyCustomHotspot" "SecurePassword123"
✓ Custom Access Point mode activated
  SSID: MyCustomHotspot
  Password: SecurePassword123
  IP Address: 192.168.4.1
  Use 'ap info' for detailed information

═══════════════════════════════════════════════════════════
                    AP CONNECTION QR CODE
═══════════════════════════════════════════════════════════
Scan this QR code with your mobile device to connect:

██████████████  ██  ██  ██████████████
██          ██  ████    ██          ██
██  ██████  ██  ██  ██  ██  ██████  ██
██  ██████  ██    ██    ██  ██████  ██
██  ██████  ██  ██      ██  ██████  ██
██          ██    ██    ██          ██
██████████████  ██  ██  ██████████████
                ██  ██
██████  ████████    ████  ██    ██████
      ██    ██  ██  ██████  ██    ████
██  ██████    ████  ██    ████      ██
████████  ██  ██    ██  ████████  ████
████    ██  ██    ████  ██        ████
                ██    ██  ████  ██████
██████████████  ██████    ████    ████
██          ██    ██    ████    ██  ██
██  ██████  ██  ██████████████  ██████
██  ██████  ██    ██      ██████  ████
██  ██████  ██  ████    ████  ████  ██
██          ██  ██    ██    ██
██████████████  ██    ████████  ██  ██

Network Information:
  SSID: MyCustomHotspot
  Security: WPA
  Password: SecurePassword123
═══════════════════════════════════════════════════════════

> ap info
📡 ACCESS POINT INFORMATION:
─────────────────────────────
SSID: MyCustomHotspot
Password: SecurePassword123
IP Address: 192.168.4.1
MAC Address: 24:6F:28:XX:XX:XX
Connected Clients: 1
Channel: 1

> mode ap
✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 123456789
  IP Address: 192.168.4.1

Note: Switching to 'mode ap' without parameters resets to default configuration
```

---

## Example 6: Status Monitoring & Mode Switching

```text
> status
📊 CURRENT STATUS:
──────────────────
WiFi Mode: 📡 Access Point
Scanning: Disabled
LED Status: Fast Blinking
AP IP: 192.168.4.1
Connected Clients: 1
Free Heap: 298756 bytes

> mode idle
🟡 Device set to IDLE mode - Ready for commands
  Use 'mode station' or 'mode ap' to activate WiFi

> mode off
✓ WiFi disabled
```

---

## Example 7: QR Code Command

```text
> qr
═══════════════════════════════════════════════════════════
                    AP CONNECTION QR CODE
═══════════════════════════════════════════════════════════
Scan this QR code with your mobile device to connect:

██████████████  ██  ██  ██████████████
██          ██  ████    ██          ██
██  ██████  ██  ██  ██  ██  ██████  ██
██  ██████  ██    ██    ██  ██████  ██
██  ██████  ██  ██      ██  ██████  ██
██          ██    ██    ██          ██
██████████████  ██  ██  ██████████████
                ██  ██
██████  ████████    ████  ██    ██████
      ██    ██  ██  ██████  ██    ████
██  ██████    ████  ██    ████      ██
████████  ██  ██    ██  ████████  ████
████    ██  ██    ████  ██        ████
                ██    ██  ████  ██████
██████████████  ██████    ████    ████
██          ██    ██    ████    ██  ██
██  ██████  ██  ██████████████  ██████
██  ██████  ██    ██      ██████  ████
██  ██████  ██  ████    ████  ████  ██
██          ██  ██    ██    ██
██████████████  ██    ████████  ██  ██

Network Information:
  SSID: ESP32-WiFiScanner
  Security: WPA
  Password: 12345678
═══════════════════════════════════════════════════════════
```

---

## Example 8: AP Client Management

```text
> mode ap
✓ Access Point mode activated
  SSID: ESP32-WiFiScanner
  Password: 12345678
  IP Address: 192.168.4.1

> ap clients
👥 CONNECTED CLIENTS:
─────────────────────
ID │ MAC Address       │ RSSI
───┼───────────────────┼──────
 1 │ AA:BB:CC:DD:EE:FF │  -45
 2 │ 11:22:33:44:55:66 │  -52

Total clients: 2
Use 'deauth <ID>' (e.g., 'deauth 1') or 'deauth <MAC>' to disconnect
Use 'deauth all' to disconnect all clients

> deauth 1
✓ Client disconnected: AA:BB:CC:DD:EE:FF

> deauth AA:BB:CC:DD:EE:FF
✗ Error: MAC address not found in connected clients

> deauth all
⚠️  Deauthenticating all 1 connected clients...
✓ Disconnected: 11:22:33:44:55:66
✓ Successfully deauthenticated 1 out of 1 clients
```

---

## Example 9: Network Connection and iPerf Testing

```text
> mode station
✓ Station mode activated - Ready to scan for networks

> connect "HomeWiFi" "mypassword123"
🔗 Connecting to 'HomeWiFi'...
........
✓ Connected to 'HomeWiFi'
  IP Address: 192.168.1.105
  Gateway: 192.168.1.1
  DNS: 192.168.1.1

> iperf
⚡ === iPerf Network Performance Testing === 📊
Available Commands:
├─ iperf start server [tcp|udp] [port]
├─ iperf start client <host> [tcp|udp] [port] [duration]
├─ iperf stop
├─ iperf status

Examples:
├─ iperf start server tcp 5201
├─ iperf start client 192.168.1.100 udp 5201 30
└─ iperf status

> iperf status
📊 iPerf Status: Idle
💡 Use 'iperf start server' or 'iperf start client <host>' to begin testing

> disconnect
✓ Disconnected from 'HomeWiFi'
```

---

## Related Documentation

- [Command Reference](COMMAND_REFERENCE.md) - Complete command documentation
- [Features Guide](FEATURES.md) - Feature descriptions
- [Quick Start Guide](../README.md#quick-start) - Getting started
- [Channel Analysis Guide](user-guides/CHANNEL_GUIDE.md) - Spectrum analysis examples
- [Latency Testing Guide](user-guides/LATENCY_GUIDE.md) - Performance testing
- [iPerf Guide](user-guides/IPERF_GUIDE.md) - Bandwidth measurement
