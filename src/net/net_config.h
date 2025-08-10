// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#ifdef USE_WIFI_PENDANT

// Network configuration interface for WiFi pendant
// Handles WiFi connection management and FluidNC host discovery

class NetConfig {
public:
    static bool init();
    static bool connectWifi(const char* ssid, const char* password);
    static bool isWifiConnected();
    static void disconnectWifi();
    static bool discoverFluidNCHost(char* host, size_t hostLen, int& port);
    static bool testFluidNCConnection(const char* host, int port);
    static const char* getWifiStatus();
    static const char* getLocalIP();
};

#endif // USE_WIFI_PENDANT