// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// System interface routines for the Arduino framework

#include "System.h"
#include "FluidNCModel.h"
#include "NVS.h"
#include "transport/transport.h"
#ifdef USE_WIFI_PENDANT
#include "transport/wifi_transport_factory.h"
#include "transport/transport_config.h"
#include "net/net_config.h"
#include <WiFi.h>
#endif

#include <Esp.h>  // ESP.restart()

#include <driver/uart.h>
#include "hal/uart_hal.h"

uart_port_t fnc_uart_port;

// Transport implementation
// Serial transport implementation for UART communication
class SerialTransport : public Transport {
private:
    bool _initialized = false;

public:
    bool begin() override {
        _initialized = true;
        return true;
    }
    
    void loop() override {
        // For UART, no periodic tasks needed
    }
    
    bool isConnected() override {
        return _initialized;
    }
    
    void sendLine(const char* line, int timeout = 2000) override {
        if (!line) return;
        
        const char* p = line;
        while (*p) {
            putChar(*p);
            p++;
        }
        putChar('\r');
        putChar('\n');
    }
    
    void sendRT(uint8_t c) override {
        putChar(c);
    }
    
    int getChar() override {
        return fnc_getchar();
    }
    
    void putChar(uint8_t c) override {
        fnc_putchar(c);
    }
    
    void resetFlowControl() override {
        fnc_putchar(0x11);
        uart_ll_force_xon(fnc_uart_port);
    }
};

// Global transport instance
#ifdef USE_WIFI_PENDANT
static SerialTransport serialTransport; // Fallback for WiFi builds
Transport* transport = nullptr; // Will be created dynamically for WiFi transports

// Flag to track if user has made an explicit transport choice
static bool userTransportChoiceActive = false;
static const char* userChosenTransportType = nullptr;
#else
static SerialTransport serialTransport;
Transport* transport = &serialTransport;
#endif

// Transport factory implementation
Transport* TransportFactory::createTransport() {
#ifdef USE_WIFI_PENDANT
    // For WiFi builds, transport will be created based on configuration
    TransportConfig::TransportType type = TransportConfig::getTransportType();
    const char* host = TransportConfig::getHost();
    int port = TransportConfig::getPort();
    
    return WiFiTransportFactory::createTransport(
        (type == TransportConfig::TELNET) ? WiFiTransportFactory::TELNET : WiFiTransportFactory::WEBSOCKET,
        host, port);
#else
    return &serialTransport;
#endif
}

#ifdef USE_WIFI_PENDANT
// Select transport based on WiFi availability and settings
void selectTransport() {
    // If user has made an explicit transport choice, respect it and don't auto-switch
    if (userTransportChoiceActive) {
        // If transport exists and matches user choice, try to maintain it
        if (transport) {
            if (strcmp(userChosenTransportType, "Serial") == 0 && transport == &serialTransport) {
                return; // User chose serial and we have serial - keep it
            } else if (strcmp(userChosenTransportType, "WiFi") == 0 && transport != &serialTransport) {
                // User chose WiFi and we have WiFi transport - only switch if not connected and WiFi is ready
                if (transport->isConnected() || !wifiReady()) {
                    return; // Keep current WiFi transport if connected or if WiFi not ready
                }
            }
        }
    }
    
    // If transport is already initialized and working, don't change it
    if (transport && transport->isConnected()) {
        return;
    }
    
    // Check if WiFi is ready and we should use WiFi transport (only if no user choice or user chose WiFi)
    if (wifiReady() && (!userTransportChoiceActive || strcmp(userChosenTransportType, "WiFi") == 0)) {
        // Try to create WiFi transport
        Transport* wifiTransport = TransportFactory::createTransport();
        if (wifiTransport && wifiTransport->begin()) {
            // Successfully created WiFi transport
            if (transport && transport != &serialTransport) {
                delete transport; // Clean up old transport if it's not the serial fallback
            }
            transport = wifiTransport;
            dbg_printf("Transport: Using WiFi transport\n");
            return;
        } else {
            // Failed to create WiFi transport, clean up
            if (wifiTransport) {
                delete wifiTransport;
            }
            dbg_printf("Transport: Failed to create WiFi transport, falling back to Serial\n");
        }
    }
    
    // Fall back to Serial transport (only if no user choice or user chose serial or WiFi failed)
    if (!userTransportChoiceActive || strcmp(userChosenTransportType, "Serial") == 0 || !wifiReady()) {
        if (transport != &serialTransport) {
            if (transport) {
                delete transport; // Clean up old transport
            }
            transport = &serialTransport;
            transport->begin();
            dbg_printf("Transport: Using Serial transport\n");
        }
    }
}

