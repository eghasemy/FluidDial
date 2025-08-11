// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "net_config.h"

#ifdef USE_WIFI_PENDANT

#include "System.h"
#include "net_store.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <LittleFS.h>
#include <WebSocketsClient.h>

// WiFi connection state management
static wl_status_t lastWifiStatus = WL_IDLE_STATUS;
static unsigned long lastConnectionAttempt = 0;
static unsigned long reconnectDelay = 5000; // Start with 5 second delay
static const unsigned long MAX_RECONNECT_DELAY = 60000; // Maximum 60 second delay
static bool connectionInProgress = false;
static char currentSSID[64] = "";
static char currentPassword[64] = "";
static bool wifiInitialized = false;

bool NetConfig::init() {
    if (wifiInitialized) {
        return true;
    }
    
    // Mount filesystem if not already mounted
    if (!LittleFS.begin()) {
        dbg_printf("WiFi: LittleFS mount failed, attempting format...\n");
        if (!LittleFS.format() || !LittleFS.begin()) {
            dbg_printf("WiFi: LittleFS format/mount failed\n");
            return false;
        }
    }
    
    // Initialize NetStore
    if (!NetStore::init()) {
        dbg_printf("WiFi: NetStore initialization failed\n");
        return false;
    }
    
    // Load WiFi credentials from storage
    if (NetStore::loadWifiCredentials(currentSSID, sizeof(currentSSID), 
                                     currentPassword, sizeof(currentPassword))) {
        dbg_printf("WiFi: Loaded credentials for SSID: %s\n", currentSSID);
    } else {
        dbg_printf("WiFi: No saved credentials found\n");
    }
    
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    lastWifiStatus = WiFi.status();
    wifiInitialized = true;
    
    dbg_printf("WiFi: Initialization complete\n");
    return true;
}

bool NetConfig::connectWifi(const char* ssid, const char* password) {
    if (!wifiInitialized && !init()) {
        return false;
    }
    
    if (!ssid || strlen(ssid) == 0) {
        dbg_printf("WiFi: Invalid SSID provided\n");
        return false;
    }
    
    // Store credentials for use in async connection
    strlcpy(currentSSID, ssid, sizeof(currentSSID));
    strlcpy(currentPassword, password ? password : "", sizeof(currentPassword));
    
    // Save credentials to persistent storage
    NetStore::saveWifiCredentials(ssid, password);
    
    // Start connection attempt
    return wifiConnectAsync();
}

bool NetConfig::wifiConnectAsync() {
    if (!wifiInitialized) {
        return false;
    }
    
    if (strlen(currentSSID) == 0) {
        dbg_printf("WiFi: No SSID configured for connection\n");
        return false;
    }
    
    unsigned long now = millis();
    
    // Check if we're already connected
    if (WiFi.status() == WL_CONNECTED) {
        reconnectDelay = 5000; // Reset delay on successful connection
        connectionInProgress = false;
        return true;
    }
    
    // Check if we should attempt connection (respecting backoff delay)
    if (connectionInProgress && (now - lastConnectionAttempt < reconnectDelay)) {
        return false; // Still waiting for current attempt or backoff
    }
    
    dbg_printf("WiFi: Attempting connection to %s\n", currentSSID);
    connectionInProgress = true;
    lastConnectionAttempt = now;
    
    // Start non-blocking connection
    WiFi.begin(currentSSID, currentPassword);
    
    return true;
}

bool NetConfig::wifiReady() {
    if (!wifiInitialized) {
        return false;
    }
    
    wl_status_t currentStatus = WiFi.status();
    
    // Handle status change events
    if (currentStatus != lastWifiStatus) {
        switch (currentStatus) {
            case WL_CONNECTED:
                dbg_printf("WiFi: Connected to %s, IP: %s\n", currentSSID, WiFi.localIP().toString().c_str());
                WiFi.setSleep(false); // Disable WiFi sleep for better performance
                reconnectDelay = 5000; // Reset backoff delay
                connectionInProgress = false;
                break;
                
            case WL_CONNECT_FAILED:
                dbg_printf("WiFi: Connection failed\n");
                connectionInProgress = false;
                // Increase backoff delay for next attempt
                reconnectDelay = min(reconnectDelay * 2, MAX_RECONNECT_DELAY);
                break;
                
            case WL_NO_SSID_AVAIL:
                dbg_printf("WiFi: SSID not available\n");
                connectionInProgress = false;
                reconnectDelay = min(reconnectDelay * 2, MAX_RECONNECT_DELAY);
                break;
                
            case WL_CONNECTION_LOST:
                dbg_printf("WiFi: Connection lost, will attempt reconnect\n");
                connectionInProgress = false;
                // Try to reconnect automatically after a short delay
                break;
                
            default:
                break;
        }
        lastWifiStatus = currentStatus;
    }
    
    // Automatic reconnection logic
    if (currentStatus != WL_CONNECTED && strlen(currentSSID) > 0) {
        wifiConnectAsync(); // Will respect backoff timing
    }
    
    return (currentStatus == WL_CONNECTED);
}

