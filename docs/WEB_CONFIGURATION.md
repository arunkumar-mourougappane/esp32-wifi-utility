# Web Configuration Interface

## Overview

The ESP32 WiFi Utility includes a web-based configuration interface accessible through any web browser. This interface allows you to manage both Access Point (AP) and Station mode configurations without needing serial console access.

## Features

- **Browser-Based Configuration**: Configure device from any web browser
- **AP Configuration**: Set SSID, password, channel, and auto-start
- **Station Configuration**: Save WiFi credentials and auto-connect settings
- **Password Security**: Passwords are not displayed, only prompted for updates
- **Real-Time Status**: View current saved configurations
- **Easy Clear**: Remove saved configurations with one click
- **Mobile Friendly**: Responsive design works on phones and tablets

## Accessing the Web Interface

### From AP Mode

1. Connect your device (phone/laptop) to the ESP32's WiFi network
2. Open browser and navigate to: `http://192.168.4.1`
3. Click on **⚙️ Config** in the navigation menu

### From Station Mode

1. Ensure ESP32 is connected to your WiFi network
2. Check serial output for the device IP address, or:
   - Type `status` in serial console to see IP
   - Check your router's DHCP client list
3. Open browser and navigate to: `http://[device-ip]`
4. Click on **⚙️ Config** in the navigation menu

## Configuration Page

The configuration page is divided into sections:

### 🔄 Quick Mode Switch

At the top of the configuration page, you'll find a quick mode switcher that allows you to instantly change between Access Point and Station modes without navigating through serial commands.

**Features:**
- **Current Mode Display**: Shows whether device is in AP, Station, or IDLE mode
- **One-Click Switch**: Two buttons for instant mode switching
- **Status Feedback**: Real-time feedback on mode switching
- **Automatic Configuration**: Uses saved configurations when available

**Buttons:**
- **📡 Switch to Access Point**: Activates AP mode
  - Uses saved AP configuration if available
  - Falls back to default configuration if no saved config
- **📶 Switch to Station**: Activates Station mode
  - Requires saved Station configuration
  - Shows error if no configuration saved

**Usage:**
1. View current mode at top of toggle section
2. Click desired mode button
3. Wait for confirmation message
4. Mode switches immediately
5. No reboot required

**Behavior:**
- **AP Mode**: Uses saved SSID, password, and channel (if configured)
- **Station Mode**: Connects to saved WiFi network (requires configuration)
- **Error Handling**: Clear messages if configuration missing

### 📡 Access Point Configuration

Manage settings for when device operates as WiFi Access Point.

**Form Fields:**

- **SSID** (required)
  - Network name for the AP
  - 1-32 characters
  - Current value pre-filled if configuration exists

- **Password** (optional when updating)
  - WPA2 password for the AP
  - 8-63 characters
  - **Leave empty to keep current password**
  - Only enter new password when changing

- **Channel**
  - WiFi channel (1-13)
  - Dropdown selection
  - Default: Channel 1
  - Current value selected if configuration exists

- **Auto-start on boot**
  - Checkbox to enable/disable
  - When checked: Device boots as AP automatically
  - When unchecked: Manual start required
  - Default: Checked

**Actions:**

- **💾 Save AP Config**: Save configuration to non-volatile storage
- **🗑️ Clear Config**: Remove saved AP configuration (only shown if config exists)

**After Saving:**

Upon successful save, you'll see a confirmation page with:
- Summary of saved settings
- **🔄 Reboot Device** button - Opens modal to reboot immediately
- **← Back to Configuration** link - Return without rebooting

**Status Display:**

If configuration is saved, displays:
- ✓ Saved Configuration
- SSID: [saved network name]
- Channel: [saved channel]
- Auto-start: Yes/No

### 📶 Station Configuration

Manage WiFi network credentials for Station mode.

**Form Fields:**

- **WiFi Network SSID** (required)
  - Name of WiFi network to connect to
  - 1-32 characters
  - Current value pre-filled if configuration exists

- **WiFi Password** (optional when updating)
  - Password for the WiFi network
  - 0-63 characters (empty for open networks)
  - **Leave empty to keep current password**
  - Only enter new password when changing

- **Auto-connect on boot**
  - Checkbox to enable/disable
  - When checked: Device connects automatically on boot
  - When unchecked: Manual connection required
  - Default: Checked

**Actions:**

- **💾 Save Station Config**: Save configuration to non-volatile storage
- **🗑️ Clear Config**: Remove saved Station configuration (only shown if config exists)

**After Saving:**

Upon successful save, you'll see a confirmation page with:
- Summary of saved settings
- **🔄 Reboot Device** button - Opens modal to reboot immediately
- **← Back to Configuration** link - Return without rebooting

**Status Display:**

If configuration is saved, displays:
- ✓ Saved Configuration
- SSID: [saved network name]
- Auto-connect: Yes/No

