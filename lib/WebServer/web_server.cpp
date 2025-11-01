#include "web_server.h"

#ifdef USE_WEBSERVER  // Web server available when enabled

#include "wifi_manager.h"
#include "ap_manager.h"
#include "ap_config.h"
#include "station_config.h"
#include "channel_analyzer.h"
#include "iperf_manager.h"
#include "latency_analyzer.h"
#include "signal_monitor.h"
#include "port_scanner.h"
#include <qrcode.h>

// Web server instance
WebServer* webServer = nullptr;
bool webServerEnabled = false;

// ==========================================
// SCAN RESULT CACHING
// ==========================================
// Structure to cache scan results for details view
struct CachedScanResult {
    String ssid;
    int32_t rssi;
    uint8_t channel;
    wifi_auth_mode_t encryptionType;
    uint8_t bssid[6];
    bool hasBssid;
};

#define MAX_CACHED_NETWORKS 50
#define CACHE_TIMEOUT_MS 300000  // 5 minutes

static CachedScanResult cachedNetworks[MAX_CACHED_NETWORKS];
static int cachedNetworkCount = 0;
static unsigned long lastScanTime = 0;

// Check if cached results are still valid
bool isCacheValid() {
    return (cachedNetworkCount > 0 && (millis() - lastScanTime < CACHE_TIMEOUT_MS));
}

// Cache scan results
void cacheScanResults() {
    cachedNetworkCount = WiFi.scanComplete();
    if (cachedNetworkCount <= 0) return;
    
    if (cachedNetworkCount > MAX_CACHED_NETWORKS) {
        cachedNetworkCount = MAX_CACHED_NETWORKS;
    }
    
    for (int i = 0; i < cachedNetworkCount; i++) {
        cachedNetworks[i].ssid = WiFi.SSID(i);
        cachedNetworks[i].rssi = WiFi.RSSI(i);
        cachedNetworks[i].channel = WiFi.channel(i);
        cachedNetworks[i].encryptionType = WiFi.encryptionType(i);
        
        uint8_t* bssid = WiFi.BSSID(i);
        if (bssid) {
            memcpy(cachedNetworks[i].bssid, bssid, 6);
            cachedNetworks[i].hasBssid = true;
        } else {
            cachedNetworks[i].hasBssid = false;
        }
    }
    
    lastScanTime = millis();
}

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

