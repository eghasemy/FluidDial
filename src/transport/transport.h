// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include <stdint.h>

// Transport layer abstraction for FluidNC communication
// Supports both UART (current) and WiFi (future) communication methods

// Transport interface for all communication methods
class Transport {
public:
    virtual ~Transport() = default;
    virtual bool begin() = 0;
    virtual void loop() = 0;
    virtual bool isConnected() = 0;
    virtual void sendLine(const char* line, int timeout = 2000) = 0;
    virtual void sendRT(uint8_t c) = 0;
    virtual int getChar() = 0;
    virtual void putChar(uint8_t c) = 0;
    virtual void resetFlowControl() = 0;
};

// Transport factory
class TransportFactory {
public:
    static Transport* createTransport();
};

extern Transport* transport;

#ifndef USE_WIFI_PENDANT
// For UART builds, provide pass-through declarations to existing functions for compatibility
extern "C" {
    void fnc_putchar(uint8_t c);
    int fnc_getchar();
}
void resetFlowControl();
#endif // USE_WIFI_PENDANT