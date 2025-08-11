// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include <cstddef> // For size_t

#ifdef USE_WIFI_PENDANT

// Network storage interface for WiFi pendant configuration
// Handles persistent storage of network credentials and settings

class NetStore {
public:
    static bool init();
    static bool saveWifiCredentials(const char* ssid, const char* password);
    static bool loadWifiCredentials(char* ssid, size_t ssidLen, char* password, size_t passwordLen);
    static bool saveFluidNCHost(const char* host, int port);
    static bool loadFluidNCHost(char* host, size_t hostLen, int& port);
    static void clear();
    
    // New functions for comprehensive network settings
    static bool netSave(const char* ssid, const char* password, const char* host, int port, const char* transport);
    static bool netLoad(char* ssid, size_t ssidLen, char* password, size_t passwordLen, 
                       char* host, size_t hostLen, int& port, char* transport, size_t transportLen);
    
    // Extended functions with connection type
    static bool netSaveWithConnectionType(const char* ssid, const char* password, const char* host, int port, 
                                         const char* transport, const char* connection_type);
    static bool netLoadWithConnectionType(char* ssid, size_t ssidLen, char* password, size_t passwordLen, 
                                         char* host, size_t hostLen, int& port, char* transport, size_t transportLen,
                                         char* connection_type, size_t connectionTypeLen);
};

#endif // USE_WIFI_PENDANT