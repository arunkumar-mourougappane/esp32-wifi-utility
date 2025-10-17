#include "web_server.h"

#ifdef USE_WEBSERVER  // Web server available when enabled

#include "wifi_manager.h"
#include "ap_manager.h"
#include "channel_analyzer.h"
#include "iperf_manager.h"
#include "latency_analyzer.h"
#include <qrcode.h>

// Web server instance
WebServer* webServer = nullptr;
bool webServerEnabled = false;

// ==========================================
// HELPER FUNCTIONS
// ==========================================
String getDeviceName() {
    // Get chip model (e.g., "ESP32", "ESP32-S3", "ESP32-C3")
    String chipModel = String(ESP.getChipModel());
    
    // Check for specific board types
    #ifdef ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT
        return "Feather " + chipModel + " TFT";
    #elif defined(ARDUINO_FEATHER_ESP32)
        return "Feather " + chipModel;
    #else
        return chipModel + " Dev Module";
    #endif
}

String getVersionString() {
    #ifdef VERSION
        // VERSION is defined as a string literal in build flags
        #define STRINGIFY(x) #x
        #define TOSTRING(x) STRINGIFY(x)
        return String(TOSTRING(VERSION));
    #else
        return "3.0.0";
    #endif
}

// HTML page styling - Minified to reduce memory
const char* HTML_HEADER = R"rawliteral(
<!DOCTYPE html>
<html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 WiFi</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#333;padding:20px;min-height:100vh}
.container{max-width:1200px;margin:0 auto;background:#fff;border-radius:15px;box-shadow:0 10px 40px rgba(0,0,0,.2);padding:30px}
h1{color:#667eea;margin-bottom:10px;font-size:2em}
h2{color:# 764ba2;margin:30px 0 15px;font-size:1.5em;border-bottom:2px solid #667eea;padding-bottom:10px}
.header{text-align:center;margin-bottom:30px}
.badge{display:inline-block;padding:8px 15px;border-radius:20px;font-weight:bold;margin:5px;font-size:.9em}
.badge.success{background:#10b981;color:#fff}
.badge.warning{background:#f59e0b;color:#fff}
.badge.info{background:#3b82f6;color:#fff}
.badge.danger{background:#ef4444;color:#fff}
.stat-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:20px;margin:20px 0}
.stat-card{background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;padding:20px;border-radius:10px;text-align:center}
.stat-value{font-size:2em;font-weight:bold;margin:10px 0}
.stat-label{font-size:.9em;opacity:.9}
.network-list{list-style:none;margin:20px 0}
.network-item{background:#f8f9fa;padding:15px;margin:10px 0;border-radius:8px;border-left:4px solid #667eea;display:flex;justify-content:space-between;align-items:center}
.network-info{flex-grow:1}
.network-name{font-weight:bold;font-size:1.1em;margin-bottom:5px}
.network-details{color:#666;font-size:.9em}
.signal-strength{font-size:1.5em;margin-left:20px}
.footer{text-align:center;margin-top:30px;padding-top:20px;border-top:1px solid #e0e0e0;color:#666;font-size:.9em}
.nav{display:flex;gap:10px;justify-content:center;flex-wrap:wrap;margin:20px 0;position:relative}
.nav a{text-decoration:none;padding:10px 20px;background:#667eea;color:#fff;border-radius:5px;transition:all .3s}
.nav a:hover{background:#764ba2;transform:translateY(-2px);box-shadow:0 4px 8px rgba(0,0,0,.2)}
.dropdown{position:relative;display:inline-block}
.dropdown-content{display:none;position:absolute;background-color:#667eea;min-width:200px;box-shadow:0 8px 16px rgba(0,0,0,.3);z-index:1;border-radius:5px;margin-top:5px}
.dropdown-content a{color:#fff;padding:12px 16px;text-decoration:none;display:block;border-radius:0;margin:0}
.dropdown-content a:first-child{border-radius:5px 5px 0 0}
.dropdown-content a:last-child{border-radius:0 0 5px 5px}
.dropdown-content a:hover{background-color:#764ba2;transform:none}
.dropdown:hover .dropdown-content{display:block}
.dropdown>a::after{content:' ▼';font-size:.8em}
.progress-backdrop{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,.7);z-index:9999;justify-content:center;align-items:center}
.progress-container{background:#fff;padding:30px;border-radius:15px;box-shadow:0 10px 40px rgba(0,0,0,.3);text-align:center;min-width:300px;max-width:500px}
.progress-title{color:#667eea;font-size:1.5em;margin-bottom:20px;font-weight:bold}
.spinner{border:4px solid #f3f3f3;border-top:4px solid #667eea;border-radius:50%;width:40px;height:40px;animation:spin 1s linear infinite;margin:20px auto}
@keyframes spin{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}
@media(max-width:768px){.container{padding:15px}h1{font-size:1.5em}.stat-grid{grid-template-columns:1fr}.dropdown-content{position:relative;margin-top:5px}.dropdown:hover .dropdown-content,.dropdown:focus-within .dropdown-content{display:block}}
</style>
<script>
function showProgress(t,m){const b=document.getElementById('progressBackdrop'),ti=document.getElementById('progressTitle'),me=document.getElementById('progressMessage');if(ti)ti.textContent=t;if(me)me.textContent=m;if(b)b.style.display='flex'}
function hideProgress(){const b=document.getElementById('progressBackdrop');if(b)b.style.display='none'}
function startScan(u,t,m){showProgress(t,m);window.location.href=u}
</script>
</head>
<body>
<div id="progressBackdrop" class="progress-backdrop"><div class="progress-container"><div id="progressTitle" class="progress-title">Scanning...</div><div class="spinner"></div><div id="progressMessage" class="progress-message">Please wait...</div></div></div>
<div class="container">
)rawliteral";

// Generate dynamic HTML footer
String generateHtmlFooter() {
    String footer = "<div class=\"footer\"><p>🚀 ESP32 WiFi v";
    footer += getVersionString();
    footer += " | ";
    footer += getDeviceName();
    footer += "</p></div></div></body></html>";
    return footer;
}

// Generate common navigation menu
String generateNav() {
    return "<div class=\"nav\"><div><a href=\"/\">🏠 Home</a></div><div><a href=\"/status\">📊 Status</a></div><div><a href=\"/scan\">🔍 Scan</a></div><div class=\"dropdown\"><a href=\"/analysis\">🔬 Analysis</a><div class=\"dropdown-content\"><a href=\"/analysis\">📊 Dashboard</a><a href=\"/iperf\">⚡ iPerf</a><a href=\"/latency\">📉 Latency</a><a href=\"/channel\">📡 Channel</a></div></div></div>";
}

// ==========================================
// INITIALIZATION
// ==========================================
void initializeWebServer() {
    webServerEnabled = false;
    webServer = nullptr;
}

// ==========================================
// START/STOP WEB SERVER
// ==========================================
bool startWebServer() {
    if (webServer != nullptr) {
        Serial.println("⚠️  Web server already running");
        return false;
    }

    // Check if we're in AP mode or connected to WiFi
    if (currentMode != MODE_AP && WiFi.status() != WL_CONNECTED) {
        Serial.println("❌ Web server requires WiFi connection or AP mode");
        return false;
    }

    Serial.println("🌐 Starting web server...");

    // Create web server instance
    webServer = new WebServer(WEB_SERVER_PORT);

    // Register route handlers
    webServer->on("/", handleRoot);
    webServer->on("/status", handleStatus);
    webServer->on("/scan", handleScan);
    webServer->on("/analysis", handleNetworkAnalysis);
    webServer->on("/channel", handleChannelAnalysis);
    webServer->on("/channel/scan", handleChannelScan);
    webServer->on("/latency", handleLatency);
    webServer->on("/latency/start", handleLatencyStart);
    webServer->on("/latency/stop", handleLatencyStop);
    webServer->on("/iperf", handleIperf);
    webServer->on("/iperf/start", handleIperfStart);
    webServer->on("/iperf/stop", handleIperfStop);
    webServer->on("/iperf/results", handleIperfResults);
    webServer->onNotFound(handleNotFound);

    // Start the server
    webServer->begin();
    webServerEnabled = true;

    String url = getWebServerURL();
    Serial.println("✅ Web server started successfully");
    Serial.println("📡 Access at: " + url);
    
    return true;
}

void stopWebServer() {
    if (webServer == nullptr) {
        Serial.println("⚠️  Web server not running");
        return;
    }

    Serial.println("🛑 Stopping web server...");
    webServer->stop();
    delete webServer;
    webServer = nullptr;
    webServerEnabled = false;
    Serial.println("✅ Web server stopped");
}

// ==========================================
// HANDLE WEB SERVER REQUESTS
// ==========================================
void handleWebServerRequests() {
    if (webServer != nullptr && webServerEnabled) {
        webServer->handleClient();
    }
}

// ==========================================
// MONITOR AND AUTO-RESTART WEB SERVER
// ==========================================
void monitorWebServerState() {
    static bool wasConnected = false;
    static WiFiMode lastMode = MODE_IDLE;
    
    bool isConnected = (WiFi.status() == WL_CONNECTED);
    bool shouldRun = (currentMode == MODE_AP) || isConnected;
    bool isRunning = isWebServerRunning();
    
    // Detect mode change
    bool modeChanged = (currentMode != lastMode);
    
    // Auto-start web server if conditions are met and it's not running
    if (shouldRun && !isRunning) {
        // Only auto-start if:
        // 1. We just switched to AP mode, OR
        // 2. We just connected to WiFi in Station mode
        bool justSwitchedToAP = (currentMode == MODE_AP && modeChanged);
        bool justConnected = (isConnected && !wasConnected && currentMode == MODE_STATION);
        
        if (justSwitchedToAP || justConnected) {
            Serial.println("🔄 Auto-starting web server...");
            if (startWebServer()) {
                Serial.println("✅ Web server auto-started at: " + getWebServerURL());
            }
        }
    }
    
    // Auto-stop web server if conditions are no longer met
    if (!shouldRun && isRunning) {
        // Only auto-stop if we disconnected in Station mode
        bool disconnectedInStation = (!isConnected && wasConnected && currentMode == MODE_STATION);
        
        if (disconnectedInStation) {
            Serial.println("🔄 Auto-stopping web server (WiFi disconnected)...");
            stopWebServer();
        }
    }
    
    // Update state tracking
    wasConnected = isConnected;
    lastMode = currentMode;
}

// ==========================================
// WEB SERVER STATUS
// ==========================================
bool isWebServerRunning() {
    return webServerEnabled && webServer != nullptr;
}

// ==========================================
// QR CODE GENERATION FOR WEB
// ==========================================
String generateQRCodeSVG(const String& ssid, const String& password, const String& security = "WPA") {
    // Create WiFi QR code string format: WIFI:T:<security>;S:<SSID>;P:<password>;;
    String qrData = "WIFI:T:" + security + ";S:" + ssid + ";P:" + password + ";;";
    
    // Create QR code object
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];  // Version 3 should be sufficient for WiFi data
    
    // Generate QR code
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrData.c_str());
    
    // Generate SVG
    String svg = "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 " + String(qrcode.size) + " " + String(qrcode.size) + "' shape-rendering='crispEdges'>";
    svg += "<rect width='" + String(qrcode.size) + "' height='" + String(qrcode.size) + "' fill='#ffffff'/>";
    
    // Draw QR code modules
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                svg += "<rect x='" + String(x) + "' y='" + String(y) + "' width='1' height='1' fill='#000000'/>";
            }
        }
    }
    
    svg += "</svg>";
    return svg;
}

String getWebServerURL() {
    if (currentMode == MODE_AP) {
        return "http://" + WiFi.softAPIP().toString();
    } else if (WiFi.status() == WL_CONNECTED) {
        return "http://" + WiFi.localIP().toString();
    }
    return "http://not-connected";
}

// ==========================================
// PAGE HANDLERS
// ==========================================

void handleRoot() {
    String html = HTML_HEADER;
    
    html += "<div class=\"header\"><h1>🚀 ESP32 WiFi</h1><p>Network Analysis & Testing</p><div><span class=\"badge info\">";
    html += getDeviceName();
    html += "</span>";
    
    if (currentMode == MODE_AP) {
        html += "<span class=\"badge success\">Access Point Mode</span>";
    } else if (WiFi.status() == WL_CONNECTED) {
        html += "<span class=\"badge success\">Connected</span>";
    } else {
        html += "<span class=\"badge warning\">Station Mode</span>";
    }
    
    html += "</div></div>";
    
    // Compact navigation
    html += generateNav();
    
    html += "<h2>📊 Stats</h2><div class=\"stat-grid\"><div class=\"stat-card\"><div class=\"stat-label\">Mode</div><div class=\"stat-value\">";
    
    switch (currentMode) {
        case MODE_IDLE: html += "IDLE"; break;
        case MODE_STATION: html += "STATION"; break;
        case MODE_AP: html += "AP"; break;
        case MODE_OFF: html += "OFF"; break;
    }
    
    html += "</div></div><div class=\"stat-card\"><div class=\"stat-label\">IP Address</div><div class=\"stat-value\" style=\"font-size:1.2em\">";
    
    if (currentMode == MODE_AP) {
        html += WiFi.softAPIP().toString();
    } else if (WiFi.status() == WL_CONNECTED) {
        html += WiFi.localIP().toString();
    } else {
        html += "N/A";
    }
    
    html += "</div></div><div class=\"stat-card\"><div class=\"stat-label\">Free Heap</div><div class=\"stat-value\">";
    html += String(ESP.getFreeHeap() / 1024) + " KB";
    html += "</div></div><div class=\"stat-card\"><div class=\"stat-label\">Clients</div><div class=\"stat-value\">";
    
    if (currentMode == MODE_AP) {
        html += String(WiFi.softAPgetStationNum());
    } else {
        html += "N/A";
    }
    
    html += "</div></div></div>";
    
    // QR Code for AP mode
    if (currentMode == MODE_AP) {
        html += "<h2>📱 Connect</h2><div style=\"background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);padding:30px;border-radius:10px;text-align:center;color:#fff\"><p style=\"font-size:1.3em;margin-bottom:20px;font-weight:bold\">📷 Scan QR Code</p><div style=\"display:inline-block;background:#fff;padding:15px;border-radius:10px\">";
        html += "<div style='width:250px;height:250px'>" + generateQRCodeSVG(currentAPSSID, currentAPPassword, "WPA") + "</div>";
        html += "</div><div style=\"margin-top:20px\"><p><strong>Network:</strong> " + String(currentAPSSID) + "</p></div></div>";
    }

    html += "<h2>ℹ️ Features</h2><ul style=\"margin:15px 0 15px 30px;line-height:1.8\"><li>📡 <strong>Channel Analysis</strong>: 2.4GHz spectrum scanning</li><li>🔍 <strong>Network Scanning</strong>: WiFi discovery</li><li>⚡ <strong>iPerf Testing</strong>: Bandwidth measurement</li><li>📊 <strong>Latency Analysis</strong>: Jitter & packet loss testing</li><li>🌐 <strong>Access Point</strong>: WiFi hotspot with QR</li></ul>";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleStatus() {
    String html = HTML_HEADER;
    
    html += "<div class=\"header\"><h1>📊 Status</h1></div>";
    html += generateNav();
    html += "<h2>🔧 System Info</h2><div style=\"background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0\">";
    
    html += "<p><strong>WiFi Mode:</strong> ";
    switch (currentMode) {
        case MODE_IDLE:
            html += "⚪ Idle (Ready)";
            break;
        case MODE_STATION:
            html += "🔍 Station (Scanner)";
            break;
        case MODE_AP:
            html += "📡 Access Point";
            break;
        case MODE_OFF:
            html += "🔴 Disabled";
            break;
    }
    html += "</p>";
    
    html += "<p><strong>Scanning:</strong> " + String(scanningEnabled ? "✅ Enabled" : "❌ Disabled") + "</p>";
    
    if (currentMode == MODE_AP) {
        html += "<p><strong>AP SSID:</strong> " + String(currentAPSSID) + "</p>";
        html += "<p><strong>AP IP:</strong> " + WiFi.softAPIP().toString() + "</p>";
        html += "<p><strong>Connected Clients:</strong> " + String(WiFi.softAPgetStationNum()) + "</p>";
    } else if (WiFi.status() == WL_CONNECTED) {
        html += "<p><strong>Connected to:</strong> " + WiFi.SSID() + "</p>";
        html += "<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>";
        html += "<p><strong>Signal Strength:</strong> " + String(WiFi.RSSI()) + " dBm</p>";
    }
    
    html += "<p><strong>Free Heap:</strong> " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "<p><strong>Chip Model:</strong> " + String(ESP.getChipModel()) + "</p>";
    html += "<p><strong>CPU Frequency:</strong> " + String(ESP.getCpuFreqMHz()) + " MHz</p>";
    html += "<p><strong>Flash Size:</strong> " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB</p>";
    html += "</div>";
    
    // QR Code for AP mode
    if (currentMode == MODE_AP) {
        html += "<h2>📱 Connect</h2><div style=\"background:#f8f9fa;padding:30px;border-radius:10px;text-align:center\"><p style=\"font-size:1.2em;margin-bottom:20px;color:#667eea;font-weight:bold\">Scan QR to connect</p><div style=\"display:inline-block;background:#fff;padding:20px;border-radius:10px\">";
        html += "<div style='width:300px;height:300px'>" + generateQRCodeSVG(currentAPSSID, currentAPPassword, "WPA") + "</div>";
        html += "</div><div style=\"margin-top:20px;padding:20px;background:#fff;border-radius:8px;display:inline-block;text-align:left\"><p><strong>SSID:</strong> " + String(currentAPSSID) + "</p><p><strong>Password:</strong> " + String(currentAPPassword) + "</p><p><strong>Security:</strong> WPA2</p><p><strong>IP:</strong> " + WiFi.softAPIP().toString() + "</p></div></div>";
    }
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleScan() {
    String html = HTML_HEADER;
    
    html += R"rawliteral(
    <div class="header">
        <h1>🔍 Network Scan</h1>
    </div>

    <div class="nav">
        <div><a href="/">🏠 Home</a></div>
        <div><a href="/status">📊 Status</a></div>
        <div><a href="/scan">🔍 Scan Networks</a></div>
        <div class="dropdown">
            <a href="/analysis">🔬 Analysis</a>
            <div class="dropdown-content">
                <a href="/analysis">📊 Dashboard</a>
                <a href="/iperf">⚡ iPerf</a>
                <a href="/latency">📉 Latency</a>
                <a href="/channel">📡 Channel</a>
            </div>
        </div>
    </div>

    <h2>📡 Available Networks</h2>
    
    <div style="text-align: center; margin: 20px 0;">
        <button onclick="startScan('/scan?doscan=1', '🔍 Scanning Networks...', 'Please wait while we discover nearby WiFi networks')" 
                style="padding: 15px 40px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border: none; border-radius: 8px; font-size: 1.1em; font-weight: bold; cursor: pointer; box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);">
            🔍 Start Network Scan
        </button>
    </div>
    )rawliteral";
    
    // Check if scan was requested
    if (webServer->hasArg("doscan")) {
        // Perform WiFi scan
        int n = WiFi.scanNetworks();
        
        if (n == 0) {
            html += "<p style='text-align: center; padding: 40px; color: #666;'>No networks found. Try scanning again.</p>";
        } else {
            html += "<ul class=\"network-list\">";
            
            for (int i = 0; i < n; i++) {
            html += "<li class=\"network-item\">";
            html += "<div class=\"network-info\">";
            html += "<div class=\"network-name\">" + WiFi.SSID(i) + "</div>";
            html += "<div class=\"network-details\">";
            html += "Channel: " + String(WiFi.channel(i)) + " | ";
            html += "Security: " + String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Secured");
            html += "</div></div>";
            
            // Signal strength with colored circles
            int rssi = WiFi.RSSI(i);
            int bars = 0;
            String color;
            
            // Determine number of bars and color based on RSSI
            if (rssi >= -50) {
                bars = 5;  // Excellent: 5 green circles
                color = "#10b981";  // Green
            } else if (rssi >= -60) {
                bars = 4;  // Very Good: 4 green circles
                color = "#10b981";  // Green
            } else if (rssi >= -67) {
                bars = 3;  // Good: 3 yellow circles
                color = "#fbbf24";  // Yellow
            } else if (rssi >= -75) {
                bars = 2;  // Fair: 2 orange circles
                color = "#fb923c";  // Orange
            } else {
                bars = 1;  // Poor: 1 red circle
                color = "#ef4444";  // Red
            }
            
            // Build signal strength indicator with colored circles
            html += "<div class=\"signal-strength\" title=\"" + String(rssi) + " dBm\" style=\"display: flex; gap: 3px; align-items: center;\">";
            for (int j = 0; j < 5; j++) {
                if (j < bars) {
                    // Filled circle with color
                    html += "<svg width=\"14\" height=\"14\" style=\"display: block;\"><circle cx=\"7\" cy=\"7\" r=\"6\" fill=\"" + color + "\"/></svg>";
                } else {
                    // Empty circle (gray outline)
                    html += "<svg width=\"14\" height=\"14\" style=\"display: block;\"><circle cx=\"7\" cy=\"7\" r=\"6\" fill=\"none\" stroke=\"#d1d5db\" stroke-width=\"2\"/></svg>";
                }
            }
            html += "<span style=\"margin-left: 8px; color: #666; font-size: 0.9em;\">" + String(rssi) + " dBm</span>";
            html += "</div>";
            html += "</li>";
        }
        
        html += "</ul>";
        html += "<p style='text-align: center; margin-top: 20px;'><strong>Found " + String(n) + " network(s)</strong></p>";
        }
        
        WiFi.scanDelete();
    } else {
        html += "<p style='text-align: center; padding: 40px; color: #999;'>Click the button above to scan for available WiFi networks.</p>";
    }
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleNetworkAnalysis() {
    String html = HTML_HEADER;
    
    html += R"rawliteral(
    <div class="header">
        <h1>🔬 Network Analysis Dashboard</h1>
        <p>Comprehensive Network Diagnostics & Performance Overview</p>
    </div>

    <div class="nav">
        <div><a href="/">🏠 Home</a></div>
        <div><a href="/status">📊 Status</a></div>
        <div><a href="/scan">🔍 Scan Networks</a></div>
        <div class="dropdown">
            <a href="/analysis">🔬 Analysis</a>
            <div class="dropdown-content">
                <a href="/analysis">📊 Dashboard</a>
                <a href="/iperf">⚡ iPerf</a>
                <a href="/latency">📉 Latency</a>
                <a href="/channel">📡 Channel</a>
            </div>
        </div>
    </div>

    <h2>🎯 Network Analysis Tools</h2>
    <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin: 20px 0;">
        
        <!-- Channel Analysis Card -->
        <div style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 25px; border-radius: 10px; box-shadow: 0 4px 15px rgba(0,0,0,0.2);">
            <h3 style="margin: 0 0 10px 0; font-size: 1.3em;">📡 Channel Analysis</h3>
            <p style="margin: 0 0 15px 0; opacity: 0.9;">Analyze 2.4GHz spectrum congestion and find optimal channels</p>
            <div style="margin: 10px 0;">
                <span style="font-size: 0.9em; opacity: 0.8;">Last Scan:</span><br>
    )rawliteral";
    
    if (lastChannelAnalysis.scan_timestamp > 0) {
        unsigned long timeSinceScan = (millis() - lastChannelAnalysis.scan_timestamp) / 1000;
        html += "<strong>" + String(timeSinceScan) + " seconds ago</strong>";
    } else {
        html += "<strong>Never</strong>";
    }
    
    html += R"rawliteral(
            </div>
            <button onclick="location.href='/channel'" style="margin-top: 15px; padding: 10px 20px; background: white; color: #667eea; border: none; border-radius: 5px; font-weight: bold; cursor: pointer; width: 100%;">
                View Channel Analysis
            </button>
        </div>

        <!-- Latency Testing Card -->
        <div style="background: linear-gradient(135deg, #10b981 0%, #059669 100%); color: white; padding: 25px; border-radius: 10px; box-shadow: 0 4px 15px rgba(0,0,0,0.2);">
            <h3 style="margin: 0 0 10px 0; font-size: 1.3em;">📉 Latency & Jitter</h3>
            <p style="margin: 0 0 15px 0; opacity: 0.9;">Test network quality with comprehensive latency analysis</p>
            <div style="margin: 10px 0;">
                <span style="font-size: 0.9em; opacity: 0.8;">Test Status:</span><br>
    )rawliteral";
    
    LatencyTestState latencyState = getLatencyTestState();
    switch (latencyState) {
        case LATENCY_IDLE:
            html += "<strong>🟢 Ready</strong>";
            break;
        case LATENCY_RUNNING:
            html += "<strong>🟡 Running</strong>";
            break;
        case LATENCY_COMPLETED:
            html += "<strong>✅ Completed</strong>";
            break;
        case LATENCY_ERROR:
            html += "<strong>🔴 Error</strong>";
            break;
        default:
            html += "<strong>Unknown</strong>";
    }
    
    html += R"rawliteral(
            </div>
            <button onclick="location.href='/latency'" style="margin-top: 15px; padding: 10px 20px; background: white; color: #10b981; border: none; border-radius: 5px; font-weight: bold; cursor: pointer; width: 100%;">
                Start Latency Test
            </button>
        </div>

        <!-- iPerf Testing Card -->
        <div style="background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%); color: white; padding: 25px; border-radius: 10px; box-shadow: 0 4px 15px rgba(0,0,0,0.2);">
            <h3 style="margin: 0 0 10px 0; font-size: 1.3em;">⚡ iPerf Testing</h3>
            <p style="margin: 0 0 15px 0; opacity: 0.9;">Measure network bandwidth and throughput performance</p>
            <div style="margin: 10px 0;">
                <span style="font-size: 0.9em; opacity: 0.8;">Test Status:</span><br>
    )rawliteral";
    
    // Use the global currentIperfState variable directly
    switch (currentIperfState) {
        case IPERF_IDLE:
            html += "<strong>🟢 Ready</strong>";
            break;
        case IPERF_RUNNING:
            html += "<strong>🟡 Running</strong>";
            break;
        case IPERF_STOPPING:
            html += "<strong>🟠 Stopping</strong>";
            break;
        default:
            html += "<strong>Unknown</strong>";
    }
    
    html += R"rawliteral(
            </div>
            <button onclick="location.href='/iperf'" style="margin-top: 15px; padding: 10px 20px; background: white; color: #f59e0b; border: none; border-radius: 5px; font-weight: bold; cursor: pointer; width: 100%;">
                Start iPerf Test
            </button>
        </div>
    </div>

    <h2>📊 Network Statistics</h2>
    <div class="stat-grid">
    )rawliteral";
    
    // Connection statistics
    if (WiFi.status() == WL_CONNECTED) {
        html += R"rawliteral(
        <div class="stat-card">
            <div class="stat-label">Signal Strength</div>
            <div class="stat-value">)rawliteral";
        html += String(WiFi.RSSI());
        html += R"rawliteral( dBm</div>
        </div>
        
        <div class="stat-card">
            <div class="stat-label">Current Channel</div>
            <div class="stat-value">)rawliteral";
        html += String(WiFi.channel());
        html += R"rawliteral(</div>
        </div>
        
        <div class="stat-card">
            <div class="stat-label">Local IP</div>
            <div class="stat-value" style="font-size: 1.2em;">)rawliteral";
        html += WiFi.localIP().toString();
        html += R"rawliteral(</div>
        </div>
        
        <div class="stat-card">
            <div class="stat-label">Gateway</div>
            <div class="stat-value" style="font-size: 1.2em;">)rawliteral";
        html += WiFi.gatewayIP().toString();
        html += R"rawliteral(</div>
        </div>
        )rawliteral";
    }
    
    // Channel analysis summary
    if (lastChannelAnalysis.scan_timestamp > 0) {
        html += R"rawliteral(
        <div class="stat-card">
            <div class="stat-label">Networks Found</div>
            <div class="stat-value">)rawliteral";
        html += String(lastChannelAnalysis.total_networks);
        html += R"rawliteral(</div>
        </div>
        
        <div class="stat-card">
            <div class="stat-label">Best Channel</div>
            <div class="stat-value">)rawliteral";
        html += String(lastChannelAnalysis.best_channel_2g4);
        html += R"rawliteral(</div>
        </div>
        
        <div class="stat-card">
            <div class="stat-label">Overall Congestion</div>
            <div class="stat-value">)rawliteral";
        html += String(lastChannelAnalysis.overall_congestion, 1);
        html += R"rawliteral(%</div>
        </div>
        )rawliteral";
    }
    
    // Latency test results summary
    LatencyTestResults latencyResults = getLastLatencyResults();
    if (latencyResults.test_completed && latencyResults.statistics.packets_received > 0) {
        html += R"rawliteral(
        <div class="stat-card">
            <div class="stat-label">Avg Latency</div>
            <div class="stat-value">)rawliteral";
        html += String(latencyResults.statistics.avg_latency_ms, 2);
        html += R"rawliteral( ms</div>
        </div>
        
        <div class="stat-card">
            <div class="stat-label">Packet Loss</div>
            <div class="stat-value">)rawliteral";
        html += String(latencyResults.statistics.packet_loss_percent, 1);
        html += R"rawliteral(%</div>
        </div>
        
        <div class="stat-card">
            <div class="stat-label">Jitter</div>
            <div class="stat-value">)rawliteral";
        html += String(latencyResults.statistics.jitter_ms, 2);
        html += R"rawliteral( ms</div>
        </div>
        )rawliteral";
    }
    
    html += R"rawliteral(
    </div>

    <h2>🔍 Quick Actions</h2>
    <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin: 20px 0;">
        <button onclick="startScan('/channel/scan', '📡 Analyzing Channels...', 'Scanning 2.4GHz spectrum for network congestion and interference')" style="padding: 15px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer;">
            🔄 Scan Channels
        </button>
        <button onclick="startScan('/scan?doscan=1', '🔍 Scanning Networks...', 'Discovering nearby WiFi networks and signal strength')" style="padding: 15px; background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%); color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer;">
            📡 Scan Networks
        </button>
        <button onclick="location.href='/latency/start'" style="padding: 15px; background: linear-gradient(135deg, #10b981 0%, #059669 100%); color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer;">
            📉 Test Latency
        </button>
        <button onclick="location.href='/iperf'" style="padding: 15px; background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%); color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer;">
            ⚡ Run iPerf
        </button>
    </div>

    <h2>💡 Network Analysis Tips</h2>
    <div style="background: #f0f9ff; padding: 20px; border-left: 4px solid #3b82f6; border-radius: 5px; margin: 20px 0;">
        <ul style="margin: 10px 0; padding-left: 25px;">
            <li><strong>Channel Analysis:</strong> Identifies congested channels and recommends optimal ones for your AP</li>
            <li><strong>Latency Testing:</strong> Measures response time, jitter, and packet loss to assess connection quality</li>
            <li><strong>iPerf Testing:</strong> Measures maximum bandwidth and throughput capabilities</li>
            <li><strong>Best Practice:</strong> Run channel analysis before deploying an access point</li>
            <li><strong>Troubleshooting:</strong> High latency or packet loss indicates network issues</li>
        </ul>
    </div>
    )rawliteral";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleChannelAnalysis() {
    String html = HTML_HEADER;
    
    html += R"rawliteral(
    <div class="header">
        <h1>📡 Channel Analysis</h1>
        <p>2.4GHz Spectrum Overview & Congestion Analysis</p>
    </div>

    <div class="nav">
        <div><a href="/">🏠 Home</a></div>
        <div><a href="/status">📊 Status</a></div>
        <div><a href="/scan">🔍 Scan Networks</a></div>
        <div class="dropdown">
            <a href="/analysis">🔬 Analysis</a>
            <div class="dropdown-content">
                <a href="/analysis">📊 Dashboard</a>
                <a href="/iperf">⚡ iPerf</a>
                <a href="/latency">📉 Latency</a>
                <a href="/channel">📡 Channel</a>
            </div>
        </div>
    </div>
    )rawliteral";
    
    // Display success message if just scanned
    if (webServer->hasArg("scanned")) {
        html += R"rawliteral(
        <div style="background: #d1fae5; padding: 15px; border-left: 4px solid #10b981; border-radius: 5px; margin: 20px 0;">
            <strong>✅ Success!</strong> Channel scan completed successfully.
        </div>
        )rawliteral";
    }
    
    html += R"rawliteral(
    <h2>📊 Channel Spectrum</h2>
    )rawliteral";
    
    // Display scan button
    html += R"rawliteral(
    <div style="text-align: center; margin: 20px 0;">
        <button onclick="startScan('/channel/scan', '📡 Analyzing Channels...', 'Scanning 2.4GHz spectrum across all WiFi channels')" style="padding: 15px 40px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border: none; border-radius: 5px; font-size: 1.1em; cursor: pointer; font-weight: bold;">
            🔄 Scan Channels
        </button>
    </div>
    )rawliteral";
    
    // Display channel analysis if available
    if (lastChannelAnalysis.scan_timestamp > 0) {
        html += R"rawliteral(
        <div style="background: #f8f9fa; padding: 20px; border-radius: 10px; margin: 20px 0;">
            <h3>🎯 Recommendations</h3>
        )rawliteral";
        
        html += "<p><strong>Best Channel:</strong> <span class=\"badge success\">Channel " + String(lastChannelAnalysis.best_channel_2g4) + "</span></p>";
        html += "<p><strong>Worst Channel:</strong> <span class=\"badge danger\">Channel " + String(lastChannelAnalysis.worst_channel_2g4) + "</span></p>";
        html += "<p><strong>Total Networks:</strong> " + String(lastChannelAnalysis.total_networks) + "</p>";
        html += "<p><strong>Overall Congestion:</strong> " + String(lastChannelAnalysis.overall_congestion, 1) + "%</p>";
        
        html += "</div>";
        
        // Display channel data in a grid
        html += R"rawliteral(
        <h3>📊 Channel Details</h3>
        <div style="display: grid; grid-template-columns: repeat(auto-fill, minmax(150px, 1fr)); gap: 15px; margin: 20px 0;">
        )rawliteral";
        
        for (int i = 0; i < 14; i++) {
            ChannelCongestionData ch = lastChannelAnalysis.channels[i];
            if (ch.channel == 0) continue; // Skip unscanned channels
            
            // Determine card color based on congestion
            String cardColor = "#10b981"; // Green (good)
            if (ch.congestion_score > 70) {
                cardColor = "#ef4444"; // Red (bad)
            } else if (ch.congestion_score > 40) {
                cardColor = "#fbbf24"; // Yellow (moderate)
            }
            
            html += "<div style=\"background: " + cardColor + "; color: white; padding: 15px; border-radius: 8px; text-align: center;\">";
            html += "<div style=\"font-size: 1.5em; font-weight: bold;\">CH " + String(ch.channel) + "</div>";
            html += "<div style=\"font-size: 0.9em; margin: 5px 0;\">🌐 " + String(ch.network_count) + " networks</div>";
            html += "<div style=\"font-size: 0.9em;\">📊 " + String(ch.congestion_score, 0) + "% congestion</div>";
            if (ch.is_recommended) {
                html += "<div style=\"margin-top: 5px; font-size: 0.8em;\">⭐ Recommended</div>";
            }
            html += "</div>";
        }
        
        html += "</div>";
        
        // Scan info
        unsigned long ageSeconds = (millis() - lastChannelAnalysis.scan_timestamp) / 1000;
        html += "<p style=\"text-align: center; color: #666; margin-top: 20px;\">";
        html += "Last scan: " + String(ageSeconds) + " seconds ago | Duration: " + String(lastChannelAnalysis.scan_duration_ms) + " ms";
        html += "</p>";
    } else {
        html += R"rawliteral(
        <div style="background: #fff3cd; padding: 20px; border-left: 4px solid #ffc107; border-radius: 5px; margin: 20px 0;">
            <p style="text-align: center; color: #666;">
                <strong>No channel data available yet.</strong><br><br>
                Click "Scan Channels" above to analyze the 2.4GHz spectrum.
            </p>
        </div>
        )rawliteral";
    }
    
    html += R"rawliteral(
    <h2>ℹ️ About Channel Analysis</h2>
    <ul style="margin: 15px 0 15px 30px; line-height: 1.8;">
        <li><strong>Channel Congestion:</strong> Measures how crowded each WiFi channel is</li>
        <li><strong>Network Count:</strong> Number of access points operating on each channel</li>
        <li><strong>Overlapping Channels:</strong> Adjacent channels that cause interference (2.4GHz)</li>
        <li><strong>Recommendations:</strong> Best channels to use for minimal interference</li>
        <li><strong>Non-overlapping Channels:</strong> In 2.4GHz, use channels 1, 6, or 11 for best results</li>
    </ul>
    
    <div style="background: #e3f2fd; padding: 20px; border-left: 4px solid #2196f3; border-radius: 5px; margin: 20px 0;">
        <strong>💡 Best Practices:</strong><br><br>
        • Use channels 1, 6, or 11 in 2.4GHz to avoid overlap<br>
        • Choose channels with fewer networks and lower congestion<br>
        • Consider 5GHz band for less interference (if available)<br>
        • Rescan periodically as network conditions change
    </div>
    )rawliteral";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleNotFound() {
    String html = HTML_HEADER;
    
    html += R"rawliteral(
    <div class="header">
        <h1>❌ 404 - Page Not Found</h1>
    </div>

    <div class="nav">
        <div><a href="/">🏠 Home</a></div>
        <div><a href="/status">📊 Status</a></div>
        <div><a href="/scan">🔍 Scan Networks</a></div>
        <div class="dropdown">
            <a href="/analysis">🔬 Analysis</a>
            <div class="dropdown-content">
                <a href="/analysis">📊 Dashboard</a>
                <a href="/iperf">⚡ iPerf</a>
                <a href="/latency">📉 Latency</a>
                <a href="/channel">📡 Channel</a>
            </div>
        </div>
    </div>

    <div style="text-align: center; padding: 60px 20px;">
        <h2 style="font-size: 4em; margin: 20px 0;">404</h2>
        <p style="font-size: 1.2em; color: #666; margin: 20px 0;">
            The page you're looking for doesn't exist.
        </p>
        <a href="/" style="display: inline-block; margin-top: 20px; padding: 15px 30px; background: #667eea; color: white; text-decoration: none; border-radius: 5px; font-weight: bold;">
            🏠 Go to Homepage
        </a>
    </div>
    )rawliteral";
    
    html += generateHtmlFooter();
    webServer->send(404, "text/html", html);
}

