// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#ifdef USE_WIFI_PENDANT

#include "telnet_transport.h"
#include "System.h"

TelnetTransport::TelnetTransport(const char* host, int port) : _host(host), _port(port) {
}

TelnetTransport::~TelnetTransport() {
    if (_connected) {
        _client.stop();
    }
}

void TelnetTransport::setHost(const char* host, int port) {
    _host = host;
    _port = port;
}

bool TelnetTransport::begin() {
    if (!WiFi.isConnected()) {
        dbg_printf("TelnetTransport: WiFi not connected\n");
        return false;
    }
    
    dbg_printf("TelnetTransport: Connecting to %s:%d\n", _host.c_str(), _port);
    
    _initialized = true;
    _lastReconnectAttempt = millis();
    
    // Attempt initial connection
    attemptReconnect();
    
    return true;
}

void TelnetTransport::loop() {
    if (!_initialized) {
        return;
    }
    
    // Check if client is still connected
    if (_connected && !_client.connected()) {
        _connected = false;
        _client.stop();
        dbg_printf("TelnetTransport: Connection lost\n");
    }
    
    // Handle reconnection
    if (!_connected && WiFi.isConnected()) {
        attemptReconnect();
    }
}

void TelnetTransport::attemptReconnect() {
    unsigned long now = millis();
    if (now - _lastReconnectAttempt >= _reconnectInterval) {
        dbg_printf("TelnetTransport: Attempting reconnect to %s:%d\n", _host.c_str(), _port);
        
        if (_client.connect(_host.c_str(), _port)) {
            _connected = true;
            _reconnectInterval = 1500; // Reset reconnect interval on successful connection
            dbg_printf("TelnetTransport: Connected successfully\n");
        } else {
            _connected = false;
            // Exponential backoff, but cap at max interval
            _reconnectInterval = min(_reconnectInterval * 2, _maxReconnectInterval);
            dbg_printf("TelnetTransport: Connection failed, retry in %lu ms\n", _reconnectInterval);
        }
        
        _lastReconnectAttempt = now;
    }
}

bool TelnetTransport::isConnected() {
    return _connected && _client.connected() && WiFi.isConnected();
}

void TelnetTransport::sendLine(const char* line, int timeout) {
    if (!isConnected() || !line) {
        return;
    }
    
    // Send line with single newline appended
    _client.print(line);
    _client.print('\n');
    _client.flush();
    
    dbg_printf("TelnetTransport: Sent line: %s\n", line);
}

void TelnetTransport::sendRT(uint8_t c) {
    if (!isConnected()) {
        return;
    }
    
    // Send single byte
    _client.write(c);
    _client.flush();
    
    dbg_printf("TelnetTransport: Sent RT: 0x%02X\n", c);
}

int TelnetTransport::getChar() {
    if (!isConnected()) {
        return -1;
    }
    
    if (_client.available()) {
        return _client.read();
    }
    
    return -1; // No data available
}

void TelnetTransport::putChar(uint8_t c) {
    if (!isConnected()) {
        return;
    }
    
    _client.write(c);
}

void TelnetTransport::resetFlowControl() {
    // Send XON character for flow control reset
    sendRT(0x11);
}

#endif // USE_WIFI_PENDANT