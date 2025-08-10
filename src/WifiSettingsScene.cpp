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
    
    // Request current WiFi information when entering the scene
    if (state != Disconnected) {
        send_line("$I");  // Request settings including WiFi info
    }
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
            send_line("$ESP/WiFi/Mode=STA");
            ackBeep();
            break;
        case 1: // Mode: AP
            send_line("$ESP/WiFi/Mode=AP");
            ackBeep();
            break;
        case 2: // Mode: Off
            send_line("$ESP/WiFi/Mode=Off");
            ackBeep();
            break;
        case 3: // STA Connect
            // Request current WiFi settings to see what's configured
            send_line("$ESP");
            ackBeep();
            break;
        case 4: // Scan Networks
            send_line("$ESP/WiFi/ListAPs");
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
        
        // Show current WiFi mode with indicator
        std::string item_text = _menuItems[i];
        if (i < 3 && wifi_mode.length()) { // Mode items
            if ((i == 0 && wifi_mode == "STA") ||
                (i == 1 && wifi_mode == "AP") ||
                (i == 2 && wifi_mode == "No Wifi")) {
                item_text += " *";  // Mark current mode
            }
        }
        
        text(item_text.c_str(), 40, y, color, SMALL, top_left);
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
    
    // Show help text for selected item
    const char* help_text = "";
    switch (_selectedItem) {
        case 0: help_text = "Connect to existing WiFi"; break;
        case 1: help_text = "Create WiFi access point"; break;
        case 2: help_text = "Disable WiFi completely"; break;
        case 3: help_text = "Show current WiFi config"; break;
        case 4: help_text = "Scan for nearby networks"; break;
        case 5: help_text = "Change screen rotation"; break;
        case 6: help_text = "Return to About scene"; break;
    }
    
    if (strlen(help_text) > 0) {
        centered_text(help_text, 200, DARKGREY, TINY);
    }
    
    drawButtonLegends("Back", "Select", "");
    refreshDisplay();
}

WifiSettingsScene wifiSettingsScene;