// ==========================================
// IPERF PAGE HANDLERS
// ==========================================

void handleIperf() {
    String html = HTML_HEADER;
    
    html += R"rawliteral(
    <div class="header">
        <h1>⚡ iPerf Performance Testing</h1>
        <p>Network Bandwidth & Throughput Analysis</p>
    </div>

    <div class="nav">
        <div><a href="/">🏠 Home</a></div>
        <div><a href="/status">📊 Status</a></div>
        <div><a href="/scan">🔍 Scan Networks</a></div>
        <div class="dropdown">
            <a href="/analysis">🔬 Analysis</a>
            <div class="dropdown-content">
                <a href="/analysis">📊 Dashboard</a>
                <a href="/iperf">⚡ iPerf</a>
                <a href="/latency">📉 Latency</a>
                <a href="/channel">📡 Channel</a>
            </div>
        </div>
    </div>
    )rawliteral";
    
    // Display success/error messages
    if (webServer->hasArg("started")) {
        html += R"rawliteral(
        <div style="background: #d1fae5; padding: 15px; border-left: 4px solid #10b981; border-radius: 5px; margin: 20px 0;">
            <strong>✅ Success!</strong> iPerf test started successfully.
        </div>
        )rawliteral";
    } else if (webServer->hasArg("error")) {
        html += "<div style=\"background: #fee; padding: 15px; border-left: 4px solid #f44; border-radius: 5px; margin: 20px 0;\">";
        html += "<strong>❌ Error:</strong> " + webServer->arg("error");
        html += "</div>";
    } else if (webServer->hasArg("stopped")) {
        html += R"rawliteral(
        <div style="background: #fff3cd; padding: 15px; border-left: 4px solid #ffc107; border-radius: 5px; margin: 20px 0;">
            <strong>🛑 Stopped:</strong> iPerf test has been stopped.
        </div>
        )rawliteral";
    }
    
    html += R"rawliteral(
    <h2>⚡ iPerf Status</h2>
    <div style="background: #f8f9fa; padding: 20px; border-radius: 10px; margin: 20px 0;">
    )rawliteral";
    
    // Display current status
    html += "<p><strong>Current State:</strong> ";
    switch (currentIperfState) {
        case IPERF_IDLE:
            html += "<span class=\"badge info\">🟡 Idle</span></p>";
            break;
        case IPERF_RUNNING:
            html += "<span class=\"badge success\">🟢 Running</span></p>";
            break;
        case IPERF_STOPPING:
            html += "<span class=\"badge warning\">🟠 Stopping</span></p>";
            break;
    }
    
    if (currentIperfState == IPERF_RUNNING) {
        html += "<p><strong>Mode:</strong> ";
        html += activeConfig.mode == IPERF_CLIENT ? "Client" : "Server";
        html += " (";
        html += activeConfig.protocol == IPERF_TCP ? "TCP" : "UDP";
        html += ")</p>";
        
        if (activeConfig.mode == IPERF_CLIENT) {
            html += "<p><strong>Server:</strong> " + activeConfig.serverIP + ":" + String(activeConfig.port) + "</p>";
        } else {
            html += "<p><strong>Listening Port:</strong> " + String(activeConfig.port) + "</p>";
        }
        
        html += "<p><strong>Duration:</strong> " + String(activeConfig.duration) + " seconds</p>";
        
        if (iperfStartTime > 0) {
            unsigned long elapsed = (millis() - iperfStartTime) / 1000;
            html += "<p><strong>Elapsed:</strong> " + String(elapsed) + " seconds</p>";
        }
    }
    
    html += "</div>";
    
    // Display last results if available
    if (lastResults.testCompleted) {
        html += R"rawliteral(
        <h2>📊 Last Test Results</h2>
        <div class="stat-grid">
            <div class="stat-card">
                <div class="stat-label">Data Transferred</div>
                <div class="stat-value" style="font-size: 1.5em;">
        )rawliteral";
        html += formatBytes(lastResults.bytesTransferred);
        html += R"rawliteral(
                </div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Throughput</div>
                <div class="stat-value" style="font-size: 1.5em;">
        )rawliteral";
        html += formatThroughput(lastResults.throughputMbps);
        html += R"rawliteral(
                </div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Duration</div>
                <div class="stat-value" style="font-size: 1.5em;">
        )rawliteral";
        html += String(lastResults.durationMs / 1000.0, 1) + "s";
        html += R"rawliteral(
                </div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Status</div>
                <div class="stat-value" style="font-size: 1.2em;">
        )rawliteral";
        html += lastResults.testCompleted ? "✅ Complete" : "⚠️ Incomplete";
        html += R"rawliteral(
                </div>
            </div>
        </div>
        )rawliteral";
    }
    
    // Control buttons
    html += R"rawliteral(
    <h2>🎮 Test Control</h2>
    <div style="display: flex; gap: 15px; justify-content: center; margin: 30px 0; flex-wrap: wrap;">
    )rawliteral";
    
    if (currentIperfState == IPERF_RUNNING) {
        html += R"rawliteral(
        <button onclick="if(confirm('Stop the running iPerf test?')) location.href='/iperf/stop'" style="padding: 15px 30px; background: #ef4444; color: white; border: none; border-radius: 5px; font-size: 1.1em; cursor: pointer; font-weight: bold;">
            🛑 Stop Test
        </button>
        <button onclick="location.reload()" style="padding: 15px 30px; background: #3b82f6; color: white; border: none; border-radius: 5px; font-size: 1.1em; cursor: pointer; font-weight: bold;">
            🔄 Refresh Status
        </button>
        )rawliteral";
    } else {
        html += R"rawliteral(
        <button onclick="location.href='/iperf/start'" style="padding: 15px 30px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border: none; border-radius: 5px; font-size: 1.1em; cursor: pointer; font-weight: bold;">
            🚀 Start New Test
        </button>
        )rawliteral";
    }
    
    html += R"rawliteral(
    </div>

    <h2>ℹ️ About iPerf Testing</h2>
    <ul style="margin: 15px 0 15px 30px; line-height: 1.8;">
        <li><strong>Server Mode:</strong> ESP32 listens for connections from external iPerf clients</li>
        <li><strong>Client Mode:</strong> ESP32 connects to an external iPerf server for testing</li>
        <li><strong>TCP Testing:</strong> Reliable throughput measurement for stable connections</li>
        <li><strong>UDP Testing:</strong> Packet loss and jitter analysis for real-time applications</li>
        <li><strong>Real-Time Monitoring:</strong> Track test progress and results in the browser</li>
    </ul>
    
    <div style="background: #e3f2fd; padding: 20px; border-left: 4px solid #2196f3; border-radius: 5px; margin: 20px 0;">
        <strong>💡 Quick Start:</strong><br><br>
        <strong>To test as Server:</strong> Click "Start New Test", select "Server Mode", choose protocol, and click start.<br>
        Connect from your computer: <code>iperf3 -c )rawliteral";
    
    if (WiFi.status() == WL_CONNECTED) {
        html += WiFi.localIP().toString();
    } else if (currentMode == MODE_AP) {
        html += WiFi.softAPIP().toString();
    } else {
        html += "&lt;ESP32-IP&gt;";
    }
    
    html += R"rawliteral(</code><br><br>
        <strong>To test as Client:</strong> Start an iPerf server on your network, then configure ESP32 as client with the server's IP address.
    </div>
    )rawliteral";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleIperfStart() {
    // Handle POST request to start iPerf test
    if (webServer->method() == HTTP_POST) {
        String mode = webServer->arg("mode");
        String protocol = webServer->arg("protocol");
        String serverIP = webServer->arg("serverIP");
        String port = webServer->arg("port");
        String duration = webServer->arg("duration");
        
        // Create configuration
        IperfConfig config = getDefaultConfig();
        config.mode = (mode == "server") ? IPERF_SERVER : IPERF_CLIENT;
        config.protocol = (protocol == "udp") ? IPERF_UDP : IPERF_TCP;
        config.port = port.length() > 0 ? port.toInt() : IPERF_DEFAULT_PORT;
        config.duration = duration.length() > 0 ? duration.toInt() : IPERF_DEFAULT_DURATION;
        
        if (config.mode == IPERF_CLIENT) {
            config.serverIP = serverIP;
        }
        
        // Validate and start
        bool success = false;
        String errorMsg = "";
        
        if (config.mode == IPERF_CLIENT && serverIP.length() == 0) {
            errorMsg = "Server IP required for client mode";
        } else {
            // Stop previous test if running
            if (currentIperfState == IPERF_RUNNING) {
                Serial.println("🛑 Stopping previous iPerf test before starting new one...");
                stopIperfTest();
                delay(100); // Brief delay to ensure clean stop
            }
            
            if (config.mode == IPERF_SERVER) {
                success = startIperfServer(config);
            } else {
                success = startIperfClient(config);
            }
            
            if (!success) {
                errorMsg = "Failed to start iPerf test. Check configuration.";
            }
        }
        
        // Redirect with status
        if (success) {
            webServer->sendHeader("Location", "/iperf?started=1", true);
        } else {
            webServer->sendHeader("Location", "/iperf?error=" + errorMsg, true);
        }
        webServer->send(302, "text/plain", "");
        return;
    }
    
    // Show configuration form (GET request)
    String html = HTML_HEADER;
    
    html += R"rawliteral(
    <style>
        .form-group {
            margin: 20px 0;
        }
        .form-group label {
            display: block;
            font-weight: bold;
            margin-bottom: 5px;
            color: #333;
        }
        .form-group input, .form-group select {
            width: 100%;
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 5px;
            font-size: 1em;
            box-sizing: border-box;
        }
        .form-group input:focus, .form-group select:focus {
            outline: none;
            border-color: #667eea;
        }
        .form-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }
        .submit-btn {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 15px 40px;
            border: none;
            border-radius: 5px;
            font-size: 1.2em;
            font-weight: bold;
            cursor: pointer;
            width: 100%;
            margin-top: 20px;
        }
        .submit-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
        }
        .info-box {
            background: #e3f2fd;
            padding: 15px;
            border-left: 4px solid #2196f3;
            border-radius: 5px;
            margin: 20px 0;
        }
        @media (max-width: 768px) {
            .form-row {
                grid-template-columns: 1fr;
            }
        }
    </style>
    
    <div class="header">
        <h1>⚡ Start iPerf Test</h1>
        <p>Configure and Launch Network Performance Test</p>
    </div>

    <div class="nav">
        <div><a href="/">🏠 Home</a></div>
        <div><a href="/status">📊 Status</a></div>
        <div><a href="/scan">🔍 Scan Networks</a></div>
        <div class="dropdown">
            <a href="/analysis">🔬 Analysis</a>
            <div class="dropdown-content">
                <a href="/analysis">📊 Dashboard</a>
                <a href="/iperf">⚡ iPerf</a>
                <a href="/latency">📉 Latency</a>
                <a href="/channel">📡 Channel</a>
            </div>
        </div>
    </div>

    <h2>🔧 Test Configuration</h2>
    
    <form method="POST" action="/iperf/start">
        <div class="form-group">
            <label for="mode">Test Mode</label>
            <select id="mode" name="mode" onchange="toggleServerIP()" required>
                <option value="server">Server Mode (Listen for connections)</option>
                <option value="client">Client Mode (Connect to server)</option>
            </select>
        </div>
        
        <div class="form-group">
            <label for="protocol">Protocol</label>
            <select id="protocol" name="protocol" required>
                <option value="tcp">TCP (Reliable throughput)</option>
                <option value="udp">UDP (Latency & jitter)</option>
            </select>
        </div>
        
        <div class="form-group" id="serverIPGroup" style="display: none;">
            <label for="serverIP">Server IP Address</label>
            <input type="text" id="serverIP" name="serverIP" placeholder="e.g., 192.168.1.100" pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
            <small style="color: #666;">Required for client mode</small>
        </div>
        
        <div class="form-row">
            <div class="form-group">
                <label for="port">Port</label>
                <input type="number" id="port" name="port" value="5201" min="1" max="65535" required>
            </div>
            
            <div class="form-group">
                <label for="duration">Duration (seconds)</label>
                <input type="number" id="duration" name="duration" value="10" min="1" max="300" required>
            </div>
        </div>
        
        <div class="info-box">
            <strong>ℹ️ Test Modes:</strong><br>
            • <strong>Server Mode:</strong> ESP32 listens for incoming connections from iPerf clients<br>
            • <strong>Client Mode:</strong> ESP32 connects to an external iPerf server for testing
        </div>
        
        <button type="submit" class="submit-btn">🚀 Start iPerf Test</button>
    </form>
    
    <script>
        function toggleServerIP() {
            const mode = document.getElementById('mode').value;
            const serverIPGroup = document.getElementById('serverIPGroup');
            const serverIPInput = document.getElementById('serverIP');
            
            if (mode === 'client') {
                serverIPGroup.style.display = 'block';
                serverIPInput.required = true;
            } else {
                serverIPGroup.style.display = 'none';
                serverIPInput.required = false;
            }
        }
        
        // Initialize on page load
        toggleServerIP();
    </script>
    )rawliteral";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleIperfStop() {
    stopIperfTest();
    
    // Redirect back to main iPerf page with stopped message
    webServer->sendHeader("Location", "/iperf?stopped=1", true);
    webServer->send(302, "text/plain", "");
}

