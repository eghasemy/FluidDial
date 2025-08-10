// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#ifdef USE_WIFI_PENDANT

#include "ws_transport.h"
#include "System.h"

WSTransport::WSTransport(const char* host, int port) : _host(host), _port(port) {
}

WSTransport::~WSTransport() {
    if (_connected) {
        _webSocket.disconnect();
    }
}

void WSTransport::setHost(const char* host, int port) {
    _host = host;
    _port = port;
}

bool WSTransport::begin() {
    if (!WiFi.isConnected()) {
        dbg_printf("WSTransport: WiFi not connected\n");
        return false;
    }
    
    dbg_printf("WSTransport: Connecting to ws://%s:%d/\n", _host.c_str(), _port);
    
    // Set up WebSocket event handler
    _webSocket.onEvent([this](WStype_t type, uint8_t* payload, size_t length) {
        this->webSocketEvent(type, payload, length);
    });
    
    // Connect to WebSocket server
    _webSocket.begin(_host.c_str(), _port, "/");
    _webSocket.setReconnectInterval(_reconnectInterval);
    
    _initialized = true;
    _lastReconnectAttempt = millis();
    
    return true;
}

void WSTransport::webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            _connected = false;
            dbg_printf("WSTransport: Disconnected\n");
            break;
            
        case WStype_CONNECTED:
            _connected = true;
            _reconnectInterval = 1500; // Reset reconnect interval on successful connection
            dbg_printf("WSTransport: Connected to %s\n", payload);
            break;
            
        case WStype_TEXT:
            // Add received text to buffer
            if (payload && length > 0) {
                _receivedData += String((char*)payload, length);
            }
            break;
            
        case WStype_BIN:
            // Handle binary data (for single bytes like RT commands)
            if (payload && length > 0) {
                for (size_t i = 0; i < length; i++) {
                    _receivedData += (char)payload[i];
                }
            }
            break;
            
        case WStype_ERROR:
            dbg_printf("WSTransport: WebSocket error\n");
            _connected = false;
            break;
            
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            // Handle fragmented messages if needed
            break;
            
        default:
            break;
    }
}

void WSTransport::loop() {
    if (!_initialized) {
        return;
    }
    
    _webSocket.loop();
    
    // Handle reconnection
    if (!_connected && WiFi.isConnected()) {
        attemptReconnect();
    }
}

void WSTransport::attemptReconnect() {
    unsigned long now = millis();
    if (now - _lastReconnectAttempt >= _reconnectInterval) {
        dbg_printf("WSTransport: Attempting reconnect to %s:%d\n", _host.c_str(), _port);
        _webSocket.begin(_host.c_str(), _port, "/");
        _lastReconnectAttempt = now;
        
        // Exponential backoff, but cap at max interval
        _reconnectInterval = min(_reconnectInterval * 2, _maxReconnectInterval);
    }
}

bool WSTransport::isConnected() {
    return _connected && WiFi.isConnected();
}

void WSTransport::sendLine(const char* line, int timeout) {
    if (!isConnected() || !line) {
        return;
    }
    
    // Send line with single newline appended
    String message = String(line) + "\n";
    _webSocket.sendTXT(message);
    
    dbg_printf("WSTransport: Sent line: %s", message.c_str());
}

void WSTransport::sendRT(uint8_t c) {
    if (!isConnected()) {
        return;
    }
    
    // Send single byte as binary data
    _webSocket.sendBIN(&c, 1);
    
    dbg_printf("WSTransport: Sent RT: 0x%02X\n", c);
}

int WSTransport::getChar() {
    if (_receivedData.length() > 0) {
        char c = _receivedData.charAt(0);
        _receivedData.remove(0, 1);
        return (int)c;
    }
    return -1; // No data available
}

void WSTransport::putChar(uint8_t c) {
    if (!isConnected()) {
        return;
    }
    
    // Send single character
    _webSocket.sendBIN(&c, 1);
}

void WSTransport::resetFlowControl() {
    // Send XON character for flow control reset
    sendRT(0x11);
}

#endif // USE_WIFI_PENDANT