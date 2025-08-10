// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

// Transport layer abstraction for FluidNC communication
// Supports both UART (current) and WiFi (future) communication methods

#ifdef USE_WIFI_PENDANT

// Future WiFi transport interface
class Transport {
public:
    virtual ~Transport() = default;
    virtual bool init() = 0;
    virtual bool isConnected() = 0;
    virtual int getChar() = 0;
    virtual void putChar(uint8_t c) = 0;
    virtual void resetFlowControl() = 0;
};

extern Transport* transport;

#else

// For UART builds, provide pass-through declarations to existing functions
extern "C" {
    void fnc_putchar(uint8_t c);
    int fnc_getchar();
}
void resetFlowControl();

#endif // USE_WIFI_PENDANT