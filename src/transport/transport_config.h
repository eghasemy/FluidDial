// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#ifdef USE_WIFI_PENDANT

#include <ArduinoJson.h>

// Transport configuration management
class TransportConfig {
public:
    enum TransportType {
        WEBSOCKET,
        TELNET
    };
    
    static bool loadConfig();
    static bool saveConfig();
    static TransportType getTransportType();
    static void setTransportType(TransportType type);
    static const char* getHost();
    static void setHost(const char* host);
    static int getPort();
    static void setPort(int port);
    
    // Default values
    static constexpr const char* DEFAULT_HOST = "192.168.1.100";
    static constexpr int DEFAULT_WS_PORT = 81;
    static constexpr int DEFAULT_TELNET_PORT = 23;
    static constexpr TransportType DEFAULT_TRANSPORT = WEBSOCKET;
    
private:
    static TransportType _transportType;
    static String _host;
    static int _port;
    static bool _configLoaded;
    
    static const char* CONFIG_FILE;
};

#endif // USE_WIFI_PENDANT