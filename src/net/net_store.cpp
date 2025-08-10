// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "net_store.h"

#ifdef USE_WIFI_PENDANT

#include "System.h"

// Network storage implementation (stub)
// Future implementation will use ESP32 NVS or similar for persistent storage

bool NetStore::init() {
    // Stub implementation
    // Future: Initialize NVS partition for network settings
    return true;
}

bool NetStore::saveWifiCredentials(const char* ssid, const char* password) {
    // Stub implementation
    // Future: Store WiFi credentials in NVS
    (void)ssid;
    (void)password;
    return true;
}

bool NetStore::loadWifiCredentials(char* ssid, size_t ssidLen, char* password, size_t passwordLen) {
    // Stub implementation  
    // Future: Load WiFi credentials from NVS
    (void)ssid;
    (void)ssidLen;
    (void)password;
    (void)passwordLen;
    return false; // No credentials stored yet
}

bool NetStore::saveFluidNCHost(const char* host, int port) {
    // Stub implementation
    // Future: Store FluidNC host/port in NVS
    (void)host;
    (void)port;
    return true;
}

bool NetStore::loadFluidNCHost(char* host, size_t hostLen, int& port) {
    // Stub implementation
    // Future: Load FluidNC host/port from NVS
    (void)host;
    (void)hostLen;
    (void)port;
    return false; // No host configured yet
}

void NetStore::clear() {
    // Stub implementation
    // Future: Clear all network settings from NVS
}

#endif // USE_WIFI_PENDANT