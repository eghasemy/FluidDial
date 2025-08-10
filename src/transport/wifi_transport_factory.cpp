// Copyright (c) 2023 Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#ifdef USE_WIFI_PENDANT

#include "wifi_transport_factory.h"
#include "ws_transport.h"
#include "telnet_transport.h"

Transport* WiFiTransportFactory::createTransport(TransportType type, const char* host, int port) {
    switch (type) {
        case WEBSOCKET:
            return createWSTransport(host, port);
        case TELNET:
            return createTelnetTransport(host, port);
        default:
            return nullptr;
    }
}

Transport* WiFiTransportFactory::createWSTransport(const char* host, int port) {
    return new WSTransport(host, port);
}

Transport* WiFiTransportFactory::createTelnetTransport(const char* host, int port) {
    return new TelnetTransport(host, port);
}

#endif // USE_WIFI_PENDANT