void forceTransportReconnect() {
    // Force disconnect current transport and recreate with updated config
    if (transport && transport != &serialTransport) {
        delete transport; // Clean up old transport
    }
    transport = nullptr; // Don't use serial fallback to avoid early return in selectTransport
    
    // Force transport selection even without WiFi ready check
    if (wifiReady()) {
        // Try to create WiFi transport with updated config
        Transport* wifiTransport = TransportFactory::createTransport();
        if (wifiTransport && wifiTransport->begin()) {
            transport = wifiTransport;
            dbg_printf("Transport: Forced WiFi transport reconnection successful\n");
            return;
        } else {
            // Failed to create WiFi transport, clean up
            if (wifiTransport) {
                delete wifiTransport;
            }
            dbg_printf("Transport: Failed to force WiFi transport reconnection\n");
        }
    }
    
    // Fallback to serial transport if WiFi transport fails
    transport = &serialTransport;
    transport->begin();
    dbg_printf("Transport: Falling back to Serial transport\n");
}

void forceTransportReconnectByType(const char* connection_type) {
    // Force disconnect current transport and recreate based on user selection
    if (transport && transport != &serialTransport) {
        delete transport; // Clean up old transport
    }
    transport = nullptr;
    
    // Set user choice flags
    userTransportChoiceActive = true;
    userChosenTransportType = connection_type; // Store pointer to string (should be static)
    
    if (strcmp(connection_type, "Serial") == 0) {
        // User selected Serial - use serial transport regardless of WiFi status
        transport = &serialTransport;
        transport->begin();
        dbg_printf("Transport: Forced to Serial transport by user selection\n");
        reset_fluidnc_connection(); // Reset FluidNC connection state for fresh start
        return;
    }
    
    // User selected WiFi - only use WiFi transport
    if (strcmp(connection_type, "WiFi") == 0) {
        if (!wifiReady()) {
            dbg_printf("Transport: WiFi selected but not ready\n");
            // Fallback to serial for now, but user will see connection error
            transport = &serialTransport;
            transport->begin();
            reset_fluidnc_connection(); // Reset FluidNC connection state for fresh start
            return;
        }
        
        // Try to create WiFi transport with updated config
        Transport* wifiTransport = TransportFactory::createTransport();
        if (wifiTransport && wifiTransport->begin()) {
            transport = wifiTransport;
            dbg_printf("Transport: Forced to WiFi transport by user selection\n");
            
            // Give WebSocket time to connect before declaring success
            // WebSocket connection is asynchronous, so we need to wait for it
            dbg_printf("Transport: Waiting for WebSocket connection to establish...\n");
            int maxWaitMs = 10000; // Wait up to 10 seconds (increased from 5)
            int waitedMs = 0;
            const int checkIntervalMs = 100;
            
            while (waitedMs < maxWaitMs && !transport->isConnected()) {
                transport->loop(); // Process WebSocket events
                delay_ms(checkIntervalMs);
                waitedMs += checkIntervalMs;
                
                // Every second, print a progress message
                if (waitedMs % 1000 == 0) {
                    dbg_printf("Transport: Still waiting for connection... %d/%d seconds\n", waitedMs/1000, maxWaitMs/1000);
                }
            }
            
            if (transport->isConnected()) {
                dbg_printf("Transport: WiFi transport connected successfully after %d ms\n", waitedMs);
            } else {
                dbg_printf("Transport: WiFi transport failed to connect within %d ms timeout\n", maxWaitMs);
                // Don't fall back to serial - let the user see the connection issue
                // The pending connection might still succeed later
            }
            
            reset_fluidnc_connection(); // Reset FluidNC connection state for fresh start
            return;
        } else {
            // Failed to create WiFi transport, clean up
            if (wifiTransport) {
                delete wifiTransport;
            }
            dbg_printf("Transport: Failed to create WiFi transport, user selected WiFi but connection failed\n");
            // Don't fallback to serial when user explicitly chose WiFi
            transport = &serialTransport;
            transport->begin();
            reset_fluidnc_connection(); // Reset FluidNC connection state for fresh start
        }
    }
}
#endif

// We use the ESP-IDF UART driver instead of the Arduino
// HardwareSerial driver so we can use software (XON/XOFF)
// flow control.  The ESP-IDF driver supports the ESP32's
// hardware implementation of XON/XOFF, but Arduino does not.

extern "C" void fnc_putchar(uint8_t c) {
    uart_write_bytes(fnc_uart_port, (const char*)&c, 1);
#ifdef ECHO_FNC_TO_DEBUG
    dbg_write(c);
#endif
}