**Priority Warning:**

⚠️ **Priority:** AP auto-start takes priority over Station auto-connect.
If both are enabled, device will boot as AP.

## Reboot Modal

After saving a configuration, you can immediately reboot the device:

### Features

- **Interactive Modal Dialog**: Clean popup interface
- **3-Second Countdown**: Visual countdown before reboot
- **Progress Feedback**: Shows reboot status
- **Auto-Close**: Page closes/redirects after reboot
- **Cancel Option**: Choose to reboot later

### Using the Reboot Modal

1. Save AP or Station configuration
2. On confirmation page, click **🔄 Reboot Device**
3. Modal appears with two options:
   - **Yes, Reboot Now**: Initiates reboot with countdown
   - **No, Later**: Closes modal, reboot manually later
4. If "Yes" selected:
   - 3-second countdown displays
   - Device reboots
   - "Rebooting..." message shows
   - Wait 10 seconds
   - Page closes/redirects automatically
5. Reconnect to device after reboot

## Usage Examples

### Example 1: Configure Device as WiFi Hotspot

**Goal**: Make device boot as WiFi Access Point

**Steps:**

1. Access web interface at `http://192.168.4.1`
2. Navigate to **⚙️ Config**
3. **Quick Mode Switch**: Click **📡 Switch to Access Point** to activate immediately
4. In **Access Point Configuration** section:
   - SSID: `MyDeviceAP`
   - Password: `SecurePass123`
   - Channel: `6`
   - Auto-start: ✓ Checked
5. Click **💾 Save AP Config**
6. On confirmation page:
   - Review saved settings
   - Click **🔄 Reboot Device** (optional - mode already active)
   - In modal, click **Yes, Reboot Now**
   - Wait for countdown and reboot
7. After reboot (10 seconds):
   - Device boots as AP with saved settings
   - Reconnect to new SSID if changed

**Result**: Device boots as AP with SSID "MyDeviceAP" on channel 6

---

### Example 2: Configure Device to Connect to Home WiFi

**Goal**: Make device auto-connect to home network

**Steps:**

1. Access web interface
2. Navigate to **⚙️ Config**
3. In **Station Configuration** section:
   - WiFi Network SSID: `HomeNetwork`
   - WiFi Password: `HomePassword123`
   - Auto-connect: ✓ Checked
4. Click **💾 Save Station Config**
5. **Quick Mode Switch**: Click **📶 Switch to Station** to connect immediately
6. On confirmation page:
   - Review saved settings
   - Wait for connection
   - Find new IP address to access web interface (check serial or router)

**Result**: Device connects to "HomeNetwork" automatically on boot

---

### Example 3: Switch Between Modes On-The-Fly

**Goal**: Quickly toggle between AP and Station without rebooting

**Steps:**

1. Device currently in AP mode
2. Access web interface at `http://192.168.4.1`
3. Navigate to **⚙️ Config**
4. See "Current Mode: AP"
5. Click **📶 Switch to Station**
6. Status shows "✓ Connecting to Station mode..."
7. Device connects to saved WiFi network
8. Find new IP and reconnect
9. To switch back: Click **📡 Switch to Access Point**
10. Immediately back in AP mode

**Result**: Seamless switching without reboots

---

### Example 3: Update Password Without Changing SSID

**Goal**: Change AP password but keep same SSID

**Steps:**

1. Access web interface
2. Navigate to **⚙️ Config**
3. In **Access Point Configuration** section:
   - SSID: Leave as is (pre-filled)
   - Password: Enter new password
   - Channel: Leave as is (pre-selected)
   - Auto-start: Leave checked
4. Click **💾 Save AP Config**

**Result**: Password updated, all other settings preserved

---

### Example 4: Disable Auto-Start/Connect

**Goal**: Save configurations but disable automatic boot behavior

**Steps:**

1. Access web interface
2. Navigate to **⚙️ Config**
3. In **Access Point Configuration** section:
   - Keep current SSID (pre-filled)
   - Password: Leave empty (keep current)
   - Auto-start: ✗ Uncheck
4. Click **💾 Save AP Config**
5. In **Station Configuration** section:
   - Keep current SSID (pre-filled)
   - Password: Leave empty (keep current)
   - Auto-connect: ✗ Uncheck
6. Click **💾 Save Station Config**

**Result**: Configurations saved but device boots in IDLE mode (manual control)

## Password Security

### Not Exposed in Web Interface

- **Saved passwords are NEVER displayed** in the web interface
- Password fields are always empty when viewing existing configurations
- This prevents passwords from being visible on shared/public screens

### Updating Passwords

To update a password:

1. **Leave field empty** to keep current password unchanged
2. **Enter new password** to change/update password

### Best Practices

