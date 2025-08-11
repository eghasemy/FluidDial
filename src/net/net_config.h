// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include <cstddef> // For size_t

#ifdef USE_WIFI_PENDANT

// Network configuration interface for WiFi pendant
// Handles WiFi connection management and FluidNC host discovery

class NetConfig {
public:
    // Core WiFi Management Functions (as required by issue)
    static bool init(); // wifiInit() - mounts FS and loads config
    static bool wifiConnectAsync(); // attempt join, non-blocking
    static bool wifiReady(); // track WL_CONNECTED state with reconnect backoff
    
    // Additional helper functions
    static bool connectWifi(const char* ssid, const char* password);
    static bool isWifiConnected();
    static void disconnectWifi();
    static bool discoverFluidNCHost(char* host, size_t hostLen, int& port);
    static bool testFluidNCConnection(const char* host, int port);
    static bool testFluidNCConnectionWithTransport(const char* host, int port, const char* transport_type);
    static const char* getWifiStatus();
    static const char* getLocalIP();
};

// Alias functions to match issue requirements exactly
#define wifiInit() NetConfig::init()
#define wifiConnectAsync() NetConfig::wifiConnectAsync()
#define wifiReady() NetConfig::wifiReady()

#endif // USE_WIFI_PENDANT