void handleIperfResults() {
    // Redirect to main iPerf page
    webServer->sendHeader("Location", "/iperf", true);
    webServer->send(302, "text/plain", "");
}

// ==========================================
// CHANNEL SCAN HANDLER
// ==========================================

void handleChannelScan() {
    // Perform channel scan
    ChannelAnalysisResults results = quickChannelScan();
    
    // Redirect back to channel page
    webServer->sendHeader("Location", "/channel?scanned=1", true);
    webServer->send(302, "text/plain", "");
}

// ==========================================
// LATENCY TEST HANDLERS
// ==========================================

void handleLatency() {
    String html = HTML_HEADER;
    
    html += R"rawliteral(
    <div class="header">
        <h1>📉 Latency & Jitter Testing</h1>
        <p>Network Performance & Quality Analysis</p>
    </div>

    <div class="nav">
        <div><a href="/">🏠 Home</a></div>
        <div><a href="/status">📊 Status</a></div>
        <div><a href="/scan">🔍 Scan Networks</a></div>
        <div class="dropdown">
            <a href="/analysis">🔬 Analysis</a>
            <div class="dropdown-content">
                <a href="/analysis">📊 Dashboard</a>
                <a href="/iperf">⚡ iPerf</a>
                <a href="/latency">📉 Latency</a>
                <a href="/channel">📡 Channel</a>
            </div>
        </div>
    </div>
    )rawliteral";
    
    // Display success/error messages
    if (webServer->hasArg("started")) {
        html += R"rawliteral(
        <div style="background: #d1fae5; padding: 15px; border-left: 4px solid #10b981; border-radius: 5px; margin: 20px 0;">
            <strong>✅ Success!</strong> Latency test started successfully.
        </div>
        )rawliteral";
    } else if (webServer->hasArg("error")) {
        html += "<div style=\"background: #fee; padding: 15px; border-left: 4px solid #f44; border-radius: 5px; margin: 20px 0;\">";
        html += "<strong>❌ Error:</strong> " + webServer->arg("error");
        html += "</div>";
    } else if (webServer->hasArg("stopped")) {
        html += R"rawliteral(
        <div style="background: #fff3cd; padding: 15px; border-left: 4px solid #ffc107; border-radius: 5px; margin: 20px 0;">
            <strong>🛑 Stopped:</strong> Latency test has been stopped.
        </div>
        )rawliteral";
    }
    
    html += R"rawliteral(
    <h2>📊 Test Status</h2>
    <div style="background: #f8f9fa; padding: 20px; border-radius: 10px; margin: 20px 0;">
    )rawliteral";
    
    // Display current status
    html += "<p><strong>Current State:</strong> ";
    switch (currentLatencyState) {
        case LATENCY_IDLE:
            html += "<span class=\"badge info\">🟡 Idle</span></p>";
            break;
        case LATENCY_RUNNING:
            html += "<span class=\"badge success\">🟢 Running</span></p>";
            break;
        case LATENCY_COMPLETED:
            html += "<span class=\"badge success\">✅ Completed</span></p>";
            break;
        case LATENCY_ERROR:
            html += "<span class=\"badge danger\">❌ Error</span></p>";
            break;
    }
    
    if (currentLatencyState == LATENCY_RUNNING) {
        html += "<p><strong>Target:</strong> " + activeLatencyConfig.target_host + "</p>";
        html += "<p><strong>Test Type:</strong> ";
        switch (activeLatencyConfig.test_type) {
            case LATENCY_UDP_ECHO:
                html += "UDP Echo";
                break;
            case LATENCY_TCP_CONNECT:
                html += "TCP Connect";
                break;
            case LATENCY_HTTP_REQUEST:
                html += "HTTP Request";
                break;
            default:
                html += "ICMP Ping";
                break;
        }
        html += "</p>";
        html += "<p><strong>Packet Count:</strong> " + String(activeLatencyConfig.packet_count) + "</p>";
        html += "<p><strong>Progress:</strong> " + String(lastLatencyResults.results_count) + " / " + String(activeLatencyConfig.packet_count) + "</p>";
    }
    
    html += "</div>";
    
    // Display last results if available
    if (lastLatencyResults.test_completed) {
        JitterStats stats = lastLatencyResults.statistics;
        
        html += R"rawliteral(
        <h2>📈 Test Results</h2>
        <div class="stat-grid">
            <div class="stat-card">
                <div class="stat-label">Average Latency</div>
                <div class="stat-value" style="font-size: 1.5em;">
        )rawliteral";
        html += String(stats.avg_latency_ms, 2) + " ms";
        html += R"rawliteral(
                </div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Min / Max Latency</div>
                <div class="stat-value" style="font-size: 1.3em;">
        )rawliteral";
        html += String(stats.min_latency_ms, 2) + " / " + String(stats.max_latency_ms, 2) + " ms";
        html += R"rawliteral(
                </div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Average Jitter</div>
                <div class="stat-value" style="font-size: 1.5em;">
        )rawliteral";
        html += String(stats.jitter_ms, 2) + " ms";
        html += R"rawliteral(
                </div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Packet Loss</div>
                <div class="stat-value" style="font-size: 1.5em;">
        )rawliteral";
        html += String(stats.packet_loss_percent, 1) + "%";
        html += R"rawliteral(
                </div>
            </div>
        </div>
        
        <div style="background: #f8f9fa; padding: 20px; border-radius: 10px; margin: 20px 0;">
            <p><strong>Packets Sent:</strong> )rawliteral";
        html += String(stats.packets_sent);
        html += R"rawliteral(</p>
            <p><strong>Packets Received:</strong> )rawliteral";
        html += String(stats.packets_received);
        html += R"rawliteral(</p>
            <p><strong>Packets Lost:</strong> )rawliteral";
        html += String(stats.packets_lost);
        html += R"rawliteral(</p>
            <p><strong>Test Duration:</strong> )rawliteral";
        html += String(lastLatencyResults.test_duration_ms / 1000.0, 2) + " seconds";
        html += R"rawliteral(</p>
        </div>
        )rawliteral";
    }
    
    // Control buttons
    html += R"rawliteral(
    <h2>🎮 Test Control</h2>
    <div style="display: flex; gap: 15px; justify-content: center; margin: 30px 0; flex-wrap: wrap;">
    )rawliteral";
    
    if (currentLatencyState == LATENCY_RUNNING) {
        html += R"rawliteral(
        <button onclick="if(confirm('Stop the running latency test?')) location.href='/latency/stop'" style="padding: 15px 30px; background: #ef4444; color: white; border: none; border-radius: 5px; font-size: 1.1em; cursor: pointer; font-weight: bold;">
            🛑 Stop Test
        </button>
        <button onclick="location.reload()" style="padding: 15px 30px; background: #3b82f6; color: white; border: none; border-radius: 5px; font-size: 1.1em; cursor: pointer; font-weight: bold;">
            🔄 Refresh Status
        </button>
        )rawliteral";
    } else {
        html += R"rawliteral(
        <button onclick="location.href='/latency/start'" style="padding: 15px 30px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border: none; border-radius: 5px; font-size: 1.1em; cursor: pointer; font-weight: bold;">
            🚀 Start New Test
        </button>
        )rawliteral";
    }
    
    html += R"rawliteral(
    </div>

    <h2>ℹ️ About Latency Testing</h2>
    <ul style="margin: 15px 0 15px 30px; line-height: 1.8;">
        <li><strong>UDP Echo:</strong> Fastest test method with minimal overhead</li>
        <li><strong>TCP Connect:</strong> Measures connection establishment time</li>
        <li><strong>HTTP Request:</strong> Real-world application latency testing</li>
        <li><strong>Jitter:</strong> Variation in latency between packets (critical for VoIP/gaming)</li>
        <li><strong>Packet Loss:</strong> Percentage of packets that failed to arrive</li>
    </ul>
    
    <div style="background: #e3f2fd; padding: 20px; border-left: 4px solid #2196f3; border-radius: 5px; margin: 20px 0;">
        <strong>💡 Quick Start:</strong><br><br>
        Click "Start New Test", enter your target host (e.g., google.com or 8.8.8.8), select test type, configure parameters, and start testing. 
        Results will show average latency, jitter, and packet loss metrics.
    </div>
    )rawliteral";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleLatencyStart() {
    // Handle POST request to start latency test
    if (webServer->method() == HTTP_POST) {
        String targetHost = webServer->arg("target");
        String testType = webServer->arg("testType");
        String packetCount = webServer->arg("packetCount");
        String interval = webServer->arg("interval");
        
        // Create configuration
        LatencyConfig config;
        config.target_host = targetHost;
        config.packet_count = packetCount.length() > 0 ? packetCount.toInt() : PING_DEFAULT_COUNT;
        config.interval_ms = interval.length() > 0 ? interval.toInt() : PING_DEFAULT_INTERVAL;
        config.timeout_ms = PING_DEFAULT_TIMEOUT;
        config.packet_size = 32;
        config.continuous_mode = false;
        
        if (testType == "udp") {
            config.test_type = LATENCY_UDP_ECHO;
            config.target_port = 7; // Echo port
        } else if (testType == "tcp") {
            config.test_type = LATENCY_TCP_CONNECT;
            config.target_port = 80;
        } else if (testType == "http") {
            config.test_type = LATENCY_HTTP_REQUEST;
            config.target_port = 80;
        } else {
            config.test_type = LATENCY_UDP_ECHO;
            config.target_port = 7;
        }
        
        // Validate and start
        bool success = false;
        String errorMsg = "";
        
        if (targetHost.length() == 0) {
            errorMsg = "Target host required";
        } else {
            // Stop previous test if running
            if (currentLatencyState == LATENCY_RUNNING) {
                Serial.println("🛑 Stopping previous latency test before starting new one...");
                stopLatencyTest();
                delay(100); // Brief delay to ensure clean stop
            }
            
            success = startLatencyTest(config);
            
            if (!success) {
                errorMsg = "Failed to start latency test. Check configuration and network connection.";
            }
        }
        
        // Redirect with status
        if (success) {
            webServer->sendHeader("Location", "/latency?started=1", true);
        } else {
            webServer->sendHeader("Location", "/latency?error=" + errorMsg, true);
        }
        webServer->send(302, "text/plain", "");
        return;
    }
    
    // Show configuration form (GET request)
    String html = HTML_HEADER;
    
    html += R"rawliteral(
    <style>
        .form-group {
            margin: 20px 0;
        }
        .form-group label {
            display: block;
            font-weight: bold;
            margin-bottom: 5px;
            color: #333;
        }
        .form-group input, .form-group select {
            width: 100%;
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 5px;
            font-size: 1em;
            box-sizing: border-box;
        }
        .form-group input:focus, .form-group select:focus {
            outline: none;
            border-color: #667eea;
        }
        .form-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }
        .submit-btn {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 15px 40px;
            border: none;
            border-radius: 5px;
            font-size: 1.2em;
            font-weight: bold;
            cursor: pointer;
            width: 100%;
            margin-top: 20px;
        }
        .submit-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
        }
        .info-box {
            background: #e3f2fd;
            padding: 15px;
            border-left: 4px solid #2196f3;
            border-radius: 5px;
            margin: 20px 0;
        }
        @media (max-width: 768px) {
            .form-row {
                grid-template-columns: 1fr;
            }
        }
    </style>
    
    <div class="header">
        <h1>📉 Start Latency Test</h1>
        <p>Configure and Launch Network Quality Test</p>
    </div>

    <div class="nav">
        <div><a href="/">🏠 Home</a></div>
        <div><a href="/status">📊 Status</a></div>
        <div><a href="/scan">🔍 Scan Networks</a></div>
        <div class="dropdown">
            <a href="/analysis">🔬 Analysis</a>
            <div class="dropdown-content">
                <a href="/analysis">📊 Dashboard</a>
                <a href="/iperf">⚡ iPerf</a>
                <a href="/latency">📉 Latency</a>
                <a href="/channel">📡 Channel</a>
            </div>
        </div>
    </div>

    <h2>🔧 Test Configuration</h2>
    
    <form method="POST" action="/latency/start">
        <div class="form-group">
            <label for="target">Target Host (IP or Domain)</label>
            <input type="text" id="target" name="target" placeholder="e.g., google.com or 8.8.8.8" required>
            <small style="color: #666;">Enter IP address or domain name to test</small>
        </div>
        
        <div class="form-group">
            <label for="testType">Test Type</label>
            <select id="testType" name="testType" required>
                <option value="udp">UDP Echo (Fast, Low Overhead)</option>
                <option value="tcp">TCP Connect (Connection Time)</option>
                <option value="http">HTTP Request (Real-World Latency)</option>
            </select>
        </div>
        
        <div class="form-row">
            <div class="form-group">
                <label for="packetCount">Packet Count</label>
                <input type="number" id="packetCount" name="packetCount" value="10" min="1" max="100" required>
            </div>
            
            <div class="form-group">
                <label for="interval">Interval (ms)</label>
                <input type="number" id="interval" name="interval" value="1000" min="100" max="10000" required>
            </div>
        </div>
        
        <div class="info-box">
            <strong>ℹ️ Test Types:</strong><br>
            • <strong>UDP Echo:</strong> Fastest method, measures round-trip time with minimal protocol overhead<br>
            • <strong>TCP Connect:</strong> Measures time to establish TCP connection<br>
            • <strong>HTTP Request:</strong> Real-world application latency including HTTP overhead
        </div>
        
        <button type="submit" class="submit-btn">🚀 Start Latency Test</button>
    </form>
    )rawliteral";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleLatencyStop() {
    stopLatencyTest();
    
    // Redirect back to main latency page with stopped message
    webServer->sendHeader("Location", "/latency?stopped=1", true);
    webServer->send(302, "text/plain", "");
}

#endif // USE_WEBSERVER
