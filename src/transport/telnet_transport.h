// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#ifdef USE_WIFI_PENDANT

#include "transport.h"
#include <WiFi.h>

// Telnet transport implementation for FluidNC communication
class TelnetTransport : public Transport {
private:
    WiFiClient _client;
    bool _connected = false;
    bool _initialized = false;
    String _host;
    int _port;
    unsigned long _lastReconnectAttempt = 0;
    unsigned long _reconnectInterval = 1500; // Start with 1.5s
    const unsigned long _maxReconnectInterval = 5000; // Max 5s
    
    void attemptReconnect();
    
public:
    TelnetTransport(const char* host, int port);
    ~TelnetTransport();
    
    // Transport interface implementation
    bool begin() override;
    void loop() override;
    bool isConnected() override;
    void sendLine(const char* line, int timeout = 2000) override;
    void sendRT(uint8_t c) override;
    int getChar() override;
    void putChar(uint8_t c) override;
    void resetFlowControl() override;
    
    // Telnet specific methods
    void setHost(const char* host, int port);
};

#endif // USE_WIFI_PENDANT