# Station Mode Configuration Persistence

## Overview

The ESP32 WiFi Utility now supports saving WiFi network credentials for Station mode to non-volatile storage (NVS). When a configuration is saved with auto-connect enabled, the device will automatically connect to the saved network on boot.

## Features

- **Save WiFi Credentials**: Store SSID and password for networks
- **Auto-Connect on Boot**: Automatically connect to saved network on startup
- **Persistent Storage**: Configuration survives power cycles and resets
- **Easy Management**: Simple commands to save, load, show, and clear credentials
- **Password Masking**: Passwords are masked when displayed for security

## Commands

### Save Current Connection

```
station save
```

Saves the current WiFi connection (must be connected). The device will auto-connect to this network on next boot.

### Save Custom Network Credentials

```
station save <ssid> <password> [autoconnect]
```

**Parameters:**
- `ssid`: WiFi SSID (1-32 characters)
- `password`: WiFi password (0-63 characters, empty for open networks)
- `autoconnect`: yes/no or true/false (optional, default: yes)

**Examples:**
```
station save MyHomeWiFi MyPassword123
station save MyHomeWiFi MyPassword123 yes
station save MyHomeWiFi MyPassword123 no
station save "Coffee Shop WiFi" "Guest Password" yes
station save OpenNetwork "" yes
```

### Load Saved Configuration

```
station load
```

Loads and displays the saved station configuration. Use `mode station` and `connect` to connect, or restart to auto-connect.

### Show Configuration

```
station show
```

Displays the saved station configuration (password is masked for security).

### Clear Saved Configuration

```
station clear
```

Removes the saved station configuration. The device will not auto-connect on next boot.

## Usage Examples

### Save Current Connection

1. Connect to a network:
   ```
   mode station
   connect MyNetwork Password123
   ```

2. Save the connection:
   ```
   station save
   ```

3. The device will now auto-connect to this network on reboot

### Configure Network for Auto-Connect

```
station save HomeNetwork SecurePass2024 yes
```

This saves:
- SSID: HomeNetwork
- Password: SecurePass2024
- Auto-connect: Enabled

### Save Without Auto-Connect

```
station save OfficeWiFi OfficePass123 no
```

The credentials are saved but the device won't auto-connect. Useful for networks you want saved but don't want to connect to automatically.

### Disable Auto-Connect

```
station clear
```

The device will return to idle mode on boot instead of trying to connect.

## Configuration Details

### Storage

Configuration is stored in ESP32 NVS (Non-Volatile Storage) in the `sta_config` namespace. The data persists across:
- Power cycles
- Software resets
- Firmware updates (unless NVS is erased)

### Validation

The system validates:
- **SSID length**: 1-32 characters
- **Password length**: 0-63 characters (empty allowed for open networks)

Invalid configurations will be rejected with an error message.

### Priority

If both AP and Station configurations are saved with auto-start/auto-connect enabled:
- **AP configuration takes priority** - Device will start as AP
- To use Station mode instead, clear AP config: `ap clear`

### Default Behavior

| Scenario | Boot Behavior |
|----------|---------------|
| No saved config | Device starts in IDLE mode |
| Saved config with auto-connect=yes | Device connects to saved network |
| Saved config with auto-connect=no | Device starts in IDLE, credentials available |

## Security Considerations

### Password Storage
- Passwords are stored in NVS
- Passwords are masked (****) when displayed via `station show`
- WiFi.psk() is used to retrieve current password when saving
- Physical access to device allows password retrieval via serial commands

### Best Practices
1. **Use Strong Passwords**: Follow WPA2/WPA3 password requirements
2. **Secure Physical Access**: Keep devices physically secure
3. **Clear Credentials**: Use `station clear` before disposing of devices
4. **Test First**: Verify connection works before saving

## Boot Sequence

When device boots with saved station configuration:

1. Initialize serial and WiFi subsystems
2. Check for saved AP configuration (priority)
3. Check for saved Station configuration
4. If found AND auto-connect enabled:
   - Set mode to STATION
   - Attempt connection to saved network
5. Otherwise:
   - Start in IDLE mode

## Troubleshooting

### Configuration Not Saving

- Ensure SSID is 1-32 characters
- Ensure password is 0-63 characters
- Check serial output for error messages
- Try `station clear` then save again

### Auto-Connect Not Working

- Verify with `station show` that auto-connect is "Yes"
- Check if AP config exists (has priority): `ap show`
- Clear AP config if needed: `ap clear`
- Ensure proper device reset (not just WiFi off/on)
- Verify network is available and credentials are correct

### Connection Fails on Boot

The device will:
- Attempt to connect using saved credentials
- Display connection status on serial
- Fall back to IDLE mode if connection fails
- Retry on next boot

To fix:
```bash
station clear              # Clear bad credentials
station save SSID Pass     # Save correct credentials
reset                      # Restart device
```

### Can't Remember Password

If you can't remember the saved password:
```bash
station clear              # Clear old credentials
# Connect to network first
mode station
connect MyNetwork NewPassword
station save               # Save new credentials
```

## Integration with AP Mode

### Priority System
1. **AP Config**: If `ap save` with auto-start enabled → Boots as AP
2. **Station Config**: If `station save` with auto-connect → Boots as Station
3. **Both Saved**: AP takes priority

### Switching Modes
```bash
# Use AP mode primarily
ap save MyAP MyAPPass 6 yes
station clear

# Use Station mode primarily  
station save MyNetwork MyPass yes
ap clear

# Manual control (no auto-start/connect)
ap save MyAP MyAPPass 6 no
station save MyNetwork MyPass no
```

## Technical Implementation

The feature uses:
- **Preferences library**: Arduino wrapper for ESP-IDF NVS
- **StationConfig structure**: Stores SSID, password, and auto-connect flag
- **Validation functions**: Ensure configuration integrity
- **Boot integration**: Checks for saved config during `initializeWiFi()`
- **WiFi.psk()**: Retrieves password from active connection

## Common Use Cases

### Home Automation
```bash
# Save home network for IoT device
station save HomeNetwork HomePass2024 yes
```

### Development Device
```bash
# Save network but don't auto-connect during development
station save DevNetwork DevPass123 no
```

### Multiple Locations
```bash
# At location A
station save LocationA_WiFi PassA yes

# At location B
station clear
station save LocationB_WiFi PassB yes
```

### Guest Network Fallback
```bash
# Primary network
station save MainNetwork MainPass yes

# If needed, clear and use guest
station clear
station save GuestNetwork GuestPass yes
```

## See Also

- [AP Configuration Persistence](../configuration/AP_CONFIG_PERSISTENCE.md)
- [WiFi Manager Documentation](../WIFI_MANAGER.md)
- [Command Reference](../COMMAND_REFERENCE.md)
- [ESP32 NVS Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html)