bool NetConfig::isWifiConnected() {
    return wifiInitialized && (WiFi.status() == WL_CONNECTED);
}

void NetConfig::disconnectWifi() {
    if (wifiInitialized) {
        WiFi.disconnect();
        connectionInProgress = false;
        dbg_printf("WiFi: Disconnected\n");
    }
}

bool NetConfig::discoverFluidNCHost(char* host, size_t hostLen, int& port) {
    // For now, load from configuration
    // Future enhancement: implement mDNS discovery
    return NetStore::loadFluidNCHost(host, hostLen, port);
}

bool NetConfig::testFluidNCConnection(const char* host, int port) {
    if (!isWifiConnected()) {
        return false;
    }
    
    // Test actual TCP connection to FluidNC host
    WiFiClient client;
    client.setTimeout(5000); // 5 second timeout
    
    dbg_printf("Testing FluidNC connection to %s:%d\n", host, port);
    
    bool connected = client.connect(host, port);
    if (connected) {
        dbg_printf("FluidNC connection test successful\n");
        client.stop();
        return true;
    } else {
        dbg_printf("FluidNC connection test failed\n");
        return false;
    }
}

bool NetConfig::testFluidNCConnectionWithTransport(const char* host, int port, const char* transport_type) {
    if (!isWifiConnected()) {
        dbg_printf("Transport test: WiFi not connected\n");
        return false;
    }
    
    if (!transport_type) {
        dbg_printf("Transport test: No transport type specified\n");
        return false;
    }
    
    dbg_printf("Testing FluidNC connection to %s:%d using %s transport\n", host, port, transport_type);
    
    if (strcmp(transport_type, "tcp") == 0) {
        // For TCP/Telnet, use the existing TCP test
        return testFluidNCConnection(host, port);
    } else if (strcmp(transport_type, "ws") == 0) {
        // For WebSocket, test the actual WebSocket connection
        WebSocketsClient wsClient;
        bool connected = false;
        bool connectionFailed = false;
        
        // Set up event handler to track connection status
        wsClient.onEvent([&](WStype_t type, uint8_t* payload, size_t length) {
            switch (type) {
                case WStype_CONNECTED:
                    dbg_printf("Transport test: WebSocket connected to %s\n", payload);
                    connected = true;
                    break;
                case WStype_DISCONNECTED:
                    dbg_printf("Transport test: WebSocket disconnected\n");
                    break;
                case WStype_ERROR:
                    dbg_printf("Transport test: WebSocket error\n");
                    connectionFailed = true;
                    break;
                default:
                    break;
            }
        });
        
        // Attempt WebSocket connection
        wsClient.begin(host, port, "/");
        
        // Wait up to 10 seconds for connection
        unsigned long startTime = millis();
        const unsigned long timeout = 10000; // 10 seconds
        
        while (!connected && !connectionFailed && (millis() - startTime < timeout)) {
            wsClient.loop();
            delay(100); // Small delay to prevent busy waiting
        }
        
        wsClient.disconnect();
        
        if (connected) {
            dbg_printf("Transport test: WebSocket connection successful\n");
            return true;
        } else {
            dbg_printf("Transport test: WebSocket connection failed (timeout or error)\n");
            return false;
        }
    } else {
        dbg_printf("Transport test: Unknown transport type: %s\n", transport_type);
        return false;
    }
}

const char* NetConfig::getWifiStatus() {
    if (!wifiInitialized) {
        return "WiFi Not Initialized";
    }
    
    switch (WiFi.status()) {
        case WL_CONNECTED:
            return "Connected";
        case WL_NO_SSID_AVAIL:
            return "SSID Not Found";
        case WL_CONNECT_FAILED:
            return "Connection Failed";
        case WL_CONNECTION_LOST:
            return "Connection Lost";
        case WL_DISCONNECTED:
            return "Disconnected";
        case WL_IDLE_STATUS:
        default:
            if (connectionInProgress) {
                return "Connecting...";
            }
            return "Idle";
    }
}

const char* NetConfig::getLocalIP() {
    if (isWifiConnected()) {
        static String ipStr = WiFi.localIP().toString();
        return ipStr.c_str();
    }
    return "0.0.0.0";
}

#endif // USE_WIFI_PENDANT