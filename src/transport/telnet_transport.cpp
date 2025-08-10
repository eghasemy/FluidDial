// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#ifdef USE_WIFI_PENDANT

#include "telnet_transport.h"
#include "System.h"
#include <ESPmDNS.h>

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
        
        // Check if hostname ends with .local and attempt mDNS resolution first
        bool connectionSuccess = false;
        if (_host.endsWith(".local")) {
            IPAddress mdnsIP = resolveMdnsHost(_host.c_str());
            if (mdnsIP != INADDR_NONE) {
                dbg_printf("TelnetTransport: mDNS resolved %s to %s\n", _host.c_str(), mdnsIP.toString().c_str());
                connectionSuccess = _client.connect(mdnsIP, _port);
            } else {
                dbg_printf("TelnetTransport: mDNS resolution failed, trying DNS\n");
                connectionSuccess = _client.connect(_host.c_str(), _port);
            }
        } else {
            connectionSuccess = _client.connect(_host.c_str(), _port);
        }
        
        if (connectionSuccess) {
            _connected = true;
            _reconnectInterval = 2000; // Reset reconnect interval on successful connection
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

IPAddress TelnetTransport::resolveMdnsHost(const char* hostname) {
    // Initialize mDNS if not already done
    if (!MDNS.begin("fluiddial")) {
        dbg_printf("TelnetTransport: mDNS initialization failed\n");
        return INADDR_NONE;
    }
    
    // Extract hostname without .local suffix for mDNS query
    String hostWithoutLocal = String(hostname);
    if (hostWithoutLocal.endsWith(".local")) {
        hostWithoutLocal = hostWithoutLocal.substring(0, hostWithoutLocal.length() - 6);
    }
    
    dbg_printf("TelnetTransport: Resolving mDNS hostname: %s\n", hostWithoutLocal.c_str());
    
    // Query mDNS for the hostname
    IPAddress serverIP = MDNS.queryHost(hostWithoutLocal);
    
    if (serverIP == INADDR_NONE) {
        dbg_printf("TelnetTransport: mDNS query failed for %s\n", hostWithoutLocal.c_str());
    } else {
        dbg_printf("TelnetTransport: mDNS resolved %s to %s\n", hostWithoutLocal.c_str(), serverIP.toString().c_str());
    }
    
    return serverIP;
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