// HTML page styling - Stored in PROGMEM (flash) to save RAM
const char HTML_HEADER[] PROGMEM = R"rawliteral(
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
h2{color:#764ba2;margin:30px 0 15px;font-size:1.5em;border-bottom:2px solid #667eea;padding-bottom:10px}
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
button,.btn{padding:10px 20px;background:#667eea;color:#fff;border:none;border-radius:5px;cursor:pointer;font-size:1em;font-weight:500;transition:all .3s;text-decoration:none;display:inline-block}
button:hover,.btn:hover{background:#764ba2;transform:translateY(-2px);box-shadow:0 4px 8px rgba(0,0,0,.2)}
button:active,.btn:active{transform:translateY(0);box-shadow:0 2px 4px rgba(0,0,0,.2)}
.footer{text-align:center;margin-top:30px;padding-top:20px;border-top:1px solid #e0e0e0;color:#666;font-size:.9em}
.nav{display:flex;gap:10px;justify-content:center;flex-wrap:wrap;margin:20px 0;position:relative;align-items:center}
.nav>div{position:relative}
.nav a{text-decoration:none;padding:10px 20px;background:#667eea;color:#fff;border-radius:5px;transition:all .3s;display:block;white-space:nowrap}
.nav a:hover{background:#764ba2;transform:translateY(-2px);box-shadow:0 4px 8px rgba(0,0,0,.2)}
.dropdown{position:relative;display:inline-block}
.dropdown-content{display:none;position:absolute;background-color:#667eea;min-width:200px;box-shadow:0 8px 16px rgba(0,0,0,.3);z-index:1000;border-radius:5px;margin-top:5px;left:0}
.dropdown-content a{color:#fff;padding:12px 16px;text-decoration:none;display:block;border-radius:0;margin:0}
.dropdown-content a:first-child{border-radius:5px 5px 0 0}
.dropdown-content a:last-child{border-radius:0 0 5px 5px}
.dropdown-content a:hover{background-color:#764ba2;transform:none}
.dropdown:hover .dropdown-content{display:block}
.dropdown>a::after{content:' ▼';font-size:.8em}
.hamburger{display:none;flex-direction:column;cursor:pointer;padding:10px;background:#667eea;border-radius:5px;position:absolute;right:0;top:0;z-index:1001}
.hamburger span{width:25px;height:3px;background:#fff;margin:3px 0;border-radius:2px;transition:all .3s}
.hamburger.active span:nth-child(1){transform:rotate(45deg) translate(7px,7px)}
.hamburger.active span:nth-child(2){opacity:0}
.hamburger.active span:nth-child(3){transform:rotate(-45deg) translate(7px,-7px)}
.nav-items{display:flex;gap:10px;flex-wrap:wrap;justify-content:center;width:100%}
.progress-backdrop{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,.7);z-index:9999;justify-content:center;align-items:center}
.progress-container{background:#fff;padding:30px;border-radius:15px;box-shadow:0 10px 40px rgba(0,0,0,.3);text-align:center;min-width:300px;max-width:500px}
.progress-title{color:#667eea;font-size:1.5em;margin-bottom:20px;font-weight:bold}
.spinner{border:4px solid #f3f3f3;border-top:4px solid #667eea;border-radius:50%;width:40px;height:40px;animation:spin 1s linear infinite;margin:20px auto}
@keyframes spin{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}
@media(max-width:768px){
body{padding:10px}
.container{padding:15px;border-radius:10px}
h1{font-size:1.5em}
h2{font-size:1.2em}
.stat-grid{grid-template-columns:1fr}
.hamburger{display:flex}
.nav{padding-top:50px}
.nav-items{display:none;flex-direction:column;width:100%;gap:5px}
.nav-items.active{display:flex}
.nav>div{width:100%}
.nav a{padding:12px 15px;text-align:center}
.dropdown{width:100%}
.dropdown>a{width:100%}
.dropdown-content{position:static;margin-top:5px;width:100%;box-shadow:none;background:#5a6dd8}
.dropdown-content a{padding-left:30px}
.dropdown:hover .dropdown-content,.dropdown:focus-within .dropdown-content{display:none}
.dropdown.active .dropdown-content{display:block}
.dropdown>a::after{float:right}
}
@media(min-width:769px) and (max-width:1024px){
.container{padding:20px}
.nav{gap:8px}
.nav a{padding:8px 15px;font-size:0.9em}
}
</style>
<script>
function showProgress(t,m){const b=document.getElementById('progressBackdrop'),ti=document.getElementById('progressTitle'),me=document.getElementById('progressMessage');if(ti)ti.textContent=t;if(me)me.textContent=m;if(b)b.style.display='flex'}
function hideProgress(){const b=document.getElementById('progressBackdrop');if(b)b.style.display='none'}
function startScan(u,t,m){showProgress(t,m);window.location.href=u}
function toggleMenu(){const hamburger=document.querySelector('.hamburger');const navItems=document.querySelector('.nav-items');hamburger.classList.toggle('active');navItems.classList.toggle('active')}
function toggleDropdown(e){if(window.innerWidth<=768){e.preventDefault();const dropdown=e.target.closest('.dropdown');dropdown.classList.toggle('active')}}
document.addEventListener('DOMContentLoaded',function(){const dropdownLinks=document.querySelectorAll('.dropdown>a');dropdownLinks.forEach(link=>{link.addEventListener('click',toggleDropdown)})})
</script>
</head>
<body>
<div id="progressBackdrop" class="progress-backdrop"><div class="progress-container"><div id="progressTitle" class="progress-title">Scanning...</div><div class="spinner"></div><div id="progressMessage" class="progress-message">Please wait...</div></div></div>
<div class="container">
)rawliteral";

// Generate dynamic HTML footer - optimized with F() macro
String generateHtmlFooter() {
    String footer;
    footer.reserve(256);
    footer = F("<div class=\"footer\"><p>🚀 ESP32 WiFi v");
    footer += getVersionString();
    footer += F(" | ");
    footer += getDeviceName();
    footer += F("</p></div></div></body></html>");
    return footer;
}

// Generate common navigation menu - stored in PROGMEM
const char NAV_MENU[] PROGMEM = "<div class=\"nav\"><div class=\"hamburger\" onclick=\"toggleMenu()\"><span></span><span></span><span></span></div><div class=\"nav-items\"><div><a href=\"/\">🏠 Home</a></div><div><a href=\"/status\">📊 Status</a></div><div><a href=\"/scan\">🔍 Scan</a></div><div><a href=\"/config\">⚙️ Config</a></div><div class=\"dropdown\"><a href=\"/analysis\">🔬 Analysis</a><div class=\"dropdown-content\"><a href=\"/analysis\">📊 Dashboard</a><a href=\"/signal\">📶 Signal</a><a href=\"/portscan\">🔒 Port Scanner</a><a href=\"/iperf\">⚡ iPerf</a><a href=\"/latency\">📉 Latency</a><a href=\"/channel\">📡 Channel</a></div></div></div></div>";

String generateNav() {
    return FPSTR(NAV_MENU);
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
    webServer->on("/scan/details", handleScanDetails);  // New route for network details
    webServer->on("/analysis", handleNetworkAnalysis);
    webServer->on("/channel", handleChannelAnalysis);
    webServer->on("/channel/scan", handleChannelScan);
    webServer->on("/channel/graph", handleChannelGraph);
    webServer->on("/latency", handleLatency);
    webServer->on("/latency/start", handleLatencyStart);
    webServer->on("/latency/stop", handleLatencyStop);
    webServer->on("/iperf", handleIperf);
    webServer->on("/iperf/start", handleIperfStart);
    webServer->on("/iperf/stop", handleIperfStop);
    webServer->on("/iperf/results", handleIperfResults);
    webServer->on("/config", handleConfig);
    webServer->on("/config/ap", HTTP_POST, handleConfigAP);
    webServer->on("/config/station", HTTP_POST, handleConfigStation);
    webServer->on("/config/clear", HTTP_POST, handleConfigClear);
    webServer->on("/reboot", HTTP_POST, handleReboot);
    webServer->on("/mode/switch", HTTP_POST, handleModeSwitch);
    webServer->on("/signal", handleSignalMonitor);
    webServer->on("/signal/api", handleSignalStrengthAPI);
    webServer->on("/portscan", handlePortScanner);
    webServer->on("/portscan/start", handlePortScanStart);
    webServer->on("/portscan/stop", handlePortScanStop);
    webServer->on("/portscan/status", handlePortScanStatus);
    webServer->on("/portscan/api", handlePortScanAPI);
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
    String html;
    html.reserve(4096);  // Pre-allocate to reduce heap fragmentation
    html = FPSTR(HTML_HEADER);  // Read from PROGMEM
    
    html += F("<div class=\"header\"><h1>🚀 ESP32 WiFi</h1><p>Network Analysis & Testing</p><div><span class=\"badge info\">");
    html += getDeviceName();
    html += F("</span>");
    
    if (currentMode == MODE_AP) {
        html += F("<span class=\"badge success\">Access Point Mode</span>");
    } else if (WiFi.status() == WL_CONNECTED) {
        html += F("<span class=\"badge success\">Connected</span>");
    } else {
        html += F("<span class=\"badge warning\">Station Mode</span>");
    }
    
    html += F("</div></div>");
    html += generateNav();
    html += F("<h2>📊 Stats</h2><div class=\"stat-grid\"><div class=\"stat-card\"><div class=\"stat-label\">Mode</div><div class=\"stat-value\">");
    
    switch (currentMode) {
        case MODE_IDLE: html += F("IDLE"); break;
        case MODE_STATION: html += F("STATION"); break;
        case MODE_AP: html += F("AP"); break;
        case MODE_OFF: html += F("OFF"); break;
    }
    
    html += F("</div></div><div class=\"stat-card\"><div class=\"stat-label\">IP Address</div><div class=\"stat-value\" style=\"font-size:1.2em\">");
    
    if (currentMode == MODE_AP) {
        html += WiFi.softAPIP().toString();
    } else if (WiFi.status() == WL_CONNECTED) {
        html += WiFi.localIP().toString();
    } else {
        html += F("N/A");
    }
    
    html += F("</div></div><div class=\"stat-card\"><div class=\"stat-label\">Free Heap</div><div class=\"stat-value\">");
    html += ESP.getFreeHeap() / 1024;
    html += F(" KB</div></div><div class=\"stat-card\"><div class=\"stat-label\">Clients</div><div class=\"stat-value\">");
    
    if (currentMode == MODE_AP) {
        html += WiFi.softAPgetStationNum();
    } else {
        html += F("N/A");
    }
    
    html += F("</div></div></div>");
    
    // QR Code for AP mode
    if (currentMode == MODE_AP) {
        html += F("<h2>📱 Connect</h2><div style=\"background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);padding:30px;border-radius:10px;text-align:center;color:#fff\"><p style=\"font-size:1.3em;margin-bottom:20px;font-weight:bold\">📷 Scan QR Code</p><div style=\"display:inline-block;background:#fff;padding:15px;border-radius:10px\">");
        html += F("<div style='width:250px;height:250px'>");
        html += generateQRCodeSVG(currentAPSSID, currentAPPassword, "WPA");
        html += F("</div></div><div style=\"margin-top:20px\"><p><strong>Network:</strong> ");
        html += currentAPSSID;
        html += F("</p></div></div>");
    }

    html += F("<h2>ℹ️ Features</h2><ul style=\"margin:15px 0 15px 30px;line-height:1.8\"><li>📡 <strong>Channel Analysis</strong>: 2.4GHz spectrum scanning</li><li>🔍 <strong>Network Scanning</strong>: WiFi discovery</li><li>⚡ <strong>iPerf Testing</strong>: Bandwidth measurement</li><li>📊 <strong>Latency Analysis</strong>: Jitter & packet loss testing</li><li>🌐 <strong>Access Point</strong>: WiFi hotspot with QR</li></ul>");
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleStatus() {
    String html;
    html.reserve(4096);  // Pre-allocate to reduce heap fragmentation
    html = FPSTR(HTML_HEADER);  // Read from PROGMEM
    
    html += F("<div class=\"header\"><h1>📊 Status</h1></div>");
    html += generateNav();
    html += F("<h2>🔧 System Info</h2><div style=\"background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0\">");
    
    html += F("<p><strong>WiFi Mode:</strong> ");
    switch (currentMode) {
        case MODE_IDLE:
            html += F("⚪ Idle (Ready)");
            break;
        case MODE_STATION:
            html += F("🔍 Station (Scanner)");
            break;
        case MODE_AP:
            html += F("📡 Access Point");
            break;
        case MODE_OFF:
            html += F("🔴 Disabled");
            break;
    }
    html += F("</p>");
    
    html += F("<p><strong>Scanning:</strong> ");
    html += scanningEnabled ? F("✅ Enabled") : F("❌ Disabled");
    html += F("</p>");
    
    if (currentMode == MODE_AP) {
        html += F("<p><strong>AP SSID:</strong> ");
        html += currentAPSSID;
        html += F("</p><p><strong>AP IP:</strong> ");
        html += WiFi.softAPIP().toString();
        html += F("</p><p><strong>Connected Clients:</strong> ");
        html += WiFi.softAPgetStationNum();
        html += F("</p>");
    } else if (WiFi.status() == WL_CONNECTED) {
        html += F("<p><strong>Connected to:</strong> ");
        html += WiFi.SSID();
        html += F("</p><p><strong>IP Address:</strong> ");
        html += WiFi.localIP().toString();
        html += F("</p><p><strong>Signal Strength:</strong> ");
        html += WiFi.RSSI();
        html += F(" dBm</p>");
    }
    
    html += F("<p><strong>Free Heap:</strong> ");
    html += ESP.getFreeHeap();
    html += F(" bytes</p><p><strong>Chip Model:</strong> ");
    html += ESP.getChipModel();
    html += F("</p><p><strong>CPU Frequency:</strong> ");
    html += ESP.getCpuFreqMHz();
    html += F(" MHz</p><p><strong>Flash Size:</strong> ");
    html += ESP.getFlashChipSize() / 1024 / 1024;
    html += F(" MB</p></div>");
    
    // QR Code for AP mode
    if (currentMode == MODE_AP) {
        html += F("<h2>📱 Connect</h2><div style=\"background:#f8f9fa;padding:30px;border-radius:10px;text-align:center\"><p style=\"font-size:1.2em;margin-bottom:20px;color:#667eea;font-weight:bold\">Scan QR to connect</p><div style=\"display:inline-block;background:#fff;padding:20px;border-radius:10px\">");
        html += F("<div style='width:300px;height:300px'>");
        html += generateQRCodeSVG(currentAPSSID, currentAPPassword, "WPA");
        html += F("</div></div><div style=\"margin-top:20px;padding:20px;background:#fff;border-radius:8px;display:inline-block;text-align:left\"><p><strong>SSID:</strong> ");
        html += currentAPSSID;
        html += F("</p><p><strong>Password:</strong> ");
        html += currentAPPassword;
        html += F("</p><p><strong>Security:</strong> WPA2</p><p><strong>IP:</strong> ");
        html += WiFi.softAPIP().toString();
        html += F("</p></div></div>");
    }
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

// Scan page header - stored in PROGMEM
const char SCAN_HEADER[] PROGMEM = R"rawliteral(<div class="header"><h1>🔍 Network Scan</h1></div><h2>📡 Available Networks</h2><div style="text-align:center;margin:20px 0"><button onclick="startScan('/scan?doscan=1','🔍 Scanning Networks...','Please wait while we discover nearby WiFi networks')" style="padding:15px 40px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;border:none;border-radius:8px;font-size:1.1em;font-weight:bold;cursor:pointer;box-shadow:0 4px 12px rgba(102,126,234,0.4)">🔍 Start Network Scan</button></div>)rawliteral";

void handleScan() {
    String html;
    html.reserve(8192);  // Pre-allocate for scan results
    html = FPSTR(HTML_HEADER);
    html += generateNav();
    html += FPSTR(SCAN_HEADER);
    
    // Check if scan was requested
    if (webServer->hasArg("doscan")) {
        // Perform WiFi scan
        int n = WiFi.scanNetworks();
        
        // Cache the results for details view
        cacheScanResults();
        
        if (n == 0) {
            html += F("<p style='text-align:center;padding:40px;color:#666'>No networks found. Try scanning again.</p>");
        } else {
            html += F("<ul class=\"network-list\">");
            
            for (int i = 0; i < n; i++) {
            // Make the entire list item clickable
            html += F("<li class=\"network-item\" onclick=\"window.location.href='/scan/details?id=");
            html += i;
            html += F("'\" style=\"cursor:pointer;transition:background-color 0.2s\" onmouseover=\"this.style.backgroundColor='#f0f0f0'\" onmouseout=\"this.style.backgroundColor='#f8f9fa'\"><div class=\"network-info\"><div class=\"network-name\">");
            html += WiFi.SSID(i).length() > 0 ? WiFi.SSID(i) : F("<Hidden Network>");
            html += F("</div><div class=\"network-details\">Channel: ");
            html += WiFi.channel(i);
            html += F(" | Security: ");
            html += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? F("Open") : F("Secured");
            html += F("</div></div>");
            
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
            html += F("<div class=\"signal-strength\" title=\"");
            html += rssi;
            html += F(" dBm\" style=\"display:flex;gap:3px;align-items:center\">");
            for (int j = 0; j < 5; j++) {
                if (j < bars) {
                    html += F("<svg width=\"14\" height=\"14\" style=\"display:block\"><circle cx=\"7\" cy=\"7\" r=\"6\" fill=\"");
                    html += color;
                    html += F("\"/></svg>");
                } else {
                    html += F("<svg width=\"14\" height=\"14\" style=\"display:block\"><circle cx=\"7\" cy=\"7\" r=\"6\" fill=\"none\" stroke=\"#d1d5db\" stroke-width=\"2\"/></svg>");
                }
            }
            html += F("<span style=\"margin-left:8px;color:#666;font-size:0.9em\">");
            html += rssi;
            html += F(" dBm</span></div></li>");
        }
        
        html += F("</ul><p style='text-align:center;margin-top:20px'><strong>Found ");
        html += n;
        html += F(" network(s)</strong></p>");
        html += F("<p style='text-align:center;color:#666;font-size:0.9em;margin-top:10px'>💡 Click on any network to view detailed information</p>");
        }
        
        // Don't delete scan results yet - keep them cached for details view
        // WiFi.scanDelete();
    } else {
        html += F("<p style='text-align:center;padding:40px;color:#999'>Click the button above to scan for available WiFi networks.</p>");
    }
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

// ==========================================
// NETWORK DETAILS PAGE HANDLER
// ==========================================
void handleScanDetails() {
    // Check if cache is valid
    if (!isCacheValid()) {
        // Redirect back to scan page if cache expired
        webServer->sendHeader("Location", "/scan");
        webServer->send(302, "text/plain", "Scan results expired. Please scan again.");
        return;
    }
    
    // Get network ID from query parameter
    if (!webServer->hasArg("id")) {
        webServer->sendHeader("Location", "/scan");
        webServer->send(302, "text/plain", "Missing network ID");
        return;
    }
    
    int networkId = webServer->arg("id").toInt();
    
    // Validate network ID
    if (networkId < 0 || networkId >= cachedNetworkCount) {
        webServer->sendHeader("Location", "/scan");
        webServer->send(302, "text/plain", "Invalid network ID");
        return;
    }
    
    // Get cached network data
    CachedScanResult& network = cachedNetworks[networkId];
    
    String html;
    html.reserve(8192);
    html = FPSTR(HTML_HEADER);
    
    // Header with back button
    html += F("<div class=\"header\"><a href=\"/scan?doscan=1\" style=\"position:absolute;left:30px;top:30px;color:#667eea;text-decoration:none;font-weight:bold;font-size:1.1em\">← Back to Scan</a><h1>🔍 Network Details</h1></div>");
    
    html += generateNav();
    
    // Network name section
    html += F("<h2>📡 Network Information</h2>");
    html += F("<div style=\"background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0\">");
    html += F("<p><strong>Network Name (SSID):</strong> ");
    html += network.ssid.length() > 0 ? network.ssid : F("<em>Hidden Network</em>");
    html += F("</p>");
    
    // BSSID (MAC Address)
    html += F("<p><strong>MAC Address (BSSID):</strong> ");
    if (network.hasBssid) {
        char bssidStr[18];
        sprintf(bssidStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
                network.bssid[0], network.bssid[1], network.bssid[2], 
                network.bssid[3], network.bssid[4], network.bssid[5]);
        html += bssidStr;
    } else {
        html += F("<em>Not Available</em>");
    }
    html += F("</p></div>");
    
    // Signal Strength section
    html += F("<h2>📶 Signal Strength</h2>");
    html += F("<div style=\"background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0\">");
    html += F("<p><strong>RSSI:</strong> ");
    html += network.rssi;
    html += F(" dBm</p>");
    
    // Calculate signal quality
    int quality = 0;
    String qualityDesc = "";
    String signalIcon = "";
    String qualityColor = "";
    
    if (network.rssi >= -30) {
        quality = 100; qualityDesc = F("Excellent (Very Close)"); signalIcon = F("🟢🟢🟢🟢"); qualityColor = "#10b981";
    } else if (network.rssi >= -50) {
        quality = 90; qualityDesc = F("Excellent"); signalIcon = F("🟢🟢🟢🟢"); qualityColor = "#10b981";
    } else if (network.rssi >= -60) {
        quality = 80; qualityDesc = F("Very Good"); signalIcon = F("🟢🟢🟢⚪"); qualityColor = "#10b981";
    } else if (network.rssi >= -67) {
        quality = 70; qualityDesc = F("Good"); signalIcon = F("🟢🟢🟡⚪"); qualityColor = "#10b981";
    } else if (network.rssi >= -70) {
        quality = 60; qualityDesc = F("Fair"); signalIcon = F("🟢🟡🟡⚪"); qualityColor = "#fbbf24";
    } else if (network.rssi >= -80) {
        quality = 50; qualityDesc = F("Weak"); signalIcon = F("🟡🟡🔴⚪"); qualityColor = "#f59e0b";
    } else if (network.rssi >= -90) {
        quality = 30; qualityDesc = F("Very Weak"); signalIcon = F("🟡🔴🔴⚪"); qualityColor = "#ef4444";
    } else {
        quality = 10; qualityDesc = F("Extremely Weak"); signalIcon = F("🔴🔴🔴⚪"); qualityColor = "#ef4444";
    }
    
    html += F("<p><strong>Signal Quality:</strong> <span style=\"color:");
    html += qualityColor;
    html += F(";font-weight:bold\">");
    html += quality;
    html += F("% - ");
    html += qualityDesc;
    html += F("</span> ");
    html += signalIcon;
    html += F("</p>");
    
    // Visual quality bar
    html += F("<div style=\"background:#e0e0e0;height:30px;border-radius:15px;overflow:hidden;margin-top:15px\"><div style=\"background:");
    html += qualityColor;
    html += F(";height:100%;width:");
    html += quality;
    html += F("%;transition:width 0.3s;display:flex;align-items:center;justify-content:center;color:white;font-weight:bold\">");
    html += quality;
    html += F("%</div></div></div>");
    
    // Channel Information section
    html += F("<h2>📻 Channel Information</h2>");
    html += F("<div style=\"background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0\">");
    html += F("<p><strong>Channel:</strong> ");
    html += network.channel;
    
    // Determine frequency band
    String bandInfo = "";
    if (network.channel >= 1 && network.channel <= 13) {
        bandInfo = F(" (2.4 GHz)");
    } else if (network.channel >= 36 && network.channel <= 165) {
        bandInfo = F(" (5 GHz)");
    }
    html += bandInfo;
    html += F("</p>");
    
    // Calculate channel congestion
    int channelUsage = 0;
    for (int i = 0; i < cachedNetworkCount; i++) {
        if (cachedNetworks[i].channel == network.channel) {
            channelUsage++;
        }
    }
    
    String congestionLevel = "";
    String congestionColor = "";
    if (channelUsage == 1) {
        congestionLevel = F("Clear"); congestionColor = "#10b981";
    } else if (channelUsage <= 3) {
        congestionLevel = F("Light"); congestionColor = "#10b981";
    } else if (channelUsage <= 6) {
        congestionLevel = F("Moderate"); congestionColor = "#fbbf24";
    } else if (channelUsage <= 10) {
        congestionLevel = F("Heavy"); congestionColor = "#fb923c";
    } else {
        congestionLevel = F("Severe"); congestionColor = "#ef4444";
    }
    
    html += F("<p><strong>Channel Congestion:</strong> <span style=\"color:");
    html += congestionColor;
    html += F(";font-weight:bold\">");
    html += congestionLevel;
    html += F("</span> (");
    html += channelUsage;
    html += F(" network");
    if (channelUsage != 1) html += F("s");
    html += F(" on this channel)</p></div>");
    
    // Security Information section
    html += F("<h2>🔐 Security</h2>");
    html += F("<div style=\"background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0\">");
    
    String encIcon = "";
    String encDescription = "";
    String securityLevel = "";
    String securityColor = "";
    
    switch (network.encryptionType) {
        case WIFI_AUTH_OPEN:
            encIcon = F("🔓"); encDescription = F("Open (No Security)"); 
            securityLevel = F("None"); securityColor = "#ef4444";
            break;
        case WIFI_AUTH_WEP:
            encIcon = F("🔒"); encDescription = F("WEP (Deprecated)"); 
            securityLevel = F("Weak"); securityColor = "#f59e0b";
            break;
        case WIFI_AUTH_WPA_PSK:
            encIcon = F("🔒"); encDescription = F("WPA Personal"); 
            securityLevel = F("Moderate"); securityColor = "#fb923c";
            break;
        case WIFI_AUTH_WPA2_PSK:
            encIcon = F("🔒"); encDescription = F("WPA2 Personal"); 
            securityLevel = F("Good"); securityColor = "#10b981";
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            encIcon = F("🔒"); encDescription = F("WPA/WPA2 Mixed"); 
            securityLevel = F("Good"); securityColor = "#10b981";
            break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            encIcon = F("🏢"); encDescription = F("WPA2 Enterprise"); 
            securityLevel = F("Excellent"); securityColor = "#10b981";
            break;
        case WIFI_AUTH_WPA3_PSK:
            encIcon = F("🔐"); encDescription = F("WPA3 Personal"); 
            securityLevel = F("Excellent"); securityColor = "#10b981";
            break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            encIcon = F("🔐"); encDescription = F("WPA2/WPA3 Mixed"); 
            securityLevel = F("Excellent"); securityColor = "#10b981";
            break;
        case WIFI_AUTH_WAPI_PSK:
            encIcon = F("🔒"); encDescription = F("WAPI"); 
            securityLevel = F("Good"); securityColor = "#10b981";
            break;
        default:
            encIcon = F("❓"); encDescription = F("Unknown"); 
            securityLevel = F("Unknown"); securityColor = "#666";
    }
    
    html += F("<p><strong>Encryption Type:</strong> ");
    html += encIcon;
    html += F(" ");
    html += encDescription;
    html += F("</p>");
    html += F("<p><strong>Security Level:</strong> <span style=\"color:");
    html += securityColor;
    html += F(";font-weight:bold\">");
    html += securityLevel;
    html += F("</span></p>");
    
    // Security warning for open/weak networks
    if (network.encryptionType == WIFI_AUTH_OPEN) {
        html += F("<div style=\"background:#fef2f2;border-left:4px solid #ef4444;padding:15px;margin-top:15px;border-radius:5px\">");
        html += F("<p style=\"color:#ef4444;margin:0\"><strong>⚠️ Security Warning:</strong> This is an open network with no encryption. Your data will be transmitted unencrypted and could be intercepted by others.</p>");
        html += F("</div>");
    } else if (network.encryptionType == WIFI_AUTH_WEP) {
        html += F("<div style=\"background:#fef2f2;border-left:4px solid #f59e0b;padding:15px;margin-top:15px;border-radius:5px\">");
        html += F("<p style=\"color:#f59e0b;margin:0\"><strong>⚠️ Security Warning:</strong> WEP encryption is deprecated and easily cracked. This network is not secure.</p>");
        html += F("</div>");
    }
    html += F("</div>");
    
    // Connection Analysis section
    html += F("<h2>💡 Connection Analysis</h2>");
    html += F("<div style=\"background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0\">");
    
    // Signal strength analysis
    if (network.rssi >= -70) {
        html += F("<p>✅ <strong>Signal Strength:</strong> Good signal strength for reliable connection</p>");
    } else if (network.rssi >= -80) {
        html += F("<p>⚠️ <strong>Signal Strength:</strong> Weak signal may cause connection issues</p>");
    } else {
        html += F("<p>❌ <strong>Signal Strength:</strong> Very weak signal, connection not recommended</p>");
    }
    
    // Security analysis
    if (network.encryptionType == WIFI_AUTH_OPEN) {
        html += F("<p>❌ <strong>Security:</strong> No encryption - avoid transmitting sensitive data</p>");
    } else if (network.encryptionType == WIFI_AUTH_WEP) {
        html += F("<p>⚠️ <strong>Security:</strong> Weak encryption - not recommended</p>");
    } else if (network.encryptionType >= WIFI_AUTH_WPA3_PSK) {
        html += F("<p>✅ <strong>Security:</strong> Excellent encryption with modern security standards</p>");
    } else {
        html += F("<p>✅ <strong>Security:</strong> Adequate encryption for most purposes</p>");
    }
    
    // Channel congestion analysis
    if (channelUsage <= 3) {
        html += F("<p>✅ <strong>Channel Congestion:</strong> Low interference, good performance expected</p>");
    } else if (channelUsage <= 6) {
        html += F("<p>⚠️ <strong>Channel Congestion:</strong> Moderate interference, may affect performance</p>");
    } else {
        html += F("<p>❌ <strong>Channel Congestion:</strong> Heavy interference, performance may be degraded</p>");
    }
    
    html += F("</div>");
    
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
                <a href="/signal">📶 Signal</a>
                <a href="/portscan">🔒 Port Scanner</a>
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

        <!-- Signal Strength Monitor Card -->
        <div style="background: linear-gradient(135deg, #06b6d4 0%, #0891b2 100%); color: white; padding: 25px; border-radius: 10px; box-shadow: 0 4px 15px rgba(0,0,0,0.2);">
            <h3 style="margin: 0 0 10px 0; font-size: 1.3em;">📶 Signal Monitor</h3>
            <p style="margin: 0 0 15px 0; opacity: 0.9;">Real-time WiFi signal strength monitoring and analysis</p>
            <div style="margin: 10px 0;">
                <span style="font-size: 0.9em; opacity: 0.8;">Current Signal:</span><br>
    )rawliteral";
    
    if (WiFi.status() == WL_CONNECTED) {
        int32_t rssi = WiFi.RSSI();
        uint8_t quality = rssiToQuality(rssi);
        html += "<strong>" + String(rssi) + " dBm (" + String(quality) + "%)</strong>";
    } else {
        html += "<strong>Not Connected</strong>";
    }
    
    html += R"rawliteral(
            </div>
            <button onclick="location.href='/signal'" style="margin-top: 15px; padding: 10px 20px; background: white; color: #06b6d4; border: none; border-radius: 5px; font-weight: bold; cursor: pointer; width: 100%;">
                View Signal Monitor
            </button>
        </div>

        <!-- Port Scanner Card -->
        <div style="background: linear-gradient(135deg, #8b5cf6 0%, #7c3aed 100%); color: white; padding: 25px; border-radius: 10px; box-shadow: 0 4px 15px rgba(0,0,0,0.2);">
            <h3 style="margin: 0 0 10px 0; font-size: 1.3em;">🔒 Port Scanner</h3>
            <p style="margin: 0 0 15px 0; opacity: 0.9;">Security audit and open port detection</p>
            <div style="margin: 10px 0;">
                <span style="font-size: 0.9em; opacity: 0.8;">Last Scan:</span><br>
    )rawliteral";
    
    PortScanResults portScanResults = getLastPortScanResults();
    if (portScanResults.scanCompleted) {
        html += "<strong>" + String(portScanResults.openPorts) + " open port(s) found</strong>";
    } else {
        html += "<strong>Never</strong>";
    }
    
    html += R"rawliteral(
            </div>
            <button onclick="location.href='/portscan'" style="margin-top: 15px; padding: 10px 20px; background: white; color: #8b5cf6; border: none; border-radius: 5px; font-weight: bold; cursor: pointer; width: 100%;">
                Start Port Scan
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
        <button onclick="location.href='/signal'" style="padding: 15px; background: linear-gradient(135deg, #06b6d4 0%, #0891b2 100%); color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer;">
            📶 Monitor Signal
        </button>
        <button onclick="location.href='/latency/start'" style="padding: 15px; background: linear-gradient(135deg, #10b981 0%, #059669 100%); color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer;">
            📉 Test Latency
        </button>
        <button onclick="location.href='/iperf'" style="padding: 15px; background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%); color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer;">
            ⚡ Run iPerf
        </button>
        <button onclick="location.href='/portscan'" style="padding: 15px; background: linear-gradient(135deg, #8b5cf6 0%, #7c3aed 100%); color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer;">
            🔒 Scan Ports
        </button>
    </div>

    <h2>💡 Network Analysis Tips</h2>
    <div style="background: #f0f9ff; padding: 20px; border-left: 4px solid #3b82f6; border-radius: 5px; margin: 20px 0;">
        <ul style="margin: 10px 0; padding-left: 25px;">
            <li><strong>Signal Monitoring:</strong> Track real-time signal strength and quality of nearby networks</li>
            <li><strong>Port Scanner:</strong> Audit network security by discovering open ports on devices</li>
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
    )rawliteral";
    
    html += generateNav();
    
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
        <button onclick="startScan('/channel/scan', '📡 Analyzing Channels...', 'Scanning 2.4GHz spectrum across all WiFi channels')" style="padding: 15px 40px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border: none; border-radius: 8px; font-size: 1.1em; cursor: pointer; font-weight: bold; box-shadow: 0 4px 12px rgba(102,126,234,0.4);">
            🔄 Scan Channels
        </button>
        <button onclick="location.href='/channel/graph'" style="padding: 15px 40px; background: linear-gradient(135deg, #10b981 0%, #059669 100%); color: white; border: none; border-radius: 8px; font-size: 1.1em; cursor: pointer; font-weight: bold; box-shadow: 0 4px 12px rgba(16,185,129,0.4); margin-left: 15px;">
            📊 View Channel Graph
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

// ==========================================
// CHANNEL GRAPH PAGE
// ==========================================
void handleChannelGraph() {
    String html = FPSTR(HTML_HEADER);
    
    html += R"rawliteral(
    <div class="header">
        <h1>📊 Channel Graph</h1>
        <p>Visual WiFi Channel Spectrum & Signal Strength</p>
    </div>
    )rawliteral";
    
    html += generateNav();
    
    // Scan button
    html += R"rawliteral(
    <div style="text-align:center;margin:20px 0">
        <button onclick="startScan('/channel/scan', '📡 Analyzing Channels...', 'Scanning 2.4GHz spectrum across all WiFi channels')" style="padding:15px 40px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;border:none;border-radius:8px;font-size:1.1em;font-weight:bold;cursor:pointer;box-shadow:0 4px 12px rgba(102,126,234,0.4)">
            🔄 Scan Channels
        </button>
    </div>
    )rawliteral";
    
    // Check if we have scan data
    if (lastChannelAnalysis.scan_timestamp > 0) {
        html += R"rawliteral(
        <h2>📈 Channel Spectrum Visualization</h2>
        <div style="background:#f8f9fa;padding:25px;border-radius:10px;margin:20px 0">
            <canvas id="channelGraph" width="1000" height="400" style="width:100%;height:400px;background:white;border-radius:8px"></canvas>
        </div>
        
        <h2>🎯 Channel Recommendations</h2>
        <div style="background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0">
        )rawliteral";
        
        html += "<p><strong>Best Channel:</strong> <span class=\"badge success\">Channel " + String(lastChannelAnalysis.best_channel_2g4) + "</span></p>";
        html += "<p><strong>Least Crowded:</strong> Channel " + String(lastChannelAnalysis.best_channel_2g4) + " with " + String((int)(100 - lastChannelAnalysis.channels[lastChannelAnalysis.best_channel_2g4 - 1].congestion_score)) + "% available capacity</p>";
        html += "<p><strong>Total Networks Found:</strong> " + String(lastChannelAnalysis.total_networks) + "</p>";
        html += "<p><strong>Recommended Non-Overlapping Channels:</strong> 1, 6, 11</p>";
        
        html += "</div>";
        
        // Network list by channel
        html += R"rawliteral(
        <h2>📡 Networks by Channel</h2>
        <div id="networksByChannel" style="margin:20px 0"></div>
        )rawliteral";
        
        // JavaScript to draw the graph
        html += "<script>";
        
        // Channel data
        html += "const channelData = [";
        for (int i = 0; i < 14; i++) {
            if (lastChannelAnalysis.channels[i].channel > 0) {
                if (i > 0) html += ",";
                html += "{ch:" + String(lastChannelAnalysis.channels[i].channel);
                html += ",nets:" + String(lastChannelAnalysis.channels[i].network_count);
                html += ",cong:" + String(lastChannelAnalysis.channels[i].congestion_score, 1);
                html += ",rec:" + String(lastChannelAnalysis.channels[i].is_recommended ? "true" : "false");
                html += "}";
            }
        }
        html += "];";
        
        // Draw function
        html += R"rawliteral(
        function drawChannelGraph() {
            const canvas = document.getElementById('channelGraph');
            const ctx = canvas.getContext('2d');
            const width = canvas.width;
            const height = canvas.height;
            
            // Clear canvas
            ctx.clearRect(0, 0, width, height);
            
            // Graph dimensions
            const padding = 60;
            const graphWidth = width - padding * 2;
            const graphHeight = height - padding * 2;
            const channels = 14;
            const channelWidth = graphWidth / channels;
            
            // Draw grid
            ctx.strokeStyle = '#e5e7eb';
            ctx.lineWidth = 1;
            for (let i = 0; i <= 10; i++) {
                const y = padding + (graphHeight / 10) * i;
                ctx.beginPath();
                ctx.moveTo(padding, y);
                ctx.lineTo(width - padding, y);
                ctx.stroke();
            }
            
            // Draw channel lines
            for (let i = 0; i <= channels; i++) {
                const x = padding + channelWidth * i;
                ctx.beginPath();
                ctx.moveTo(x, padding);
                ctx.lineTo(x, height - padding);
                ctx.stroke();
            }
            
            // Draw axes
            ctx.strokeStyle = '#333';
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo(padding, height - padding);
            ctx.lineTo(width - padding, height - padding);
            ctx.lineTo(width - padding, padding);
            ctx.stroke();
            
            // Draw Y-axis labels (signal strength)
            ctx.fillStyle = '#666';
            ctx.font = '12px Arial';
            ctx.textAlign = 'right';
            for (let i = 0; i <= 10; i++) {
                const y = padding + (graphHeight / 10) * i;
                const value = 100 - (i * 10);
                ctx.fillText(value + '%', padding - 10, y + 4);
            }
            
            // Draw X-axis labels (channels)
            ctx.textAlign = 'center';
            for (let i = 0; i < channels; i++) {
                const x = padding + channelWidth * i + channelWidth / 2;
                ctx.fillText('Ch ' + (i + 1), x, height - padding + 20);
            }
            
            // Axis titles
            ctx.save();
            ctx.translate(20, height / 2);
            ctx.rotate(-Math.PI / 2);
            ctx.font = 'bold 14px Arial';
            ctx.fillStyle = '#333';
            ctx.textAlign = 'center';
            ctx.fillText('Congestion / Signal Strength', 0, 0);
            ctx.restore();
            
            ctx.font = 'bold 14px Arial';
            ctx.fillText('WiFi Channel', width / 2, height - 10);
            
            // Draw channel congestion bars
            channelData.forEach((data, idx) => {
                if (data.ch > 0 && data.ch <= 14) {
                    const x = padding + channelWidth * (data.ch - 1);
                    const barHeight = (data.cong / 100) * graphHeight;
                    const y = height - padding - barHeight;
                    
                    // Color based on congestion
                    let color;
                    if (data.cong > 70) {
                        color = '#ef4444'; // Red
                    } else if (data.cong > 40) {
                        color = '#fbbf24'; // Yellow
                    } else {
                        color = '#10b981'; // Green
                    }
                    
                    // Draw bar
                    ctx.fillStyle = color;
                    ctx.fillRect(x + 5, y, channelWidth - 10, barHeight);
                    
                    // Draw border
                    ctx.strokeStyle = '#333';
                    ctx.lineWidth = 1;
                    ctx.strokeRect(x + 5, y, channelWidth - 10, barHeight);
                    
                    // Draw network count
                    if (data.nets > 0) {
                        ctx.fillStyle = '#fff';
                        ctx.font = 'bold 12px Arial';
                        ctx.textAlign = 'center';
                        const textY = y + barHeight / 2;
                        ctx.fillText(data.nets + ' net' + (data.nets > 1 ? 's' : ''), x + channelWidth / 2, textY + 4);
                    }
                    
                    // Mark recommended channels
                    if (data.rec) {
                        ctx.fillStyle = '#fbbf24';
                        ctx.font = 'bold 16px Arial';
                        ctx.fillText('⭐', x + channelWidth / 2, padding - 10);
                    }
                }
            });
            
            // Draw legend
            const legendX = width - padding - 150;
            const legendY = padding + 20;
            
            ctx.font = 'bold 12px Arial';
            ctx.fillStyle = '#333';
            ctx.textAlign = 'left';
            ctx.fillText('Congestion Level:', legendX, legendY);
            
            // Legend items
            const items = [
                {color: '#10b981', text: 'Low (0-40%)'},
                {color: '#fbbf24', text: 'Medium (40-70%)'},
                {color: '#ef4444', text: 'High (70-100%)'}
            ];
            
            items.forEach((item, idx) => {
                const y = legendY + 20 + idx * 20;
                ctx.fillStyle = item.color;
                ctx.fillRect(legendX, y - 10, 15, 15);
                ctx.fillStyle = '#333';
                ctx.fillText(item.text, legendX + 20, y + 2);
            });
        }
        
        // Draw on page load
        window.onload = drawChannelGraph;
        window.onresize = drawChannelGraph;
        )rawliteral";
        
        html += "</script>";
        
        // Scan info
        unsigned long ageSeconds = (millis() - lastChannelAnalysis.scan_timestamp) / 1000;
        html += "<p style=\"text-align:center;color:#666;margin-top:20px\">";
        html += "Last scan: " + String(ageSeconds) + " seconds ago | Duration: " + String(lastChannelAnalysis.scan_duration_ms) + " ms";
        html += "</p>";
        
    } else {
        html += R"rawliteral(
        <div style="background:#fff3cd;padding:20px;border-left:4px solid #ffc107;border-radius:5px;margin:20px 0">
            <p style="text-align:center;color:#666">
                <strong>No channel data available yet.</strong><br><br>
                Click "Scan Channels" above to analyze the 2.4GHz spectrum and generate the channel graph.
            </p>
        </div>
        )rawliteral";
    }
    
    html += R"rawliteral(
    <h2>💡 Understanding the Channel Graph</h2>
    <div style="background:#e3f2fd;padding:20px;border-left:4px solid #2196f3;border-radius:5px;margin:20px 0">
        <ul style="margin:10px 0;padding-left:25px;line-height:1.8">
            <li><strong>Bar Height:</strong> Represents channel congestion (higher = more crowded)</li>
            <li><strong>Colors:</strong> Green (good), Yellow (moderate), Red (congested)</li>
            <li><strong>Numbers:</strong> Show count of networks on each channel</li>
            <li><strong>Stars (⭐):</strong> Mark recommended channels with lowest interference</li>
            <li><strong>Non-Overlapping:</strong> Channels 1, 6, and 11 don't overlap in 2.4GHz</li>
            <li><strong>Best Practice:</strong> Choose a channel with lowest bar and fewest networks</li>
        </ul>
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

// ==========================================
// CONFIGURATION HANDLERS
// ==========================================

void handleConfig() {
    String html = HTML_HEADER;
    
    html += R"rawliteral(
    <div class="header">
        <h1>⚙️ Configuration</h1>
        <p>Manage Access Point and Station Mode Settings</p>
    </div>
    )rawliteral";
    
    html += FPSTR(NAV_MENU);
    
    html += R"rawliteral(
    <style>
        .config-section {
            background: white;
            padding: 20px;
            margin: 20px 0;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .config-header {
            display: flex;
            align-items: center;
            margin-bottom: 15px;
            padding-bottom: 10px;
            border-bottom: 2px solid #4CAF50;
        }
        .config-header h2 {
            margin: 0;
            color: #333;
        }
        .config-status {
            background: #f0f0f0;
            padding: 10px;
            margin: 10px 0;
            border-radius: 5px;
            font-family: monospace;
        }
        .status-saved {
            color: #4CAF50;
            font-weight: bold;
        }
        .status-none {
            color: #999;
        }
        .form-group {
            margin: 15px 0;
        }
        .form-group label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
            color: #555;
        }
        .form-group input,
        .form-group select {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 14px;
        }
        .form-group small {
            color: #666;
            font-size: 12px;
        }
        .btn-group {
            display: flex;
            gap: 10px;
            margin-top: 15px;
        }
        .btn {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
            font-weight: bold;
            transition: all 0.3s;
        }
        .btn-save {
            background: #4CAF50;
            color: white;
        }
        .btn-save:hover {
            background: #45a049;
        }
        .btn-clear {
            background: #f44336;
            color: white;
        }
        .btn-clear:hover {
            background: #da190b;
        }
        .info-box {
            background: #e7f3ff;
            border-left: 4px solid #2196F3;
            padding: 10px;
            margin: 15px 0;
            border-radius: 4px;
        }
        .warning-box {
            background: #fff3cd;
            border-left: 4px solid #ffc107;
            padding: 10px;
            margin: 15px 0;
            border-radius: 4px;
        }
        .checkbox-group {
            display: flex;
            align-items: center;
            margin: 10px 0;
        }
        .checkbox-group input {
            width: auto;
            margin-right: 10px;
        }
        .mode-toggle-section {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            text-align: center;
        }
        .mode-toggle-section h3 {
            margin: 0 0 15px 0;
            color: white;
        }
        .toggle-container {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 15px;
            margin: 15px 0;
        }
        .mode-label {
            font-size: 1.1em;
            font-weight: bold;
        }
        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 80px;
            height: 40px;
        }
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .toggle-slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #4CAF50;
            transition: .4s;
            border-radius: 40px;
        }
        .toggle-slider:before {
            position: absolute;
            content: "";
            height: 32px;
            width: 32px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        input:checked + .toggle-slider {
            background-color: #2196F3;
        }
        input:checked + .toggle-slider:before {
            transform: translateX(40px);
        }
        .mode-buttons {
            display: flex;
            gap: 10px;
            justify-content: center;
            margin-top: 15px;
        }
        .mode-btn {
            padding: 10px 20px;
            border: 2px solid white;
            background: rgba(255,255,255,0.2);
            color: white;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
            font-weight: bold;
            transition: all 0.3s;
        }
        .mode-btn:hover {
            background: rgba(255,255,255,0.3);
        }
        .mode-btn.active {
            background: white;
            color: #667eea;
        }
    </style>
    
    <h1>⚙️ Configuration</h1>
    )rawliteral";
    
    // Check for saved configurations
    APConfig apConfig;
    bool hasAP = loadAPConfig(apConfig);
    
    StationConfig staConfig;
    bool hasStation = loadStationConfig(staConfig);
    
    // Get current WiFi mode
    wifi_mode_t currentMode = WiFi.getMode();
    String currentModeStr = "IDLE";
    if (currentMode == WIFI_MODE_AP || currentMode == WIFI_MODE_APSTA) {
        currentModeStr = "AP";
    } else if (currentMode == WIFI_MODE_STA) {
        currentModeStr = "Station";
    }
    
    // Mode Toggle Section
    html += R"rawliteral(
    <div class="mode-toggle-section">
        <h3>🔄 Quick Mode Switch</h3>
        <p style="margin: 5px 0; font-size: 0.9em;">Current Mode: <strong id="currentMode">)rawliteral";
    html += currentModeStr;
    html += R"rawliteral(</strong></p>
        
        <div class="mode-buttons">
            <button class="mode-btn" id="btnSwitchAP" onclick="switchMode('ap')">📡 Switch to Access Point</button>
            <button class="mode-btn" id="btnSwitchStation" onclick="switchMode('station')">📶 Switch to Station</button>
        </div>
        
        <div id="switchStatus" style="margin-top: 10px; font-size: 0.9em;"></div>
    </div>
    
    <script>
        function switchMode(mode) {
            const statusDiv = document.getElementById('switchStatus');
            const apBtn = document.getElementById('btnSwitchAP');
            const staBtn = document.getElementById('btnSwitchStation');
            
            // Disable buttons
            apBtn.disabled = true;
            staBtn.disabled = true;
            
            statusDiv.innerHTML = '⏳ Switching to ' + (mode === 'ap' ? 'Access Point' : 'Station') + ' mode...';
            
            fetch('/mode/switch', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'mode=' + mode
            })
            .then(response => response.text())
            .then(data => {
                statusDiv.innerHTML = '✓ ' + data;
                document.getElementById('currentMode').textContent = mode === 'ap' ? 'AP' : 'Station';
                
                // Re-enable buttons after 2 seconds
                setTimeout(() => {
                    apBtn.disabled = false;
                    staBtn.disabled = false;
                    statusDiv.innerHTML = '';
                }, 2000);
            })
            .catch(error => {
                statusDiv.innerHTML = '✗ Error: ' + error;
                apBtn.disabled = false;
                staBtn.disabled = false;
            });
        }
    </script>
    )rawliteral";
    
    // AP Configuration Section
    html += R"rawliteral(
    <div class="config-section">
        <div class="config-header">
            <h2>📡 Access Point Configuration</h2>
        </div>
        )rawliteral";
    
    if (hasAP) {
        html += "<div class='config-status'>";
        html += "<span class='status-saved'>✓ Saved Configuration</span><br>";
        html += "SSID: <strong>" + String(apConfig.ssid) + "</strong><br>";
        html += "Channel: <strong>" + String(apConfig.channel) + "</strong><br>";
        html += "Auto-start: <strong>" + String(apConfig.autoStart ? "Yes" : "No") + "</strong>";
        html += "</div>";
    } else {
        html += "<div class='config-status'><span class='status-none'>No saved configuration</span></div>";
    }
    
    html += R"rawliteral(
        <form action="/config/ap" method="POST">
            <div class="form-group">
                <label for="ap_ssid">SSID *</label>
                <input type="text" id="ap_ssid" name="ssid" placeholder="Enter AP SSID" 
                       maxlength="32" required )rawliteral";
    if (hasAP) {
        html += " value=\"" + String(apConfig.ssid) + "\"";
    }
    html += R"rawliteral(>
                <small>1-32 characters</small>
            </div>
            
            <div class="form-group">
                <label for="ap_password">Password *</label>
                <input type="password" id="ap_password" name="password" 
                       placeholder="Enter new password (8-63 chars)" minlength="8" maxlength="63">
                <small>Leave empty to keep current password. 8-63 characters for new password.</small>
            </div>
            
            <div class="form-group">
                <label for="ap_channel">Channel</label>
                <select id="ap_channel" name="channel">
    )rawliteral";
    
    for (int ch = 1; ch <= 13; ch++) {
        html += "<option value=\"" + String(ch) + "\"";
        if (hasAP && apConfig.channel == ch) {
            html += " selected";
        } else if (!hasAP && ch == 1) {
            html += " selected";
        }
        html += ">Channel " + String(ch) + "</option>";
    }
    
    html += R"rawliteral(
                </select>
            </div>
            
            <div class="checkbox-group">
                <input type="checkbox" id="ap_autostart" name="autostart" value="1" )rawliteral";
    if (!hasAP || apConfig.autoStart) {
        html += " checked";
    }
    html += R"rawliteral(>
                <label for="ap_autostart">Auto-start AP on boot</label>
            </div>
            
            <div class="info-box">
                <strong>ℹ️ Note:</strong> Auto-start has highest priority. If enabled, device will boot as AP.
            </div>
            
            <div class="btn-group">
                <button type="submit" class="btn btn-save">💾 Save AP Config</button>
    )rawliteral";
    
    if (hasAP) {
        html += R"rawliteral(
                <button type="button" class="btn btn-clear" onclick="clearAPConfig()">🗑️ Clear Config</button>
        )rawliteral";
    }
    
    html += R"rawliteral(
            </div>
        </form>
    </div>
    )rawliteral";
    
    // Station Configuration Section
    html += R"rawliteral(
    <div class="config-section">
        <div class="config-header">
            <h2>📶 Station Configuration</h2>
        </div>
        )rawliteral";
    
    if (hasStation) {
        html += "<div class='config-status'>";
        html += "<span class='status-saved'>✓ Saved Configuration</span><br>";
        html += "SSID: <strong>" + String(staConfig.ssid) + "</strong><br>";
        html += "Auto-connect: <strong>" + String(staConfig.autoConnect ? "Yes" : "No") + "</strong>";
        html += "</div>";
    } else {
        html += "<div class='config-status'><span class='status-none'>No saved configuration</span></div>";
    }
    
    html += R"rawliteral(
        <form action="/config/station" method="POST">
            <div class="form-group">
                <label for="sta_ssid">WiFi Network SSID *</label>
                <input type="text" id="sta_ssid" name="ssid" placeholder="Enter WiFi SSID" 
                       maxlength="32" required )rawliteral";
    if (hasStation) {
        html += " value=\"" + String(staConfig.ssid) + "\"";
    }
    html += R"rawliteral(>
                <small>1-32 characters</small>
            </div>
            
            <div class="form-group">
                <label for="sta_password">WiFi Password *</label>
                <input type="password" id="sta_password" name="password" 
                       placeholder="Enter new password (0-63 chars)" maxlength="63">
                <small>Leave empty to keep current password. 0-63 characters for new password (empty for open network).</small>
            </div>
            
            <div class="checkbox-group">
                <input type="checkbox" id="sta_autoconnect" name="autoconnect" value="1" )rawliteral";
    if (!hasStation || staConfig.autoConnect) {
        html += " checked";
    }
    html += R"rawliteral(>
                <label for="sta_autoconnect">Auto-connect on boot</label>
            </div>
            
            <div class="warning-box">
                <strong>⚠️ Priority:</strong> AP auto-start takes priority over Station auto-connect.
                If both are enabled, device will boot as AP.
            </div>
            
            <div class="btn-group">
                <button type="submit" class="btn btn-save">💾 Save Station Config</button>
    )rawliteral";
    
    if (hasStation) {
        html += R"rawliteral(
                <button type="button" class="btn btn-clear" onclick="clearStationConfig()">🗑️ Clear Config</button>
        )rawliteral";
    }
    
    html += R"rawliteral(
            </div>
        </form>
    </div>
    
    <script>
        function clearAPConfig() {
            if (confirm('Are you sure you want to clear the AP configuration?')) {
                fetch('/config/clear', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'type=ap'
                })
                .then(response => response.text())
                .then(result => {
                    alert('AP configuration cleared');
                    location.reload();
                })
                .catch(error => {
                    alert('Error clearing AP configuration');
                });
            }
        }
        
        function clearStationConfig() {
            if (confirm('Are you sure you want to clear the Station configuration?')) {
                fetch('/config/clear', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'type=station'
                })
                .then(response => response.text())
                .then(result => {
                    alert('Station configuration cleared');
                    location.reload();
                })
                .catch(error => {
                    alert('Error clearing Station configuration');
                });
            }
        }
    </script>
    )rawliteral";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

