// Copyright (c) 2023 - FluidDial Contributors
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "Scene.h"

class WifiSettingsScene : public Scene {
private:
    int _selectedItem;
    int _numItems;
    static const int MAX_ITEMS = 7;
    
    const char* _menuItems[MAX_ITEMS] = {
        "Mode: STA",
        "Mode: AP", 
        "Mode: Off",
        "STA Connect",
        "Scan Networks",
        "Screen Layout",
        "Back"
    };
    
public:
    WifiSettingsScene() : Scene("WiFi Settings", 1), _selectedItem(0), _numItems(MAX_ITEMS) {}

    void onEntry(void* arg) override;
    void onDialButtonPress() override;
    void onGreenButtonPress() override;
    void onRedButtonPress() override;
    void onEncoder(int delta) override;
    void onTouchClick() override;
    void reDisplay() override;
    
private:
    void executeSelectedItem();
    void drawMenu();
};

extern WifiSettingsScene wifiSettingsScene;