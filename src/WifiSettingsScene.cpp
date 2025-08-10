// Copyright (c) 2023 - FluidDial Contributors
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "WifiSettingsScene.h"
#include "Scene.h"
#include "FluidNCModel.h"
#include "FileParser.h"
#include "Drawing.h"
#include "System.h"

extern Scene aboutScene;

void WifiSettingsScene::onEntry(void* arg) {
    _selectedItem = 0;
}

void WifiSettingsScene::onDialButtonPress() {
    executeSelectedItem();
}

void WifiSettingsScene::onGreenButtonPress() {
    executeSelectedItem();
}

void WifiSettingsScene::onRedButtonPress() {
    pop_scene();
}

void WifiSettingsScene::onEncoder(int delta) {
    _selectedItem += delta;
    if (_selectedItem < 0) {
        _selectedItem = _numItems - 1;
    } else if (_selectedItem >= _numItems) {
        _selectedItem = 0;
    }
    reDisplay();
}

void WifiSettingsScene::onTouchClick() {
    executeSelectedItem();
}

void WifiSettingsScene::executeSelectedItem() {
    switch (_selectedItem) {
        case 0: // Mode: STA
            send_line("$Wifi/Mode=STA");
            ackBeep();
            break;
        case 1: // Mode: AP
            send_line("$Wifi/Mode=AP");
            ackBeep();
            break;
        case 2: // Mode: Off
            send_line("$Wifi/Mode=Off");
            ackBeep();
            break;
        case 3: // STA Connect
            // This would ideally open a submenu for SSID/password entry
            // For now, just request current WiFi status
            send_line("$Wifi/ListAPs");
            ackBeep();
            break;
        case 4: // Scan Networks
            send_line("$Wifi/ListAPs");
            ackBeep();
            break;
        case 5: // Screen Layout
#ifndef USE_M5
            next_layout(1);
            ackBeep();
#endif
            break;
        case 6: // Back
            pop_scene();
            break;
    }
}

void WifiSettingsScene::drawMenu() {
    int y_start = 40;
    int y_spacing = 20;
    
    for (int i = 0; i < _numItems; i++) {
        int y = y_start + (i * y_spacing);
        color_t color = (i == _selectedItem) ? GREEN : WHITE;
        
        if (i == _selectedItem) {
            // Draw selection indicator
            text(">", 20, y, GREEN, SMALL, top_left);
        }
        
        text(_menuItems[i], 40, y, color, SMALL, top_left);
    }
}

void WifiSettingsScene::reDisplay() {
    drawMenuTitle("WiFi Settings");
    
    // Show current WiFi status if available
    if (wifi_ssid.length()) {
        std::string status = wifi_mode + " " + wifi_ssid;
        if (wifi_connected == "Connected" && wifi_ip.length()) {
            status += " (" + wifi_ip + ")";
        }
        centered_text(status.c_str(), 20, LIGHTGREY, TINY);
    }
    
    drawMenu();
    
    drawButtonLegends("Back", "Select", "");
    refreshDisplay();
}

WifiSettingsScene wifiSettingsScene;