void handleConfigAP() {
    if (!webServer->hasArg("ssid")) {
        webServer->send(400, "text/plain", "Missing SSID parameter");
        return;
    }
    
    String ssid = webServer->arg("ssid");
    String password = webServer->arg("password");
    String channelStr = webServer->arg("channel");
    bool autoStart = webServer->hasArg("autostart");
    
    // Validate SSID
    if (ssid.length() == 0 || ssid.length() > 32) {
        webServer->send(400, "text/plain", "SSID must be 1-32 characters");
        return;
    }
    
    // Check if we're updating existing config
    APConfig config;
    bool hasExisting = loadAPConfig(config);
    
    // If password is empty and we have existing config, keep old password
    if (password.length() == 0 && hasExisting) {
        // Keep existing password
        Serial.println("[Web Config] Keeping existing AP password");
    } else {
        // Validate new password
        if (password.length() < 8 || password.length() > 63) {
            webServer->send(400, "text/plain", "Password must be 8-63 characters");
            return;
        }
        // Use new password
        strncpy(config.password, password.c_str(), sizeof(config.password) - 1);
        config.password[sizeof(config.password) - 1] = '\0';
    }
    
    // Set SSID
    strncpy(config.ssid, ssid.c_str(), sizeof(config.ssid) - 1);
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    
    // Set channel
    int channel = channelStr.toInt();
    if (channel < 1 || channel > 13) {
        channel = 1;
    }
    config.channel = channel;
    
    // Set auto-start
    config.autoStart = autoStart;
    config.isValid = true;
    
    // Save configuration
    if (saveAPConfig(config)) {
        String html = HTML_HEADER;
        html += R"rawliteral(
        <div class="header">
            <h1 style="color: #4CAF50;">✓ AP Configuration Saved</h1>
        </div>
        )rawliteral";
        html += FPSTR(NAV_MENU);
        html += R"rawliteral(
        <style>
            .modal {
                display: none;
                position: fixed;
                z-index: 10000;
                left: 0;
                top: 0;
                width: 100%;
                height: 100%;
                background-color: rgba(0,0,0,0.6);
                animation: fadeIn 0.3s;
            }
            @keyframes fadeIn {
                from { opacity: 0; }
                to { opacity: 1; }
            }
            .modal-content {
                background-color: white;
                margin: 10% auto;
                padding: 30px;
                border-radius: 15px;
                width: 90%;
                max-width: 500px;
                box-shadow: 0 10px 40px rgba(0,0,0,0.3);
                animation: slideIn 0.3s;
            }
            @keyframes slideIn {
                from { transform: translateY(-50px); opacity: 0; }
                to { transform: translateY(0); opacity: 1; }
            }
            .modal-header {
                font-size: 1.5em;
                color: #667eea;
                margin-bottom: 20px;
                text-align: center;
            }
            .modal-body {
                color: #666;
                line-height: 1.6;
                margin-bottom: 25px;
                text-align: center;
            }
            .modal-buttons {
                display: flex;
                gap: 10px;
                justify-content: center;
            }
            .modal-btn {
                padding: 12px 30px;
                border: none;
                border-radius: 5px;
                cursor: pointer;
                font-size: 14px;
                font-weight: bold;
                transition: all 0.3s;
            }
            .modal-btn-primary {
                background: #4CAF50;
                color: white;
            }
            .modal-btn-primary:hover {
                background: #45a049;
            }
            .modal-btn-secondary {
                background: #ddd;
                color: #333;
            }
            .modal-btn-secondary:hover {
                background: #ccc;
            }
            .countdown {
                font-size: 1.2em;
                font-weight: bold;
                color: #667eea;
                margin: 10px 0;
            }
        </style>
        
        <div style="text-align: center; padding: 40px;">
            <p style="font-size: 16px; color: #666;">
                SSID: <strong>)rawliteral" + ssid + R"rawliteral(</strong><br>
                Channel: <strong>)rawliteral" + String(channel) + R"rawliteral(</strong><br>
                Auto-start: <strong>)rawliteral" + String(autoStart ? "Yes" : "No") + R"rawliteral(</strong>
            </p>
            <p style="margin-top: 30px;">
                <button onclick="showRebootModal()" style="padding: 12px 30px; background: #667eea; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 14px; font-weight: bold; margin-right: 10px;">🔄 Reboot Device</button>
                <a href="/config" style="padding: 12px 30px; background: #4CAF50; color: white; text-decoration: none; border-radius: 5px; display: inline-block;">← Back to Configuration</a>
            </p>
        </div>
        
        <!-- Reboot Modal -->
        <div id="rebootModal" class="modal">
            <div class="modal-content">
                <div class="modal-header">🔄 Reboot Device</div>
                <div class="modal-body">
                    <p>Configuration has been saved successfully.</p>
                    <p>Do you want to reboot the device now to apply the changes?</p>
                    <div class="countdown" id="countdown" style="display: none;">Rebooting in <span id="countdownValue">3</span> seconds...</div>
                </div>
                <div class="modal-buttons">
                    <button class="modal-btn modal-btn-primary" onclick="rebootDevice()">Yes, Reboot Now</button>
                    <button class="modal-btn modal-btn-secondary" onclick="closeModal()">No, Later</button>
                </div>
            </div>
        </div>
        
        <script>
            function showRebootModal() {
                document.getElementById('rebootModal').style.display = 'block';
            }
            
            function closeModal() {
                document.getElementById('rebootModal').style.display = 'none';
            }
            
            function rebootDevice() {
                // Disable buttons
                const buttons = document.querySelectorAll('.modal-btn');
                buttons.forEach(btn => btn.disabled = true);
                
                // Show countdown
                document.getElementById('countdown').style.display = 'block';
                document.querySelector('.modal-body p:last-of-type').style.display = 'none';
                
                let count = 3;
                const countdownEl = document.getElementById('countdownValue');
                
                const countdown = setInterval(() => {
                    count--;
                    countdownEl.textContent = count;
                    if (count <= 0) {
                        clearInterval(countdown);
                        countdownEl.textContent = 'Rebooting...';
                    }
                }, 1000);
                
                // Send reboot request
                fetch('/reboot', { method: 'POST' })
                    .then(() => {
                        setTimeout(() => {
                            document.querySelector('.modal-body').innerHTML = 
                                '<p>Device is rebooting...</p>' +
                                '<p>Please wait 10 seconds and reconnect.</p>' +
                                '<p style="font-size: 0.9em; color: #999; margin-top: 20px;">This page will close automatically.</p>';
                            setTimeout(() => {
                                window.close();
                                window.location.href = '/';
                            }, 10000);
                        }, 3000);
                    })
                    .catch(error => {
                        document.querySelector('.modal-body').innerHTML = 
                            '<p style="color: #f44336;">Error initiating reboot.</p>' +
                            '<p>Please manually reboot the device.</p>';
                        buttons.forEach(btn => btn.disabled = false);
                    });
            }
            
            // Close modal when clicking outside
            window.onclick = function(event) {
                const modal = document.getElementById('rebootModal');
                if (event.target == modal) {
                    closeModal();
                }
            }
        </script>
        )rawliteral";
        html += generateHtmlFooter();
        webServer->send(200, "text/html", html);
    } else {
        webServer->send(500, "text/plain", "Failed to save configuration");
    }
}

