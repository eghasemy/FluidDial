// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#ifdef USE_WIFI_PENDANT

#include "transport.h"

// WiFi transport factory for creating WebSocket or Telnet transports
class WiFiTransportFactory {
public:
    enum TransportType {
        WEBSOCKET,
        TELNET
    };
    
    static Transport* createTransport(TransportType type, const char* host, int port);
    static Transport* createWSTransport(const char* host, int port);
    static Transport* createTelnetTransport(const char* host, int port);
};

#endif // USE_WIFI_PENDANT