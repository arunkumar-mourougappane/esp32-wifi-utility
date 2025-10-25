# Station Configuration Quick Reference

## Save Configuration

```bash
# Save current connection (must be connected)
station save

# Save custom credentials
station save MyNetwork Password123           # Auto-connect enabled
station save MyNetwork Password123 yes       # Explicit auto-connect
station save MyNetwork Password123 no        # No auto-connect

# Open network (no password)
station save OpenNetwork ""

# With spaces (use quotes)
station save "My Network" "My Password 123" yes
```

## View Configuration

```bash
# Show saved configuration (password masked)
station show
```

## Load Configuration

```bash
# Load and display saved configuration
station load
```

## Clear Configuration

```bash
# Remove saved configuration (disables auto-connect)
station clear
```

## Parameters

| Parameter | Required | Valid Values | Default |
|-----------|----------|--------------|---------|
| SSID      | Yes      | 1-32 chars   | -       |
| Password  | Yes      | 0-63 chars   | -       |
| Auto-connect| No     | yes/no, true/false, 1/0 | yes |

## Boot Behavior

| Scenario | Boot Behavior |
|----------|---------------|
| No saved config | Device starts in IDLE mode |
| Saved config with auto-connect=yes | Device connects to saved network |
| Saved config with auto-connect=no | Device starts in IDLE, credentials available |
| Both AP and Station saved | **AP takes priority** |

## Priority System

When both AP and Station configs are saved:

1. **AP Config with auto-start**: Device boots as AP (highest priority)
2. **Station Config with auto-connect**: Device connects to WiFi
3. **Neither with auto**: Device starts in IDLE

To change priority:
```bash
# Use Station mode
ap clear
station save MyNetwork Pass yes

# Use AP mode
station clear
ap save MyAP APPass 6 yes
```

## Common Use Cases

### IoT Device (Home Network)
```bash
connect MyHomeWiFi Password123
station save
# Device will auto-connect on every boot
```

### Development Device
```bash
station save DevNetwork DevPass no
# Credentials saved but no auto-connect
# Use 'connect MyDevWiFi DevPass' when needed
```

### Portable Device (Multiple Networks)
```bash
# At home
station save HomeWiFi HomePass yes

# At office (clear home network first)
station clear
station save OfficeWiFi OfficePass yes
```

### Backup Configuration
```bash
# Save backup network
station save GuestWiFi GuestPass no
# Won't auto-connect but credentials are saved
```

## Workflow Examples

### First Time Setup
```bash
1. mode station          # Enter station mode
2. scan now              # Find networks
3. connect MyWiFi Pass   # Connect
4. station save          # Save credentials
5. reset                 # Test auto-connect
```

### Change Network
```bash
1. station clear         # Clear old network
2. connect NewWiFi Pass  # Connect to new
3. station save          # Save new network
4. reset                 # Test
```

### Temporary Connection
```bash
1. mode station
2. connect TempWiFi TempPass  # Connect without saving
3. # Work...
4. disconnect            # Disconnect
# Saved config unchanged
```

## Troubleshooting

**Not auto-connecting?**
```bash
station show             # Check if auto-connect is "Yes"
ap show                  # Check if AP has priority
ap clear                 # Clear AP if needed
```

**Wrong credentials saved?**
```bash
station clear
connect CorrectSSID CorrectPass
station save
```

**Can't connect?**
```bash
mode station
scan now                 # Check if network is visible
# Verify SSID spelling and password
station clear
station save SSID Pass
reset
```

**Want manual control?**
```bash
# Save credentials but disable auto-connect
station save MyWiFi MyPass no

# Connect manually when needed
mode station
connect MyWiFi MyPass
```

## Security Tips

✓ Use strong passwords (WPA2/WPA3 standards)
✓ Clear credentials before disposing device
✓ Keep device physically secure
✓ Use `station show` to verify (password masked)
⚠ Serial access can retrieve credentials
⚠ Physical access = full access to config

## Quick Commands

```bash
# Save current
station save

# View saved
station show

# Clear
station clear

# Priority check
ap show && station show

# Reset all
ap clear && station clear && reset
```
