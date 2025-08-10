// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "Scene.h"
#include "Menu.h"

#ifdef USE_WIFI_PENDANT

class NetworkSettingsScene : public Scene {
private:
    // Network configuration fields
    char _ssid[64];
    char _password[64];
    char _host[64];
    int _port;
    char _transport[16];
    
    // UI state
    int _current_field;
    bool _editing;
    bool _password_masked;
    std::string _edit_buffer;
    int _cursor_pos;
    
    // Field indices
    enum FieldIndex {
        FIELD_SSID = 0,
        FIELD_PASSWORD,
        FIELD_HOST,
        FIELD_PORT,
        FIELD_TRANSPORT,
        FIELD_COUNT
    };
    
    // Keyboard state for soft keyboard
    bool _keyboard_active;
    int _keyboard_row;
    int _keyboard_col;
    
    enum KeyboardMode {
        KEYBOARD_LOWERCASE = 0,
        KEYBOARD_UPPERCASE,
        KEYBOARD_NUMBERS
    };
    KeyboardMode _keyboard_mode;
    
    static const char* keyboard_layout_lower[4][10];
    static const char* keyboard_layout_upper[4][10];
    static const char* keyboard_layout_numbers[4][10];

public:
    NetworkSettingsScene() : Scene("Network Settings", 4), _current_field(0), _editing(false), 
                           _password_masked(true), _cursor_pos(0), _keyboard_active(false),
                           _keyboard_row(0), _keyboard_col(0), _keyboard_mode(KEYBOARD_LOWERCASE) {}

    void onEntry(void* arg) override;
    void onExit() override;
    void reDisplay() override;
    
    void onDialButtonPress() override;
    void onGreenButtonPress() override;
    void onRedButtonPress() override;
    void onTouchClick() override;
    void onEncoder(int delta) override;
    
    void onStateChange(state_t old_state) override;

private:
    void loadNetworkSettings();
    void saveNetworkSettings();
    bool testNetworkConnection();
    
    void drawField(int field_index, int y);
    void drawSoftKeyboard();
    
    void startEditing();
    void stopEditing();
    void commitEdit();
    void cancelEdit();
    
    void moveCursor(int delta);
    void insertChar(char c);
    void deleteChar();
    void moveKeyboardCursor(int row_delta, int col_delta);
    char getCurrentKeyboardChar();
    const char* const* getCurrentKeyboardLayout();
    void switchKeyboardMode(KeyboardMode mode);
    
    void showTestResult(bool success, const char* message);
};

extern NetworkSettingsScene networkSettingsScene;

#endif // USE_WIFI_PENDANT