void handleConfigStation() {
    if (!webServer->hasArg("ssid")) {
        webServer->send(400, "text/plain", "Missing SSID parameter");
        return;
    }
    
    String ssid = webServer->arg("ssid");
    String password = webServer->arg("password");
    bool autoConnect = webServer->hasArg("autoconnect");
    
    // Validate SSID
    if (ssid.length() == 0 || ssid.length() > 32) {
        webServer->send(400, "text/plain", "SSID must be 1-32 characters");
        return;
    }
    
    // Check if we're updating existing config
    StationConfig config;
    bool hasExisting = loadStationConfig(config);
    
    // If password is empty and we have existing config, keep old password
    if (password.length() == 0 && hasExisting) {
        // Keep existing password
        Serial.println("[Web Config] Keeping existing Station password");
    } else {
        // Validate new password (can be empty for open networks)
        if (password.length() > 63) {
            webServer->send(400, "text/plain", "Password must be 0-63 characters");
            return;
        }
        // Use new password
        strncpy(config.password, password.c_str(), sizeof(config.password) - 1);
        config.password[sizeof(config.password) - 1] = '\0';
    }
    
    // Set SSID
    strncpy(config.ssid, ssid.c_str(), sizeof(config.ssid) - 1);
    config.ssid[sizeof(config.ssid) - 1] = '\0';
    
    // Set auto-connect
    config.autoConnect = autoConnect;
    config.isValid = true;
    
    // Save configuration
    if (saveStationConfig(config)) {
        String html = HTML_HEADER;
        html += R"rawliteral(
        <div class="header">
            <h1 style="color: #4CAF50;">✓ Station Configuration Saved</h1>
        </div>
        )rawliteral";
        html += FPSTR(NAV_MENU);
        html += R"rawliteral(
        <style>
            .modal {
                display: none;
                position: fixed;
                z-index: 10000;
                left: 0;
                top: 0;
                width: 100%;
                height: 100%;
                background-color: rgba(0,0,0,0.6);
                animation: fadeIn 0.3s;
            }
            @keyframes fadeIn {
                from { opacity: 0; }
                to { opacity: 1; }
            }
            .modal-content {
                background-color: white;
                margin: 10% auto;
                padding: 30px;
                border-radius: 15px;
                width: 90%;
                max-width: 500px;
                box-shadow: 0 10px 40px rgba(0,0,0,0.3);
                animation: slideIn 0.3s;
            }
            @keyframes slideIn {
                from { transform: translateY(-50px); opacity: 0; }
                to { transform: translateY(0); opacity: 1; }
            }
            .modal-header {
                font-size: 1.5em;
                color: #667eea;
                margin-bottom: 20px;
                text-align: center;
            }
            .modal-body {
                color: #666;
                line-height: 1.6;
                margin-bottom: 25px;
                text-align: center;
            }
            .modal-buttons {
                display: flex;
                gap: 10px;
                justify-content: center;
            }
            .modal-btn {
                padding: 12px 30px;
                border: none;
                border-radius: 5px;
                cursor: pointer;
                font-size: 14px;
                font-weight: bold;
                transition: all 0.3s;
            }
            .modal-btn-primary {
                background: #4CAF50;
                color: white;
            }
            .modal-btn-primary:hover {
                background: #45a049;
            }
            .modal-btn-secondary {
                background: #ddd;
                color: #333;
            }
            .modal-btn-secondary:hover {
                background: #ccc;
            }
            .countdown {
                font-size: 1.2em;
                font-weight: bold;
                color: #667eea;
                margin: 10px 0;
            }
        </style>
        
        <div style="text-align: center; padding: 40px;">
            <p style="font-size: 16px; color: #666;">
                SSID: <strong>)rawliteral" + ssid + R"rawliteral(</strong><br>
                Auto-connect: <strong>)rawliteral" + String(autoConnect ? "Yes" : "No") + R"rawliteral(</strong>
            </p>
            <p style="margin-top: 30px;">
                <button onclick="showRebootModal()" style="padding: 12px 30px; background: #667eea; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 14px; font-weight: bold; margin-right: 10px;">🔄 Reboot Device</button>
                <a href="/config" style="padding: 12px 30px; background: #4CAF50; color: white; text-decoration: none; border-radius: 5px; display: inline-block;">← Back to Configuration</a>
            </p>
        </div>
        
        <!-- Reboot Modal -->
        <div id="rebootModal" class="modal">
            <div class="modal-content">
                <div class="modal-header">🔄 Reboot Device</div>
                <div class="modal-body">
                    <p>Configuration has been saved successfully.</p>
                    <p>Do you want to reboot the device now to apply the changes?</p>
                    <div class="countdown" id="countdown" style="display: none;">Rebooting in <span id="countdownValue">3</span> seconds...</div>
                </div>
                <div class="modal-buttons">
                    <button class="modal-btn modal-btn-primary" onclick="rebootDevice()">Yes, Reboot Now</button>
                    <button class="modal-btn modal-btn-secondary" onclick="closeModal()">No, Later</button>
                </div>
            </div>
        </div>
        
        <script>
            function showRebootModal() {
                document.getElementById('rebootModal').style.display = 'block';
            }
            
            function closeModal() {
                document.getElementById('rebootModal').style.display = 'none';
            }
            
            function rebootDevice() {
                // Disable buttons
                const buttons = document.querySelectorAll('.modal-btn');
                buttons.forEach(btn => btn.disabled = true);
                
                // Show countdown
                document.getElementById('countdown').style.display = 'block';
                document.querySelector('.modal-body p:last-of-type').style.display = 'none';
                
                let count = 3;
                const countdownEl = document.getElementById('countdownValue');
                
                const countdown = setInterval(() => {
                    count--;
                    countdownEl.textContent = count;
                    if (count <= 0) {
                        clearInterval(countdown);
                        countdownEl.textContent = 'Rebooting...';
                    }
                }, 1000);
                
                // Send reboot request
                fetch('/reboot', { method: 'POST' })
                    .then(() => {
                        setTimeout(() => {
                            document.querySelector('.modal-body').innerHTML = 
                                '<p>Device is rebooting...</p>' +
                                '<p>Please wait 10 seconds and reconnect.</p>' +
                                '<p style="font-size: 0.9em; color: #999; margin-top: 20px;">This page will close automatically.</p>';
                            setTimeout(() => {
                                window.close();
                                window.location.href = '/';
                            }, 10000);
                        }, 3000);
                    })
                    .catch(error => {
                        document.querySelector('.modal-body').innerHTML = 
                            '<p style="color: #f44336;">Error initiating reboot.</p>' +
                            '<p>Please manually reboot the device.</p>';
                        buttons.forEach(btn => btn.disabled = false);
                    });
            }
            
            // Close modal when clicking outside
            window.onclick = function(event) {
                const modal = document.getElementById('rebootModal');
                if (event.target == modal) {
                    closeModal();
                }
            }
        </script>
        )rawliteral";
        html += generateHtmlFooter();
        webServer->send(200, "text/html", html);
    } else {
        webServer->send(500, "text/plain", "Failed to save configuration");
    }
}

