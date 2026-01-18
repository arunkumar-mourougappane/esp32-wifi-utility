# Access Point Configuration Persistence

## Overview

The ESP32 WiFi Utility now supports saving Access Point configuration to non-volatile storage (NVS). When a configuration is saved with auto-start enabled, the device will automatically launch in Access Point mode on boot.

## Features

- **Save AP Configuration**: Store SSID, password, and channel settings
- **Auto-Start on Boot**: Optionally configure the device to start in AP mode automatically
- **Persistent Storage**: Configuration survives power cycles and resets
- **Easy Management**: Simple commands to save, load, show, and clear configurations

## Commands

### Save Current AP Configuration

```
ap save
```

Saves the current Access Point configuration (must be in AP mode). The device will auto-start in AP mode on next boot.

### Save Custom AP Configuration

```
ap save <ssid> <password> [channel] [autostart]
```

**Parameters:**
- `ssid`: WiFi SSID (1-32 characters)
- `password`: WiFi password (8-63 characters)
- `channel`: WiFi channel 1-13 (optional, default: 1)
- `autostart`: yes/no or true/false (optional, default: yes)

**Examples:**
```
ap save MyNetwork MyPass123
ap save MyNetwork MyPass123 6
ap save MyNetwork MyPass123 6 yes
ap save MyNetwork MyPass123 1 no
ap save "My Network" "My Password 123" 11 yes
```

### Load Saved Configuration

```
ap load
```

Loads the saved AP configuration and applies it to current settings. Use `mode ap` afterwards to start the AP with loaded settings.

### Show Configuration

```
ap show
```

Displays the saved AP configuration, or the default configuration if none is saved.

### Clear Saved Configuration

```
ap clear
```

Removes the saved AP configuration. The device will not auto-start AP mode on next boot.

## Usage Examples

### Save Current AP and Enable Auto-Start

1. Start AP mode:
   ```
   mode ap MyNetwork SecurePass123
   ```

2. Save the configuration:
   ```
   ap save
   ```

3. The device will now auto-start with these settings on reboot

### Configure Custom AP with Specific Channel

```
ap save GuestNetwork GuestPass2024 6 yes
```

This creates an AP configuration:
- SSID: GuestNetwork
- Password: GuestPass2024
- Channel: 6
- Auto-start: Enabled

### Disable Auto-Start

```
ap clear
```

The device will return to idle mode on boot instead of starting the AP.

## Configuration Details

### Storage

Configuration is stored in ESP32 NVS (Non-Volatile Storage) in the `ap_config` namespace. The data persists across:
- Power cycles
- Software resets
- Firmware updates (unless NVS is erased)

### Validation

The system validates:
- **SSID length**: 1-32 characters
- **Password length**: 8-63 characters (WPA2 requirement)
- **Channel range**: 1-13 (2.4 GHz WiFi channels)

Invalid configurations will be rejected with an error message.

### Default Behavior

- **No saved config**: Device starts in IDLE mode, uses default AP settings if AP mode is manually started
- **Saved config with auto-start**: Device automatically starts in AP mode with saved settings
- **Saved config without auto-start**: Device starts in IDLE mode but uses saved settings when AP mode is manually started

## Technical Implementation

The feature uses:
- **Preferences library**: Arduino wrapper for ESP-IDF NVS
- **APConfig structure**: Stores SSID, password, channel, and auto-start flag
- **Validation functions**: Ensure configuration integrity
- **Boot integration**: Checks for saved config during `initializeWiFi()`

## Troubleshooting

### Configuration Not Saving

- Ensure SSID is 1-32 characters
- Ensure password is 8-63 characters
- Check serial output for error messages
- Try `ap clear` then save again

### Auto-Start Not Working

- Verify configuration with `ap show`
- Check that auto-start flag is set to "Yes"
- Ensure device is being properly reset (not just WiFi off/on)

### Loading Fails

- Check if configuration exists with `ap show`
- Verify NVS is not corrupted (try `ap clear` and save new config)
- Check serial output for specific error messages

## Best Practices

1. **Test Before Saving**: Start AP mode manually first to verify settings work
2. **Document Passwords**: Keep a record of saved passwords
3. **Use Descriptive SSIDs**: Make it easy to identify your device
4. **Choose Clear Channels**: Use `channel scan` to find less congested channels
5. **Secure Passwords**: Use strong passwords with mixed characters

## Security Considerations

- Passwords are stored in NVS and can be retrieved via serial commands
- Physical access to the device allows configuration viewing
- Use strong passwords for production deployments
- Consider encryption requirements for sensitive deployments

## See Also

- [WiFi Manager Documentation](../WIFI_MANAGER.md)
- [Command Reference](../COMMAND_REFERENCE.md)
- [ESP32 NVS Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html)