void ledcolor(int n) {
    digitalWrite(4, !(n & 1));
    digitalWrite(16, !(n & 2));
    digitalWrite(17, !(n & 4));
}
extern "C" int fnc_getchar() {
    char c;
    int  res = uart_read_bytes(fnc_uart_port, &c, 1, 0);
    if (res == 1) {
#ifdef LED_DEBUG
        if (c == '\r' || c == '\n') {
            ledcolor(0);
        } else {
            ledcolor(c & 7);
        }
#endif
        update_rx_time();
#ifdef ECHO_FNC_TO_DEBUG
        dbg_write(c);
#endif
        return c;
    }
    return -1;
}

extern "C" void poll_extra() {
#ifdef DEBUG_TO_USB
    if (debugPort.available()) {
        char c = debugPort.read();
        if (c == 0x12) {  // CTRL-R
            ESP.restart();
            while (1) {}
        }
        fnc_putchar(c);  // So you can type commands to FluidNC
    }
#endif
}

void drawPngFile(const char* filename, int x, int y) {
    drawPngFile(&canvas, filename, x, y);
}
void drawPngFile(LGFX_Sprite* sprite, const char* filename, int x, int y) {
    // When datum is middle_center, the origin is the center of the canvas and the
    // +Y direction is down.
    std::string fn { "/" };
    fn += filename;
    sprite->drawPngFile(LittleFS, fn.c_str(), x, -y, 0, 0, 0, 0, 1.0f, 1.0f, datum_t::middle_center);
}

#define FORMAT_LITTLEFS_IF_FAILED true

// Baud rates up to 10M work
#ifndef FNC_BAUD
#    define FNC_BAUD 115200
#endif

extern void init_hardware();

void init_fnc_uart(int uart_num, int tx_pin, int rx_pin) {
    fnc_uart_port = (uart_port_t)uart_num;
    int baudrate  = FNC_BAUD;
    uart_driver_delete(fnc_uart_port);
    uart_set_pin(fnc_uart_port, (gpio_num_t)tx_pin, (gpio_num_t)rx_pin, -1, -1);
    uart_config_t conf;
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2)
    conf.source_clk = UART_SCLK_APB;  // ESP32, ESP32S2
#endif
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3)
    // UART_SCLK_XTAL is independent of the APB frequency
    conf.source_clk = UART_SCLK_XTAL;  // ESP32C3, ESP32S3
#endif
    conf.baud_rate = baudrate;

    conf.data_bits           = UART_DATA_8_BITS;
    conf.parity              = UART_PARITY_DISABLE;
    conf.stop_bits           = UART_STOP_BITS_1;
    conf.flow_ctrl           = UART_HW_FLOWCTRL_DISABLE;
    conf.rx_flow_ctrl_thresh = 0;
    if (uart_param_config(fnc_uart_port, &conf) != ESP_OK) {
        dbg_println("UART config failed");
        while (1) {}
        return;
    };
    uart_driver_install(fnc_uart_port, 256, 0, 0, NULL, ESP_INTR_FLAG_IRAM);
    uart_set_sw_flow_ctrl(fnc_uart_port, true, 64, 120);
    uint32_t baud;
    uart_get_baudrate(fnc_uart_port, &baud);
}

void init_system() {
    init_hardware();

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        dbg_println("LittleFS Mount Failed");
        return;
    }

#ifdef USE_WIFI_PENDANT
    // For WiFi pendant, select transport at boot based on WiFi availability
    selectTransport();
#else
    // Initialize transport layer for non-WiFi builds
    if (transport) {
        transport->begin();
    }
#endif

    // Make an offscreen canvas that can be copied to the screen all at once
    canvas.setColorDepth(8);
    canvas.createSprite(240, 240);  // display.width(), display.height());
}

#ifdef USE_WIFI_PENDANT
// Initialize WiFi transport once WiFi connection is established
void init_wifi_transport() {
    // Use the new selectTransport function which handles WiFi availability
    selectTransport();
}
#endif
void resetFlowControl() {
    if (transport) {
        transport->resetFlowControl();
    } else {
        fnc_putchar(0x11);
        uart_ll_force_xon(fnc_uart_port);
    }
}

extern "C" int milliseconds() {
    return millis();
}

void delay_ms(uint32_t ms) {
    delay(ms);
}

void dbg_write(uint8_t c) {
#ifdef DEBUG_TO_USB
    if (debugPort.availableForWrite() > 1) {
        debugPort.write(c);
    }
#endif
}

void dbg_print(const char* s) {
#ifdef DEBUG_TO_USB
    if (debugPort.availableForWrite() > strlen(s)) {
        debugPort.print(s);
    }
#endif
}

nvs_handle_t nvs_init(const char* name) {
    nvs_handle_t handle;
    esp_err_t    err = nvs_open(name, NVS_READWRITE, &handle);
    return err == ESP_OK ? handle : 0;
}