void handleConfigClear() {
    if (!webServer->hasArg("type")) {
        webServer->send(400, "text/plain", "Missing type parameter");
        return;
    }
    
    String type = webServer->arg("type");
    
    if (type == "ap") {
        if (clearAPConfig()) {
            webServer->send(200, "text/plain", "AP configuration cleared");
        } else {
            webServer->send(500, "text/plain", "Failed to clear AP configuration");
        }
    } else if (type == "station") {
        if (clearStationConfig()) {
            webServer->send(200, "text/plain", "Station configuration cleared");
        } else {
            webServer->send(500, "text/plain", "Failed to clear Station configuration");
        }
    } else {
        webServer->send(400, "text/plain", "Invalid type parameter");
    }
}

void handleReboot() {
    Serial.println("[Web] Reboot requested via web interface");
    
    webServer->send(200, "text/plain", "Rebooting device...");
    
    // Give time for response to be sent
    delay(100);
    
    Serial.println("[Web] Rebooting device in 1 second...");
    delay(1000);
    
    ESP.restart();
}

void handleModeSwitch() {
    if (!webServer->hasArg("mode")) {
        webServer->send(400, "text/plain", "Missing mode parameter");
        return;
    }
    
    String mode = webServer->arg("mode");
    
    if (mode == "ap") {
        // Switch to Access Point mode
        Serial.println("[Web] Switching to Access Point mode");
        
        // Check if we have saved AP config
        APConfig config;
        if (loadAPConfig(config)) {
            // Use saved config
            startAccessPoint(String(config.ssid), String(config.password));
            webServer->send(200, "text/plain", "Switched to Access Point mode with saved config");
            Serial.println("[Web] Switched to AP mode with saved config");
        } else {
            // No saved config, use defaults
            startAccessPoint();
            webServer->send(200, "text/plain", "Switched to Access Point mode (default config)");
            Serial.println("[Web] Switched to AP mode with default config");
        }
    } else if (mode == "station") {
        // Switch to Station mode
        Serial.println("[Web] Switching to Station mode");
        
        // Check if we have saved Station config
        StationConfig config;
        if (loadStationConfig(config)) {
            // Use saved config
            connectToNetwork(String(config.ssid), String(config.password));
            webServer->send(200, "text/plain", "Connecting to Station mode... Check status in a moment");
            Serial.println("[Web] Initiated Station mode connection");
        } else {
            webServer->send(400, "text/plain", "No saved Station configuration. Please save credentials first.");
            Serial.println("[Web] No Station config available");
        }
    } else {
        webServer->send(400, "text/plain", "Invalid mode parameter. Use 'ap' or 'station'");
    }
}

