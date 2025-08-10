// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "transport.h"

#ifdef USE_WIFI_PENDANT

#include "System.h"

// Serial transport implementation for UART communication
// This provides a transport layer wrapper around the existing UART functionality
class SerialTransport : public Transport {
private:
    bool _initialized = false;

public:
    bool init() override {
        // For now, rely on existing UART initialization in System.cpp
        // Future implementation could move UART init logic here
        _initialized = true;
        return true;
    }
    
    bool isConnected() override {
        // For UART, we assume connection is always available
        // Future WiFi implementation will check actual connection status
        return _initialized;
    }
    
    int getChar() override {
        // Delegate to existing UART implementation
        extern "C" int fnc_getchar();
        return fnc_getchar();
    }
    
    void putChar(uint8_t c) override {
        // Delegate to existing UART implementation  
        extern "C" void fnc_putchar(uint8_t c);
        fnc_putchar(c);
    }
    
    void resetFlowControl() override {
        // Delegate to existing UART implementation
        extern void resetFlowControl();
        resetFlowControl();
    }
};

// Global transport instance
SerialTransport serialTransport;
Transport* transport = &serialTransport;

#endif // USE_WIFI_PENDANT