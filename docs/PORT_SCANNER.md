# Port Scanner - Network Security Auditing

## Overview

The Port Scanner is a comprehensive network security auditing tool that scans target devices to discover open TCP ports and identify running services. This feature enables network administrators and security professionals to assess network security posture and identify potential vulnerabilities.

## Features

### Scan Types

The Port Scanner supports four different scan modes to suit various needs:

#### 1. **Common Ports Scan (Fast)**

- **Ports Scanned**: 16 most commonly used ports
- **Duration**: ~10-30 seconds
- **Best For**: Quick security assessment
- **Ports Included**:
  - 21 (FTP)
  - 22 (SSH)
  - 23 (Telnet)
  - 25 (SMTP)
  - 53 (DNS)
  - 80 (HTTP)
  - 110 (POP3)
  - 143 (IMAP)
  - 443 (HTTPS)
  - 445 (SMB)
  - 3306 (MySQL)
  - 3389 (RDP)
  - 5900 (VNC)
  - 8080 (HTTP-Alt)
  - 8443 (HTTPS-Alt)
  - 9100 (Printer)

#### 2. **Well-Known Ports Scan**

- **Ports Scanned**: 1-1024 (IANA well-known ports)
- **Duration**: ~10-30 minutes
- **Best For**: Standard security audit
- **Coverage**: All standardized service ports

#### 3. **Custom Range Scan**

- **Ports Scanned**: User-defined start and end ports
- **Duration**: Varies based on range
- **Best For**: Targeted port analysis
- **Example**: Scan ports 8000-9000 for web services

#### 4. **All Ports Scan (Comprehensive)**

- **Ports Scanned**: 1-65535 (complete port range)
- **Duration**: ~8-12 hours
- **Best For**: Complete security assessment
- **Coverage**: Every possible TCP port

### Service Identification

The Port Scanner automatically identifies 25+ common services:

| Port  | Service         | Description                      |
| ----- | --------------- | -------------------------------- |
| 20    | FTP-Data        | File Transfer Protocol Data      |
| 21    | FTP             | File Transfer Protocol           |
| 22    | SSH             | Secure Shell                     |
| 23    | Telnet          | Telnet Protocol                  |
| 25    | SMTP            | Simple Mail Transfer Protocol    |
| 53    | DNS             | Domain Name System               |
| 80    | HTTP            | Hypertext Transfer Protocol      |
| 110   | POP3            | Post Office Protocol v3          |
| 143   | IMAP            | Internet Message Access Protocol |
| 443   | HTTPS           | HTTP Secure                      |
| 445   | SMB             | Server Message Block             |
| 465   | SMTPS           | SMTP Secure                      |
| 587   | SMTP-Submission | SMTP Mail Submission             |
| 993   | IMAPS           | IMAP Secure                      |
| 995   | POP3S           | POP3 Secure                      |
| 1433  | MSSQL           | Microsoft SQL Server             |
| 1521  | Oracle          | Oracle Database                  |
| 3306  | MySQL           | MySQL Database                   |
| 3389  | RDP             | Remote Desktop Protocol          |
| 5432  | PostgreSQL      | PostgreSQL Database              |
| 5900  | VNC             | Virtual Network Computing        |
| 6379  | Redis           | Redis Database                   |
| 8080  | HTTP-Proxy      | HTTP Alternative/Proxy           |
| 8443  | HTTPS-Alt       | HTTPS Alternative                |
| 9100  | Printer         | Network Printer                  |
| 27017 | MongoDB         | MongoDB Database                 |

## Web Interface

### Accessing the Port Scanner

Navigate to: **`http://<device-ip>/portscan`**

### Interface Components

#### 1. **Scan Configuration Section**

**Target IP Address:**

- Input field for target device IP
- Auto-populated with gateway IP by default
- Validates IP format

**Scan Type Selector:**

- Dropdown menu with four options
- Common Ports (recommended for quick scans)
- Well-Known (standard security audit)
- Custom Range (shows port range inputs)
- All Ports (comprehensive but slow)

**Custom Range Inputs** (shown when Custom Range selected):

- Start Port: Minimum port number (1-65535)
- End Port: Maximum port number (1-65535)
- Validation ensures end port ≥ start port

#### 2. **Scan Control Buttons**