// ==========================================
// SIGNAL STRENGTH MONITOR PAGE
// ==========================================
void handleSignalMonitor() {
    String html = FPSTR(HTML_HEADER);
    
    html += R"rawliteral(
    <div class="header">
        <h1>📶 Signal Strength Monitor</h1>
        <p>Real-Time WiFi Signal Analysis</p>
    </div>
    )rawliteral";
    
    html += generateNav();
    
    // Current Connection Signal
    html += R"rawliteral(
    <h2>📡 Current Connection</h2>
    <div style="background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0">
        <div id='currentSignal'>
            <p style="color:#666">Loading signal information...</p>
        </div>
    </div>
    )rawliteral";
    
    // Nearby Networks Signal Strength
    html += R"rawliteral(
    <h2>📊 Nearby Networks</h2>
    <div style="background:#f8f9fa;padding:20px;border-radius:10px;margin:20px 0">
        <div style="display:flex;align-items:center;gap:15px;flex-wrap:wrap;margin-bottom:15px">
            <button id="scanBtn" onclick='scanNetworks()' style="padding:12px 30px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;border:none;border-radius:8px;font-size:1em;font-weight:bold;cursor:pointer;box-shadow:0 4px 12px rgba(102,126,234,0.4);transition:all 0.3s">🔍 Scan Now</button>
            
            <button id="autoScanBtn" onclick='toggleAutoScan()' style="padding:12px 30px;background:#10b981;color:white;border:none;border-radius:8px;font-size:1em;font-weight:bold;cursor:pointer;transition:all 0.3s">▶️ Auto Scan</button>
            
            <div style="display:flex;align-items:center;gap:10px">
                <label for="scanInterval" style="font-weight:500;color:#333">Interval:</label>
                <select id="scanInterval" onchange="updateScanInterval()" style="padding:8px 12px;border:2px solid #667eea;border-radius:5px;font-size:1em;cursor:pointer">
                    <option value="5">5 seconds</option>
                    <option value="10" selected>10 seconds</option>
                    <option value="15">15 seconds</option>
                    <option value="30">30 seconds</option>
                    <option value="60">60 seconds</option>
                </select>
            </div>
            
            <div id="scanStatus" style="margin-left:auto;font-weight:500;color:#667eea"></div>
        </div>
    </div>
    <div id='nearbySignals'>
        <p style='text-align:center;padding:40px;color:#999'>Click "Scan Now" or enable "Auto Scan" to view nearby networks and their signal strengths.</p>
    </div>
    )rawliteral";
    
    // JavaScript for real-time updates
    html += "<script>";
    html += "let autoRefresh = true;";
    html += "let refreshInterval;";
    html += "let autoScanEnabled = false;";
    html += "let autoScanInterval;";
    html += "let scanIntervalSeconds = 10;";
    html += "let lastScanTime = 0;";
    html += "let scanInProgress = false;";
    
    // Function to get signal quality color
    html += "function getSignalColor(rssi) {";
    html += "  if (rssi >= -50) return '#10b981';";
    html += "  if (rssi >= -60) return '#10b981';";
    html += "  if (rssi >= -67) return '#fbbf24';";
    html += "  if (rssi >= -75) return '#fb923c';";
    html += "  return '#ef4444';";
    html += "}";
    
    // Update scan interval
    html += "function updateScanInterval() {";
    html += "  scanIntervalSeconds = parseInt(document.getElementById('scanInterval').value);";
    html += "  if (autoScanEnabled) {";
    html += "    stopAutoScan();";
    html += "    startAutoScan();";
    html += "  }";
    html += "}";
    
    // Toggle auto scan
    html += "function toggleAutoScan() {";
    html += "  if (autoScanEnabled) {";
    html += "    stopAutoScan();";
    html += "  } else {";
    html += "    startAutoScan();";
    html += "  }";
    html += "}";
    
    // Start auto scan
    html += "function startAutoScan() {";
    html += "  autoScanEnabled = true;";
    html += "  const btn = document.getElementById('autoScanBtn');";
    html += "  btn.innerHTML = '⏸️ Stop Auto';";
    html += "  btn.style.background = '#ef4444';";
    html += "  scanNetworks();";
    html += "  autoScanInterval = setInterval(scanNetworks, scanIntervalSeconds * 1000);";
    html += "  updateScanStatus();";
    html += "}";
    
    // Stop auto scan
    html += "function stopAutoScan() {";
    html += "  autoScanEnabled = false;";
    html += "  clearInterval(autoScanInterval);";
    html += "  const btn = document.getElementById('autoScanBtn');";
    html += "  btn.innerHTML = '▶️ Auto Scan';";
    html += "  btn.style.background = '#10b981';";
    html += "  document.getElementById('scanStatus').innerHTML = '';";
    html += "}";
    
    // Update scan status
    html += "function updateScanStatus() {";
    html += "  if (!autoScanEnabled) return;";
    html += "  const elapsed = Math.floor((Date.now() - lastScanTime) / 1000);";
    html += "  const remaining = Math.max(0, scanIntervalSeconds - elapsed);";
    html += "  if (scanInProgress) {";
    html += "    document.getElementById('scanStatus').innerHTML = '🔄 Scanning...';";
    html += "  } else if (remaining > 0) {";
    html += "    document.getElementById('scanStatus').innerHTML = '⏱️ Next scan in ' + remaining + 's';";
    html += "  }";
    html += "  setTimeout(updateScanStatus, 1000);";
    html += "}";
    
    // Function to create signal bars (similar to scan page circles)
    html += "function createSignalBars(rssi) {";
    html += "  let bars = 0;";
    html += "  if (rssi >= -50) bars = 5;";
    html += "  else if (rssi >= -60) bars = 4;";
    html += "  else if (rssi >= -67) bars = 3;";
    html += "  else if (rssi >= -75) bars = 2;";
    html += "  else bars = 1;";
    html += "  let color = getSignalColor(rssi);";
    html += "  let html = '<div style=\"display:flex;gap:3px;align-items:center\">';";
    html += "  for (let i = 0; i < 5; i++) {";
    html += "    if (i < bars) {";
    html += "      html += '<svg width=\"14\" height=\"14\" style=\"display:block\"><circle cx=\"7\" cy=\"7\" r=\"6\" fill=\"' + color + '\"/></svg>';";
    html += "    } else {";
    html += "      html += '<svg width=\"14\" height=\"14\" style=\"display:block\"><circle cx=\"7\" cy=\"7\" r=\"6\" fill=\"none\" stroke=\"#d1d5db\" stroke-width=\"2\"/></svg>';";
    html += "    }";
    html += "  }";
    html += "  html += '<span style=\"margin-left:8px;color:#666;font-size:0.9em\">' + rssi + ' dBm</span>';";
    html += "  html += '</div>';";
    html += "  return html;";
    html += "}";
    
    // Update current signal
    html += "function updateCurrentSignal() {";
    html += "  fetch('/signal/api?current=1')";
    html += "    .then(response => response.json())";
    html += "    .then(data => {";
    html += "      let html = '';";
    html += "      if (data.connected) {";
    html += "        html += '<div style=\"display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:20px\">';";
    html += "        html += '<div style=\"background:#fff;padding:20px;border-radius:10px;border-left:4px solid ' + getSignalColor(data.rssi) + ';\">';";
    html += "        html += '<div style=\"color:#666;font-size:0.9em;margin-bottom:5px\">Network Name</div>';";
    html += "        html += '<div style=\"font-size:1.3em;font-weight:bold;color:#333\">' + data.ssid + '</div>';";
    html += "        html += '</div>';";
    html += "        html += '<div style=\"background:#fff;padding:20px;border-radius:10px;border-left:4px solid ' + getSignalColor(data.rssi) + ';\">';";
    html += "        html += '<div style=\"color:#666;font-size:0.9em;margin-bottom:5px\">Signal Strength</div>';";
    html += "        html += '<div style=\"font-size:1.3em;font-weight:bold;color:' + getSignalColor(data.rssi) + ';\">' + data.rssi + ' dBm</div>';";
    html += "        html += '</div>';";
    html += "        html += '<div style=\"background:#fff;padding:20px;border-radius:10px;border-left:4px solid ' + getSignalColor(data.rssi) + ';\">';";
    html += "        html += '<div style=\"color:#666;font-size:0.9em;margin-bottom:5px\">Quality</div>';";
    html += "        html += '<div style=\"font-size:1.3em;font-weight:bold;color:#333\">' + data.quality + '%</div>';";
    html += "        html += '</div>';";
    html += "        html += '<div style=\"background:#fff;padding:20px;border-radius:10px;border-left:4px solid ' + getSignalColor(data.rssi) + ';\">';";
    html += "        html += '<div style=\"color:#666;font-size:0.9em;margin-bottom:5px\">Status</div>';";
    html += "        html += '<div style=\"font-size:1.3em;font-weight:bold;color:#333\">' + data.qualityText + '</div>';";
    html += "        html += '</div>';";
    html += "        html += '</div>';";
    html += "      } else {";
    html += "        html += '<p style=\"text-align:center;color:#666;padding:20px\">Not connected to any WiFi network</p>';";
    html += "      }";
    html += "      document.getElementById('currentSignal').innerHTML = html;";
    html += "    })";
    html += "    .catch(err => {";
    html += "      document.getElementById('currentSignal').innerHTML = '<p style=\"color:#ef4444;text-align:center\">Error loading signal data</p>';";
    html += "    });";
    html += "}";
    
    // Scan nearby networks
    html += "function scanNetworks() {";
    html += "  if (scanInProgress) return;";
    html += "  scanInProgress = true;";
    html += "  lastScanTime = Date.now();";
    html += "  document.getElementById('scanBtn').disabled = true;";
    html += "  document.getElementById('scanBtn').style.opacity = '0.6';";
    html += "  if (autoScanEnabled) {";
    html += "    document.getElementById('scanStatus').innerHTML = '🔄 Scanning...';";
    html += "  }";
    html += "  fetch('/signal/api?scan=1')";
    html += "    .then(response => response.json())";
    html += "    .then(data => {";
    html += "      scanInProgress = false;";
    html += "      lastScanTime = Date.now();";
    html += "      document.getElementById('scanBtn').disabled = false;";
    html += "      document.getElementById('scanBtn').style.opacity = '1';";
    html += "      if (data.networks && data.networks.length > 0) {";
    html += "        let html = '<ul class=\"network-list\">';";
    html += "        data.networks.forEach(function(network) {";
    html += "          html += '<li class=\"network-item\" style=\"cursor:default\">';";
    html += "          html += '<div class=\"network-info\">';";
    html += "          let displayName = network.ssid;";
    html += "          let isHidden = network.ssid.includes('Hidden Network');";
    html += "          if (isHidden) {";
    html += "            html += '<div class=\"network-name\" style=\"color:#999;font-style:italic\">' + displayName;";
    html += "          } else {";
    html += "            html += '<div class=\"network-name\">' + displayName;";
    html += "          }";
    html += "          if (network.connected) html += ' <span class=\"badge success\">CONNECTED</span>';";
    html += "          html += '</div>';";
    html += "          html += '<div class=\"network-details\">Quality: ' + network.quality + '% (' + network.qualityText + ')</div>';";
    html += "          html += '</div>';";
    html += "          html += '<div class=\"signal-strength\">' + createSignalBars(network.rssi) + '</div>';";
    html += "          html += '</li>';";
    html += "        });";
    html += "        html += '</ul>';";
    html += "        html += '<p style=\"text-align:center;margin-top:20px\"><strong>Found ' + data.networks.length + ' network(s)</strong></p>';";
    html += "        const scanTime = new Date().toLocaleTimeString();";
    html += "        html += '<p style=\"text-align:center;color:#666;font-size:0.9em;margin-top:5px\">Last scan: ' + scanTime + '</p>';";
    html += "        document.getElementById('nearbySignals').innerHTML = html;";
    html += "      } else {";
    html += "        document.getElementById('nearbySignals').innerHTML = '<p style=\"text-align:center;padding:40px;color:#666\">No networks found. Try scanning again.</p>';";
    html += "      }";
    html += "    })";
    html += "    .catch(err => {";
    html += "      scanInProgress = false;";
    html += "      document.getElementById('scanBtn').disabled = false;";
    html += "      document.getElementById('scanBtn').style.opacity = '1';";
    html += "      document.getElementById('nearbySignals').innerHTML = '<p style=\"color:#ef4444;text-align:center;padding:40px\">❌ Error scanning networks</p>';";
    html += "    });";
    html += "}";
    
    // Auto-refresh current signal
    html += "function startAutoRefresh() {";
    html += "  updateCurrentSignal();";
    html += "  refreshInterval = setInterval(updateCurrentSignal, 3000);";
    html += "}";
    
    html += "function stopAutoRefresh() {";
    html += "  clearInterval(refreshInterval);";
    html += "  stopAutoScan();";
    html += "}";
    
    html += "window.onload = startAutoRefresh;";
    html += "window.onbeforeunload = stopAutoRefresh;";
    html += "</script>";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

