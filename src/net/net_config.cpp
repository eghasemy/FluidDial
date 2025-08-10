// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "net_config.h"

#ifdef USE_WIFI_PENDANT

#include "System.h"

// Network configuration implementation (stub)
// Future implementation will use ESP32 WiFi APIs

bool NetConfig::init() {
    // Stub implementation
    // Future: Initialize WiFi subsystem
    return true;
}

bool NetConfig::connectWifi(const char* ssid, const char* password) {
    // Stub implementation
    // Future: Connect to WiFi network using ESP32 WiFi APIs
    (void)ssid;
    (void)password;
    return false; // Not connected yet
}

bool NetConfig::isWifiConnected() {
    // Stub implementation
    // Future: Check WiFi connection status
    return false;
}

void NetConfig::disconnectWifi() {
    // Stub implementation
    // Future: Disconnect from WiFi
}

bool NetConfig::discoverFluidNCHost(char* host, size_t hostLen, int& port) {
    // Stub implementation
    // Future: Use mDNS or network scan to discover FluidNC instances
    (void)host;
    (void)hostLen;
    (void)port;
    return false;
}

bool NetConfig::testFluidNCConnection(const char* host, int port) {
    // Stub implementation
    // Future: Test TCP connection to FluidNC host
    (void)host;
    (void)port;
    return false;
}

const char* NetConfig::getWifiStatus() {
    // Stub implementation
    // Future: Return current WiFi status string
    return "WiFi Disabled";
}

const char* NetConfig::getLocalIP() {
    // Stub implementation
    // Future: Return local IP address
    return "0.0.0.0";
}

#endif // USE_WIFI_PENDANT