✓ Use strong passwords (8+ characters, mixed case, numbers, symbols)
✓ Don't share web interface access on untrusted networks
✓ Clear configurations before disposing of devices
✓ Change passwords regularly for security

⚠ **Security Note**: While passwords are not displayed in the web interface,
they are stored unencrypted in NVS and can be retrieved via serial console.
Physical access to device = potential access to credentials.

## Configuration Workflow

### Recommended Setup Process

1. **Initial Setup**
   - Connect to device via default AP
   - Access web interface at `http://192.168.4.1`
   - Configure desired mode (AP or Station)
   - Save configuration
   - Restart device

2. **Verification**
   - Device should boot into configured mode
   - Access web interface at new IP/address
   - Verify settings in **⚙️ Config** page

3. **Updates**
   - Make changes as needed
   - Leave password fields empty if not changing
   - Save and restart

### Factory Reset via Web

To completely reset configurations:

1. Navigate to **⚙️ Config**
2. Click **🗑️ Clear Config** for AP section
3. Click **🗑️ Clear Config** for Station section  
4. Restart device

Device will boot in IDLE mode with no saved configurations.

## Troubleshooting

### Can't Access Web Interface

**Problem**: Browser can't connect

**Solutions**:
- Check WiFi connection to device
- Verify correct IP address
- In AP mode, try: `http://192.168.4.1`
- Check if web server is started (serial: `webserver status`)
- Restart web server (serial: `webserver stop` then `webserver start`)

### Configuration Not Saving

**Problem**: Save button doesn't work or shows error

**Solutions**:
- Check SSID length (1-32 characters)
- Check password length (8-63 for AP, 0-63 for Station)
- Ensure no special characters causing issues
- Check serial output for error messages
- Try clearing existing config first, then save new one

### Device Not Auto-Starting/Connecting

**Problem**: Saved config but device doesn't auto-boot

**Solutions**:
- Verify auto-start/auto-connect checkbox was checked
- Check with serial commands:
  - `ap show` - view AP config
  - `station show` - view Station config
- Remember: AP takes priority over Station
- Ensure proper device restart (not just WiFi off/on)

### Password Field Confusion

**Problem**: Not sure if password is saved correctly

**Solutions**:
- Leave password field empty = keeps current password
- Enter new password = changes password
- If unsure, clear config and re-enter all details
- Use serial commands to verify:
  - `ap show` - shows AP config (password visible)
  - `station show` - shows Station config (password masked)

## Integration with Serial Commands

Web interface and serial commands work together:

### View Configurations

**Web**: Navigate to **⚙️ Config** page
**Serial**: `ap show` and `station show`

### Save Configurations

**Web**: Use save buttons in config page
**Serial**: `ap save ...` and `station save ...`

### Clear Configurations

**Web**: Click **🗑️ Clear Config** buttons
**Serial**: `ap clear` and `station clear`

### Check Status

**Web**: Navigate to **📊 Status** page
**Serial**: `status` command

Both methods achieve the same result - use whichever is more convenient!

## Mobile Access

The web interface is fully responsive and works well on mobile devices:

### Tips for Mobile Use

1. **Portrait Mode**: Works best for form filling
2. **Landscape Mode**: Better for viewing navigation menu
3. **Saved Credentials**: Use browser's password manager for convenience
4. **Bookmarks**: Bookmark the config page for quick access
5. **Desktop Mode**: Switch to desktop site if needed (browser option)

### Mobile Workflow

1. Connect phone to ESP32 WiFi
2. Open browser
3. Go to `http://192.168.4.1`
4. Tap **⚙️ Config**
5. Fill forms
6. Save configurations
7. Disconnect and restart device

## Advanced Usage

### Scripted Configuration

For automated setup, you can POST to the configuration endpoints:

```bash
# Save AP configuration
curl -X POST http://192.168.4.1/config/ap \
  -d "ssid=MyAP&password=MyPass123&channel=6&autostart=1"

# Save Station configuration
curl -X POST http://192.168.4.1/config/station \
  -d "ssid=HomeWiFi&password=HomePass&autoconnect=1"

# Clear configurations
curl -X POST http://192.168.4.1/config/clear -d "type=ap"
curl -X POST http://192.168.4.1/config/clear -d "type=station"
```

### API Endpoints

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/config` | GET | Display configuration page |
| `/config/ap` | POST | Save AP configuration |
| `/config/station` | POST | Save Station configuration |
| `/config/clear` | POST | Clear configuration (specify type) |
| `/mode/switch` | POST | Switch between AP and Station modes |
| `/reboot` | POST | Reboot device |

## See Also

- [Configuration System Overview](CONFIGURATION_SYSTEM.md)
- [AP Configuration Guide](AP_CONFIG_PERSISTENCE.md)
- [Station Configuration Guide](STATION_CONFIG_PERSISTENCE.md)
- [Serial Command Reference](../README.md)