// ==========================================
// SIGNAL STRENGTH API ENDPOINT
// ==========================================
void handleSignalStrengthAPI() {
    String json = "{";
    
    // Check if requesting current signal or scan
    if (webServer->hasArg("current")) {
        // Get current connection signal
        SignalInfo info = getCurrentSignalStrength();
        
        json += "\"connected\":" + String(info.isConnected ? "true" : "false") + ",";
        json += "\"ssid\":\"" + info.ssid + "\",";
        json += "\"rssi\":" + String(info.rssi) + ",";
        json += "\"quality\":" + String(info.quality) + ",";
        json += "\"qualityText\":\"" + info.qualityText + "\",";
        json += "\"timestamp\":" + String(info.timestamp);
        
    } else if (webServer->hasArg("scan")) {
        // Scan nearby networks
        std::vector<SignalInfo> networks = getNearbySignalStrengths(20);
        
        json += "\"count\":" + String(networks.size()) + ",";
        json += "\"networks\":[";
        
        for (size_t i = 0; i < networks.size(); i++) {
            if (i > 0) json += ",";
            json += "{";
            json += "\"ssid\":\"" + networks[i].ssid + "\",";
            json += "\"rssi\":" + String(networks[i].rssi) + ",";
            json += "\"quality\":" + String(networks[i].quality) + ",";
            json += "\"qualityText\":\"" + networks[i].qualityText + "\",";
            json += "\"connected\":" + String(networks[i].isConnected ? "true" : "false");
            json += "}";
        }
        
        json += "]";
    } else {
        json += "\"error\":\"Invalid request\"";
    }
    
    json += "}";
    
    webServer->send(200, "application/json", json);
}