**Start Scan Button:**

- Gradient purple styling
- Initiates port scan
- Disables during active scan
- Changes opacity when disabled

**Stop Scan Button:**

- Red styling
- Stops active scan
- Only enabled during scanning
- Immediately halts scan operation

#### 3. **Scan Status Display**

Real-time progress information:

- Animated progress bar (0-100%)
- Current port being scanned
- Ports scanned / Total ports
- Percentage completion
- Scan duration (when complete)

#### 4. **Results Display**

**Results Table Format:**
| Port | Service | Status |
|------|---------|--------|
| 80 | HTTP | OPEN |
| 443 | HTTPS | OPEN |
| 22 | SSH | OPEN |

**Result Features:**

- Port number
- Identified service name
- Status badge (green "OPEN")
- Sorted by port number
- Summary count at top

#### 5. **Important Notes Section**

Security warnings and guidelines:

- **Authorization**: Only scan devices you own or have permission to test
- **Network Impact**: Port scanning generates network traffic
- **Legal Disclaimer**: Unauthorized scanning may violate laws
- **Scan Duration**: Full scans can take considerable time
- **Best Practice**: Common ports scan recommended for quick assessment

## API Endpoints

### 1. Start Scan

```
GET /portscan/start?ip=<target>&type=<scan_type>[&start=<port>&end=<port>]
```

**Parameters:**

- `ip`: Target IP address (required)
- `type`: Scan type - `common`, `well-known`, `all`, or `range` (required)
- `start`: Start port number (required for `range` type)
- `end`: End port number (required for `range` type)

**Response:**

```json
{
  "success": true
}
```

**Example:**

```
/portscan/start?ip=192.168.1.1&type=common
/portscan/start?ip=192.168.1.100&type=range&start=8000&end=9000
```

### 2. Stop Scan

```
GET /portscan/stop
```

**Response:**

```json
{
  "success": true
}
```

### 3. Get Scan Status

```
GET /portscan/status
```

**Response:**

```json
{
  "state": "running",
  "targetIP": "192.168.1.1",
  "totalPorts": 16,
  "portsScanned": 8,
  "currentPort": 9,
  "openPorts": 3,
  "closedPorts": 5,
  "progress": 50,
  "duration": 15,
  "ports": [
    {
      "port": 80,
      "service": "HTTP"
    },
    {
      "port": 443,
      "service": "HTTPS"
    }
  ]
}
```

**State Values:**

- `idle`: No scan running
- `running`: Scan in progress
- `completed`: Scan finished
- `error`: Scan encountered error

### 4. Gateway IP Helper

```
GET /portscan/api?gateway=1
```

**Response:**

```json
{
  "gateway": "192.168.1.1"
}
```

## Usage Examples

### Quick Security Check

**Scenario**: Quickly check if common services are exposed on gateway

**Steps:**

1. Navigate to `/portscan`
2. Leave default gateway IP
3. Select "Common Ports (Fast)"
4. Click "Start Scan"
5. Wait ~30 seconds for results
6. Review open ports

**Expected Results:**

- HTTP (80) - OPEN
- HTTPS (443) - OPEN
- SSH (22) - OPEN (if remote management enabled)

### Custom Port Range Scan

**Scenario**: Scan web service ports on a server

**Steps:**

1. Navigate to `/portscan`
2. Enter server IP: `192.168.1.50`
3. Select "Custom Range"
4. Start Port: `8000`
5. End Port: `9000`
6. Click "Start Scan"
7. Wait for completion
8. Review results

**Use Case**: Finding non-standard web services

### Comprehensive Security Audit

**Scenario**: Complete port scan for security assessment

**Steps:**

1. Navigate to `/portscan`
2. Enter target IP: `192.168.1.100`
3. Select "All Ports (1-65535)"
4. Click "Start Scan"
5. **Wait 8-12 hours** for completion
6. Review comprehensive results

**Best Practice**: Run overnight, review results next day

## Security Considerations

### Legal and Ethical Use

⚠️ **IMPORTANT**: Port scanning without authorization may be illegal

**Legal Guidelines:**

- Only scan devices you own
- Obtain written permission for third-party devices
- Comply with organizational security policies
- Check local laws regarding network scanning
- Do not scan public internet without authorization

