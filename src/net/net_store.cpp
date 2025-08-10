// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "net_store.h"

#ifdef USE_WIFI_PENDANT

#include "System.h"
#include <ArduinoJson.h>

// Network storage implementation using LittleFS-backed JSON file
const char* NET_CONFIG_FILE = "/net.json";

bool NetStore::init() {
    // LittleFS is already initialized in SystemArduino.cpp
    return true;
}

bool NetStore::saveWifiCredentials(const char* ssid, const char* password) {
    // Load existing settings
    char host[64] = "fluidnc.local";
    int port = 81;
    char transport[16] = "ws";
    
    // Try to load existing host/port/transport settings
    JsonDocument doc;
    File file = LittleFS.open(NET_CONFIG_FILE, "r");
    if (file) {
        if (deserializeJson(doc, file) == DeserializationError::Ok) {
            strlcpy(host, doc["host"] | "fluidnc.local", sizeof(host));
            port = doc["port"] | 81;
            strlcpy(transport, doc["transport"] | "ws", sizeof(transport));
        }
        file.close();
    }
    
    return netSave(ssid, password, host, port, transport);
}

bool NetStore::loadWifiCredentials(char* ssid, size_t ssidLen, char* password, size_t passwordLen) {
    char host[64];
    int port;
    char transport[16];
    
    return netLoad(ssid, ssidLen, password, passwordLen, host, sizeof(host), port, transport, sizeof(transport));
}

bool NetStore::saveFluidNCHost(const char* host, int port) {
    // Load existing WiFi credentials
    char ssid[64] = "";
    char password[64] = "";
    char transport[16] = "ws";
    
    // Try to load existing WiFi/transport settings
    JsonDocument doc;
    File file = LittleFS.open(NET_CONFIG_FILE, "r");
    if (file) {
        if (deserializeJson(doc, file) == DeserializationError::Ok) {
            strlcpy(ssid, doc["ssid"] | "", sizeof(ssid));
            strlcpy(password, doc["pass"] | "", sizeof(password));
            strlcpy(transport, doc["transport"] | "ws", sizeof(transport));
        }
        file.close();
    }
    
    return netSave(ssid, password, host, port, transport);
}

bool NetStore::loadFluidNCHost(char* host, size_t hostLen, int& port) {
    char ssid[64];
    char password[64];
    char transport[16];
    
    if (netLoad(ssid, sizeof(ssid), password, sizeof(password), host, hostLen, port, transport, sizeof(transport))) {
        return true;
    }
    
    // Return defaults if no config found
    strlcpy(host, "fluidnc.local", hostLen);
    port = 81;
    return false;
}

void NetStore::clear() {
    // Remove the network configuration file
    if (LittleFS.exists(NET_CONFIG_FILE)) {
        LittleFS.remove(NET_CONFIG_FILE);
    }
}

// Core JSON-based network settings storage functions
bool NetStore::netSave(const char* ssid, const char* password, const char* host, int port, const char* transport) {
    JsonDocument doc;
    
    // Set values, using defaults where appropriate
    doc["ssid"] = ssid ? ssid : "";
    doc["pass"] = password ? password : "";
    doc["host"] = host ? host : "fluidnc.local";
    doc["port"] = port > 0 ? port : 81;
    doc["transport"] = transport ? transport : "ws";
    
    File file = LittleFS.open(NET_CONFIG_FILE, "w");
    if (!file) {
        return false;
    }
    
    bool success = (serializeJson(doc, file) > 0);
    file.close();
    
    return success;
}

bool NetStore::netLoad(char* ssid, size_t ssidLen, char* password, size_t passwordLen, 
                      char* host, size_t hostLen, int& port, char* transport, size_t transportLen) {
    // Set defaults first
    if (ssid && ssidLen > 0) ssid[0] = '\0';
    if (password && passwordLen > 0) password[0] = '\0';
    if (host && hostLen > 0) strlcpy(host, "fluidnc.local", hostLen);
    port = 81;
    if (transport && transportLen > 0) strlcpy(transport, "ws", transportLen);
    
    // Try to load from file
    if (!LittleFS.exists(NET_CONFIG_FILE)) {
        return false; // Missing file → defaults apply (as per requirements)
    }
    
    File file = LittleFS.open(NET_CONFIG_FILE, "r");
    if (!file) {
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        return false; // Parse error → defaults apply
    }
    
    // Load values from JSON, keeping defaults for missing keys
    if (ssid && ssidLen > 0) {
        strlcpy(ssid, doc["ssid"] | "", ssidLen);
    }
    if (password && passwordLen > 0) {
        strlcpy(password, doc["pass"] | "", passwordLen);
    }
    if (host && hostLen > 0) {
        strlcpy(host, doc["host"] | "fluidnc.local", hostLen);
    }
    port = doc["port"] | 81;
    if (transport && transportLen > 0) {
        strlcpy(transport, doc["transport"] | "ws", transportLen);
    }
    
    return true;
}

// Extended functions with connection type
bool NetStore::netSaveWithConnectionType(const char* ssid, const char* password, const char* host, int port, 
                                        const char* transport, const char* connection_type) {
    JsonDocument doc;
    
    // Set values, using defaults where appropriate
    doc["ssid"] = ssid ? ssid : "";
    doc["pass"] = password ? password : "";
    doc["host"] = host ? host : "fluidnc.local";
    doc["port"] = port > 0 ? port : 81;
    doc["transport"] = transport ? transport : "ws";
    doc["connection_type"] = connection_type ? connection_type : "WiFi";
    
    File file = LittleFS.open(NET_CONFIG_FILE, "w");
    if (!file) {
        return false;
    }
    
    bool success = (serializeJson(doc, file) > 0);
    file.close();
    
    return success;
}

bool NetStore::netLoadWithConnectionType(char* ssid, size_t ssidLen, char* password, size_t passwordLen, 
                                        char* host, size_t hostLen, int& port, char* transport, size_t transportLen,
                                        char* connection_type, size_t connectionTypeLen) {
    // Set defaults first
    if (ssid && ssidLen > 0) ssid[0] = '\0';
    if (password && passwordLen > 0) password[0] = '\0';
    if (host && hostLen > 0) strlcpy(host, "fluidnc.local", hostLen);
    port = 81;
    if (transport && transportLen > 0) strlcpy(transport, "ws", transportLen);
    if (connection_type && connectionTypeLen > 0) strlcpy(connection_type, "WiFi", connectionTypeLen);
    
    // Try to load from file
    if (!LittleFS.exists(NET_CONFIG_FILE)) {
        return false; // Missing file → defaults apply
    }
    
    File file = LittleFS.open(NET_CONFIG_FILE, "r");
    if (!file) {
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        return false; // Parse error → defaults apply
    }
    
    // Load values from JSON, keeping defaults for missing keys
    if (ssid && ssidLen > 0) {
        strlcpy(ssid, doc["ssid"] | "", ssidLen);
    }
    if (password && passwordLen > 0) {
        strlcpy(password, doc["pass"] | "", passwordLen);
    }
    if (host && hostLen > 0) {
        strlcpy(host, doc["host"] | "fluidnc.local", hostLen);
    }
    port = doc["port"] | 81;
    if (transport && transportLen > 0) {
        strlcpy(transport, doc["transport"] | "ws", transportLen);
    }
    if (connection_type && connectionTypeLen > 0) {
        strlcpy(connection_type, doc["connection_type"] | "WiFi", connectionTypeLen);
    }
    
    return true;
}

#endif // USE_WIFI_PENDANT