// ==========================================
// PORT SCANNER PAGE
// ==========================================
void handlePortScanner() {
    String html = FPSTR(HTML_HEADER);
    
    html += R"rawliteral(
    <div class="header">
        <h1>🔒 Port Scanner</h1>
        <p>Network Security Audit & Port Analysis</p>
    </div>
    )rawliteral";
    
    html += generateNav();
    
    // Scan Configuration
    html += R"rawliteral(
    <h2>🎯 Scan Configuration</h2>
    <div style="background:#f8f9fa;padding:25px;border-radius:10px;margin:20px 0">
        <div style="display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:20px">
            <div>
                <label style="display:block;font-weight:500;margin-bottom:8px;color:#333">Target IP Address:</label>
                <input type="text" id="targetIP" placeholder="192.168.1.100" style="width:100%;padding:12px;border:2px solid #667eea;border-radius:5px;font-size:1em" value="">
            </div>
            
            <div>
                <label style="display:block;font-weight:500;margin-bottom:8px;color:#333">Scan Type:</label>
                <select id="scanType" onchange="togglePortRange()" style="width:100%;padding:12px;border:2px solid #667eea;border-radius:5px;font-size:1em;cursor:pointer">
                    <option value="common">Common Ports (Fast)</option>
                    <option value="range">Custom Range</option>
                    <option value="well-known">Well-Known (1-1024)</option>
                    <option value="all">All Ports (1-65535)</option>
                </select>
            </div>
        </div>
        
        <div id="portRangeDiv" style="display:none;margin-top:20px">
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:20px">
                <div>
                    <label style="display:block;font-weight:500;margin-bottom:8px;color:#333">Start Port:</label>
                    <input type="number" id="startPort" min="1" max="65535" value="1" style="width:100%;padding:12px;border:2px solid #667eea;border-radius:5px;font-size:1em">
                </div>
                <div>
                    <label style="display:block;font-weight:500;margin-bottom:8px;color:#333">End Port:</label>
                    <input type="number" id="endPort" min="1" max="65535" value="1024" style="width:100%;padding:12px;border:2px;solid #667eea;border-radius:5px;font-size:1em">
                </div>
            </div>
        </div>
        
        <div style="margin-top:20px;display:flex;gap:15px;flex-wrap:wrap">
            <button id="startScanBtn" onclick="startPortScan()" style="padding:15px 40px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;border:none;border-radius:8px;font-size:1.1em;font-weight:bold;cursor:pointer;box-shadow:0 4px 12px rgba(102,126,234,0.4)">
                🔍 Start Scan
            </button>
            <button id="stopScanBtn" onclick="stopPortScan()" disabled style="padding:15px 40px;background:#ef4444;color:white;border:none;border-radius:8px;font-size:1.1em;font-weight:bold;cursor:pointer;opacity:0.5">
                ⏸️ Stop Scan
            </button>
        </div>
        
        <div id="scanStatus" style="margin-top:20px"></div>
    </div>

    <h2>📊 Scan Results</h2>
    <div id="scanResults" style="background:#f8f9fa;padding:25px;border-radius:10px;margin:20px 0">
        <p style="text-align:center;color:#666">Configure scan parameters above and click "Start Scan" to begin port analysis.</p>
    </div>

    <h2>⚠️ Important Notes</h2>
    <div style="background:#fff3cd;padding:20px;border-left:4px solid #ffc107;border-radius:5px;margin:20px 0">
        <ul style="margin:10px 0;padding-left:25px">
            <li><strong>Authorization:</strong> Only scan devices you own or have permission to test</li>
            <li><strong>Network Impact:</strong> Port scanning generates network traffic and may trigger security alerts</li>
            <li><strong>Legal Disclaimer:</strong> Unauthorized port scanning may violate laws and network policies</li>
            <li><strong>Scan Duration:</strong> Full port scans (1-65535) can take considerable time</li>
            <li><strong>Common Ports:</strong> Recommended for quick security assessment</li>
        </ul>
    </div>
    )rawliteral";
    
    // JavaScript
    html += "<script>";
    html += "let scanInterval;";
    html += "let scanRunning = false;";
    
    // Set default IP to gateway
    html += "window.onload = function() {";
    html += "  fetch('/portscan/api?gateway=1')";
    html += "    .then(response => response.json())";
    html += "    .then(data => {";
    html += "      if (data.gateway) document.getElementById('targetIP').value = data.gateway;";
    html += "    });";
    html += "};";
    
    // Toggle port range inputs
    html += "function togglePortRange() {";
    html += "  const scanType = document.getElementById('scanType').value;";
    html += "  const rangeDiv = document.getElementById('portRangeDiv');";
    html += "  rangeDiv.style.display = (scanType === 'range') ? 'block' : 'none';";
    html += "}";
    
    // Start scan
    html += "function startPortScan() {";
    html += "  const targetIP = document.getElementById('targetIP').value;";
    html += "  const scanType = document.getElementById('scanType').value;";
    html += "  if (!targetIP) { alert('Please enter target IP address'); return; }";
    html += "  let url = '/portscan/start?ip=' + encodeURIComponent(targetIP) + '&type=' + scanType;";
    html += "  if (scanType === 'range') {";
    html += "    const start = document.getElementById('startPort').value;";
    html += "    const end = document.getElementById('endPort').value;";
    html += "    url += '&start=' + start + '&end=' + end;";
    html += "  }";
    html += "  fetch(url)";
    html += "    .then(response => response.json())";
    html += "    .then(data => {";
    html += "      if (data.success) {";
    html += "        scanRunning = true;";
    html += "        document.getElementById('startScanBtn').disabled = true;";
    html += "        document.getElementById('startScanBtn').style.opacity = '0.5';";
    html += "        document.getElementById('stopScanBtn').disabled = false;";
    html += "        document.getElementById('stopScanBtn').style.opacity = '1';";
    html += "        document.getElementById('scanResults').innerHTML = '<p style=\"text-align:center;color:#667eea\">🔄 Initializing scan...</p>';";
    html += "        scanInterval = setInterval(updateScanStatus, 1000);";
    html += "      } else {";
    html += "        alert('Failed to start scan: ' + (data.error || 'Unknown error'));";
    html += "      }";
    html += "    });";
    html += "}";
    
    // Stop scan
    html += "function stopPortScan() {";
    html += "  fetch('/portscan/stop')";
    html += "    .then(() => {";
    html += "      clearInterval(scanInterval);";
    html += "      scanRunning = false;";
    html += "      document.getElementById('startScanBtn').disabled = false;";
    html += "      document.getElementById('startScanBtn').style.opacity = '1';";
    html += "      document.getElementById('stopScanBtn').disabled = true;";
    html += "      document.getElementById('stopScanBtn').style.opacity = '0.5';";
    html += "      document.getElementById('scanStatus').innerHTML = '<p style=\"color:#ef4444;font-weight:500\">⏸️ Scan stopped by user</p>';";
    html += "    });";
    html += "}";
    
    // Update scan status
    html += "function updateScanStatus() {";
    html += "  fetch('/portscan/status')";
    html += "    .then(response => response.json())";
    html += "    .then(data => {";
    html += "      if (data.state === 'running') {";
    html += "        const progress = data.progress || 0;";
    html += "        document.getElementById('scanStatus').innerHTML = ";
    html += "          '<div style=\"margin-top:10px\">' +";
    html += "          '<div style=\"background:#e5e7eb;border-radius:5px;height:30px;position:relative;overflow:hidden\">' +";
    html += "          '<div style=\"background:linear-gradient(135deg,#667eea,#764ba2);height:100%;width:' + progress + '%;transition:width 0.3s\"></div>' +";
    html += "          '<div style=\"position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);font-weight:bold;color:#333\">' + progress + '%</div>' +";
    html += "          '</div>' +";
    html += "          '<p style=\"margin-top:10px;text-align:center;color:#666\">Scanning port ' + data.currentPort + ' of ' + data.totalPorts + '</p>' +";
    html += "          '</div>';";
    html += "        if (data.openPorts > 0) {";
    html += "          displayResults(data);";
    html += "        }";
    html += "      } else if (data.state === 'completed') {";
    html += "        clearInterval(scanInterval);";
    html += "        scanRunning = false;";
    html += "        document.getElementById('startScanBtn').disabled = false;";
    html += "        document.getElementById('startScanBtn').style.opacity = '1';";
    html += "        document.getElementById('stopScanBtn').disabled = true;";
    html += "        document.getElementById('stopScanBtn').style.opacity = '0.5';";
    html += "        document.getElementById('scanStatus').innerHTML = '<p style=\"color:#10b981;font-weight:500\">✅ Scan completed in ' + data.duration + ' seconds</p>';";
    html += "        displayResults(data);";
    html += "      }";
    html += "    });";
    html += "}";
    
    // Display results
    html += "function displayResults(data) {";
    html += "  let html = '';";
    html += "  if (data.openPorts === 0) {";
    html += "    html = '<p style=\"text-align:center;color:#666\">No open ports found</p>';";
    html += "  } else {";
    html += "    html += '<div style=\"margin-bottom:15px\">';";
    html += "    html += '<p style=\"font-weight:500;font-size:1.1em\">Found ' + data.openPorts + ' open port(s) on ' + data.targetIP + '</p>';";
    html += "    html += '</div>';";
    html += "    html += '<table style=\"width:100%;border-collapse:collapse\">';";
    html += "    html += '<tr style=\"background:#667eea;color:white\">';";
    html += "    html += '<th style=\"padding:12px;text-align:left\">Port</th>';";
    html += "    html += '<th style=\"padding:12px;text-align:left\">Service</th>';";
    html += "    html += '<th style=\"padding:12px;text-align:center\">Status</th>';";
    html += "    html += '</tr>';";
    html += "    data.ports.forEach(function(port) {";
    html += "      html += '<tr style=\"border-bottom:1px solid #ddd\">';";
    html += "      html += '<td style=\"padding:12px;font-weight:500\">' + port.port + '</td>';";
    html += "      html += '<td style=\"padding:12px\">' + port.service + '</td>';";
    html += "      html += '<td style=\"padding:12px;text-align:center\"><span style=\"background:#10b981;color:white;padding:4px 12px;border-radius:12px;font-size:0.9em\">OPEN</span></td>';";
    html += "      html += '</tr>';";
    html += "    });";
    html += "    html += '</table>';";
    html += "  }";
    html += "  document.getElementById('scanResults').innerHTML = html;";
    html += "}";
    
    html += "</script>";
    
    html += generateHtmlFooter();
    webServer->send(200, "text/html", html);
}

// ==========================================
// PORT SCANNER START ENDPOINT
// ==========================================
void handlePortScanStart() {
    String json = "{";
    
    if (!webServer->hasArg("ip") || !webServer->hasArg("type")) {
        json += "\"success\":false,\"error\":\"Missing parameters\"";
        json += "}";
        webServer->send(400, "application/json", json);
        return;
    }
    
    String targetIP = webServer->arg("ip");
    String scanType = webServer->arg("type");
    
    bool started = false;
    
    if (scanType == "common") {
        started = startCommonPortScan(targetIP);
    } else if (scanType == "well-known") {
        started = startPortScan(targetIP, 1, 1024);
    } else if (scanType == "all") {
        started = startPortScan(targetIP, 1, 65535);
    } else if (scanType == "range") {
        if (webServer->hasArg("start") && webServer->hasArg("end")) {
            uint16_t startPort = webServer->arg("start").toInt();
            uint16_t endPort = webServer->arg("end").toInt();
            started = startPortScan(targetIP, startPort, endPort);
        }
    }
    
    json += "\"success\":" + String(started ? "true" : "false");
    if (!started) {
        json += ",\"error\":\"Failed to start scan\"";
    }
    json += "}";
    
    webServer->send(200, "application/json", json);
}

// ==========================================
// PORT SCANNER STOP ENDPOINT
// ==========================================
void handlePortScanStop() {
    stopPortScan();
    webServer->send(200, "application/json", "{\"success\":true}");
}

// ==========================================
// PORT SCANNER STATUS ENDPOINT
// ==========================================
void handlePortScanStatus() {
    PortScanResults results = getLastPortScanResults();
    PortScanState state = getPortScanState();
    
    String json = "{";
    json += "\"state\":\"";
    switch(state) {
        case PORTSCAN_IDLE: json += "idle"; break;
        case PORTSCAN_RUNNING: json += "running"; break;
        case PORTSCAN_COMPLETED: json += "completed"; break;
        case PORTSCAN_ERROR: json += "error"; break;
    }
    json += "\",";
    json += "\"targetIP\":\"" + results.targetIP + "\",";
    json += "\"totalPorts\":" + String(results.totalPorts) + ",";
    json += "\"portsScanned\":" + String(results.portsScanned) + ",";
    json += "\"currentPort\":" + String(results.portsScanned + 1) + ",";
    json += "\"openPorts\":" + String(results.openPorts) + ",";
    json += "\"closedPorts\":" + String(results.closedPorts) + ",";
    json += "\"progress\":" + String(getPortScanProgress()) + ",";
    
    if (results.scanCompleted) {
        unsigned long duration = (results.endTime - results.startTime) / 1000;
        json += "\"duration\":" + String(duration) + ",";
    }
    
    json += "\"ports\":[";
    for (size_t i = 0; i < results.openPortsList.size(); i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"port\":" + String(results.openPortsList[i].port) + ",";
        json += "\"service\":\"" + results.openPortsList[i].service + "\"";
        json += "}";
    }
    json += "]";
    json += "}";
    
    webServer->send(200, "application/json", json);
}

// ==========================================
// PORT SCANNER API ENDPOINT
// ==========================================
void handlePortScanAPI() {
    String json = "{";
    
    if (webServer->hasArg("gateway")) {
        // Return gateway IP
        if (WiFi.status() == WL_CONNECTED) {
            json += "\"gateway\":\"" + WiFi.gatewayIP().toString() + "\"";
        } else {
            json += "\"gateway\":\"\"";
        }
    } else {
        json += "\"error\":\"Invalid request\"";
    }
    
    json += "}";
    webServer->send(200, "application/json", json);
}


#endif // USE_WEBSERVER
