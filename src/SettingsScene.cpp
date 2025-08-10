// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "SettingsScene.h"
#include "Menu.h"

#ifdef USE_WIFI_PENDANT
#include "NetworkSettingsScene.h"
#endif

extern Scene aboutScene;

static const char* settings_help_text[] = {
    "Settings",
    "Touch icon for option", 
    "Touch center for help",
    "Flick left to go back",
    NULL
};

// Define button radius (should match main menu)
const int settings_button_radius = 30;

// Button class for settings
class SB : public ImageButton {
public:
    SB(const char* text, callback_t callback, const char* filename) : 
        ImageButton(text, callback, filename, settings_button_radius, WHITE) {}
    SB(const char* text, Scene* scene, const char* filename) : 
        ImageButton(text, scene, filename, settings_button_radius, WHITE) {}
};

// Settings menu buttons
SB aboutButton("About", &aboutScene, "abouttp.png");

#ifdef USE_WIFI_PENDANT
SB networkButton("Network", &networkSettingsScene, "abouttp.png"); // Using abouttp.png temporarily
#endif

SettingsScene::SettingsScene() : PieMenu("Settings", settings_button_radius, settings_help_text) {
    addItem(&aboutButton);
    
#ifdef USE_WIFI_PENDANT
    addItem(&networkButton);
#endif
}

SettingsScene settingsScene;