### Network Impact

Port scanning can affect network performance:

**Considerations:**

- Generates TCP connection attempts
- May trigger IDS/IPS alerts
- Can cause temporary service disruptions
- May be logged by target systems
- Could violate terms of service

**Best Practices:**

- Scan during maintenance windows
- Use Common Ports scan for quick checks
- Avoid All Ports scan on production systems
- Monitor network impact during scans
- Inform IT staff before scanning

### Responsible Disclosure

If vulnerabilities are discovered:

1. Document findings professionally
2. Report to system owner/administrator
3. Provide details: port, service, risk level
4. Allow reasonable time for remediation
5. Do not publicly disclose before fix

## Technical Implementation

### Scan Methodology

**TCP Connection Test:**

```cpp
WiFiClient client;
client.setTimeout(1000); // 1 second timeout
bool isOpen = client.connect(targetIP, port);
client.stop();
```

**Background Scanning:**

- Sequential port checking
- Non-blocking main loop
- 10ms delay between ports
- Progress tracking
- State management

### Performance Characteristics

| Scan Type  | Ports    | Avg Duration | Network Load |
| ---------- | -------- | ------------ | ------------ |
| Common     | 16       | 20s          | Low          |
| Well-Known | 1024     | 15min        | Medium       |
| Custom     | Variable | Variable     | Variable     |
| All Ports  | 65535    | 10hrs        | High         |

**Timeout Configuration:**

- Default: 1000ms (1 second)
- Adjustable in code
- Faster timeout = quicker scan
- Slower timeout = fewer false negatives

## Integration

### Analysis Dashboard

The Port Scanner is integrated into the Analysis Dashboard:

**Access Points:**

- Main Dashboard: `/analysis`
- Port Scanner Card: Purple gradient
- Quick Actions button
- Navigation dropdown: "🔒 Port Scanner"

**Status Display:**

- Last scan results
- Number of open ports found
- Quick access button

## Troubleshooting

### Scan Not Starting

**Symptoms:**

- "Start Scan" button does nothing
- Error message displayed

**Solutions:**

1. Check WiFi connection
2. Verify target IP is valid
3. Ensure no other scan running
4. Check browser console for errors

### No Open Ports Found

**Symptoms:**

- Scan completes successfully
- Results show 0 open ports

**Possible Causes:**

1. **Firewall Blocking**: Target has firewall enabled
2. **Wrong IP**: Target IP incorrect or offline
3. **Network Isolation**: Device on different network segment
4. **Services Disabled**: No services running on target

**Debugging:**

1. Ping target IP to verify connectivity
2. Try scanning known-good device (router)
3. Check network configuration
4. Verify device is powered on

### Scan Timeout/Slow

**Symptoms:**

- Scan takes very long
- Progress bar not moving

**Solutions:**

1. **Use Common Ports**: Faster than full scan
2. **Check Network**: Slow network = slow scan
3. **Reduce Range**: Smaller port range = faster
4. **Target Reachability**: Ensure target responds quickly

## Best Practices

### Security Assessment Workflow

1. **Planning Phase**:

   - Get authorization
   - Document scope
   - Choose scan type
   - Schedule scan window

2. **Execution Phase**:

   - Start with Common Ports scan
   - Document open ports
   - Identify unexpected services
   - Note security concerns

3. **Analysis Phase**:

   - Compare against baseline
   - Identify unnecessary services
   - Assess risk level
   - Create remediation plan

4. **Remediation Phase**:
   - Close unnecessary ports
   - Update firewall rules
   - Disable unused services
   - Verify changes with rescan

### Regular Scanning Schedule

**Recommended Frequency:**

- **Critical Systems**: Weekly
- **Standard Systems**: Monthly
- **Low-Risk Systems**: Quarterly
- **After Changes**: Immediate

## Conclusion

The Port Scanner provides essential network security auditing capabilities directly from the ESP32 device. Use it responsibly, ethically, and in compliance with all applicable laws and policies to maintain secure network infrastructure.

For more information, see:

- [Analysis Dashboard Documentation](ANALYSIS_DASHBOARD.md)
- [Web Configuration Guide](WEB_CONFIGURATION.md)
- [Security Best Practices](SECURITY_BEST_PRACTICES.md)
