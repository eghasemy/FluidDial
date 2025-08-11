// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#ifdef USE_WIFI_PENDANT

#include "transport_config.h"
#include "System.h"
#include <LittleFS.h>

// Static member definitions
const char* TransportConfig::CONFIG_FILE = "/transport.json";
TransportConfig::TransportType TransportConfig::_transportType = DEFAULT_TRANSPORT;
String TransportConfig::_host = DEFAULT_HOST;
int TransportConfig::_port = DEFAULT_WS_PORT;
bool TransportConfig::_configLoaded = false;

// Define static constexpr members for the linker
constexpr const char* TransportConfig::DEFAULT_HOST;
constexpr int TransportConfig::DEFAULT_WS_PORT;
constexpr int TransportConfig::DEFAULT_TELNET_PORT;
constexpr TransportConfig::TransportType TransportConfig::DEFAULT_TRANSPORT;

bool TransportConfig::loadConfig() {
    if (_configLoaded) {
        return true;
    }
    
    if (!LittleFS.exists(CONFIG_FILE)) {
        dbg_printf("TransportConfig: No config file found, using defaults\n");
        _configLoaded = true;
        return true;
    }
    
    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        dbg_printf("TransportConfig: Failed to open config file\n");
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        dbg_printf("TransportConfig: Failed to parse config JSON: %s\n", error.c_str());
        return false;
    }
    
    // Load transport type
    const char* typeStr = doc["type"] | "websocket";
    if (strcmp(typeStr, "telnet") == 0) {
        _transportType = TELNET;
        _port = doc["port"] | DEFAULT_TELNET_PORT;
    } else {
        _transportType = WEBSOCKET;
        _port = doc["port"] | DEFAULT_WS_PORT;
    }
    
    // Load host
    _host = doc["host"] | DEFAULT_HOST;
    
    dbg_printf("TransportConfig: Loaded - Type: %s, Host: %s, Port: %d\n", 
               typeStr, _host.c_str(), _port);
    
    _configLoaded = true;
    return true;
}

bool TransportConfig::saveConfig() {
    JsonDocument doc;
    
    doc["type"] = (_transportType == TELNET) ? "telnet" : "websocket";
    doc["host"] = _host;
    doc["port"] = _port;
    
    File file = LittleFS.open(CONFIG_FILE, "w");
    if (!file) {
        dbg_printf("TransportConfig: Failed to open config file for writing\n");
        return false;
    }
    
    if (serializeJson(doc, file) == 0) {
        dbg_printf("TransportConfig: Failed to write config JSON\n");
        file.close();
        return false;
    }
    
    file.close();
    dbg_printf("TransportConfig: Config saved successfully\n");
    return true;
}

void TransportConfig::invalidateCache() {
    _configLoaded = false;
    dbg_printf("TransportConfig: Cache invalidated, will reload on next access\n");
}

TransportConfig::TransportType TransportConfig::getTransportType() {
    loadConfig();
    return _transportType;
}

void TransportConfig::setTransportType(TransportType type) {
    _transportType = type;
    
    // Set default port for transport type if current port is default for other type
    if (type == WEBSOCKET && _port == DEFAULT_TELNET_PORT) {
        _port = DEFAULT_WS_PORT;
    } else if (type == TELNET && _port == DEFAULT_WS_PORT) {
        _port = DEFAULT_TELNET_PORT;
    }
}

const char* TransportConfig::getHost() {
    loadConfig();
    return _host.c_str();
}

void TransportConfig::setHost(const char* host) {
    _host = host;
}

int TransportConfig::getPort() {
    loadConfig();
    return _port;
}

void TransportConfig::setPort(int port) {
    _port = port;
}

#endif // USE_WIFI_PENDANT