// Copyright (c) 2023 -	Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "System.h"
#include "FileParser.h"
#include "Scene.h"
#include "AboutScene.h"
#include "transport/transport.h"
#ifdef USE_WIFI_PENDANT
#include "net/net_config.h"
#endif

extern void base_display();
extern void show_logo();

extern const char* git_info;

extern AboutScene aboutScene;

void setup() {
    init_system();

    display.setBrightness(aboutScene.getBrightness());

    show_logo();
    delay_ms(2000);  // view the logo and wait for the debug port to connect

    base_display();

    dbg_printf("FluidNC Pendant %s\n", git_info);

#ifdef USE_WIFI_PENDANT
    // Initialize WiFi connection manager
    if (wifiInit()) {
        dbg_printf("WiFi: Connection manager initialized\n");
        // Start async connection if credentials are available
        wifiConnectAsync();
    } else {
        dbg_printf("WiFi: Failed to initialize connection manager\n");
    }
#endif

    if (transport) {
        transport->sendRT(StatusReport);  // Kick FluidNC into action using transport
    } else {
        fnc_realtime(StatusReport);       // Fallback to direct call
    }

    // init_file_list();

    extern Scene* initMenus();
    activate_scene(initMenus());
}

void loop() {
#ifdef USE_WIFI_PENDANT
    // Update WiFi connection status and handle reconnects
    wifiReady();
    
    // Initialize WiFi transport if not already done
    init_wifi_transport();
#endif
    
    if (transport) {
        transport->loop();  // Handle transport-specific tasks
    }
    fnc_poll();         // Handle messages from FluidNC
    dispatch_events();  // Handle dial, touch, buttons
}
