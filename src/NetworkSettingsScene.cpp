// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "NetworkSettingsScene.h"

#ifdef USE_WIFI_PENDANT

#    include "System.h"
#    include "net/net_store.h"
#    include "net/net_config.h"
#    include "transport/transport_config.h"
#    include "transport/transport.h"
#    include "Text.h"
#    include "Drawing.h"

// Soft keyboard layouts
const char* NetworkSettingsScene::keyboard_layout_lower[4][10] = { 
    { "q", "w", "e", "r", "t", "y", "u", "i", "o", "p" },
    { "a", "s", "d", "f", "g", "h", "j", "k", "l", "ENT" },
    { "z", "x", "c", "v", "b", "n", "m", ".", "DEL", "" },
    { "123", " ", "SHIFT", "←", "→", "SAVE", "TEST", "EXIT", "", "" } 
};

const char* NetworkSettingsScene::keyboard_layout_upper[4][10] = { 
    { "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P" },
    { "A", "S", "D", "F", "G", "H", "J", "K", "L", "ENT" },
    { "Z", "X", "C", "V", "B", "N", "M", ".", "DEL", "" },
    { "123", " ", "shift", "←", "→", "SAVE", "TEST", "EXIT", "", "" } 
};

const char* NetworkSettingsScene::keyboard_layout_numbers[4][10] = { 
    { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" },
    { "@", "#", "$", "%", "&", "*", "(", ")", "-", "ENT" },
    { "!", "?", ":", ";", "_", "+", "=", "/", "DEL", "" },
    { "ABC", " ", "\\", "←", "→", "SAVE", "TEST", "EXIT", "", "" } 
};

static const char* field_names[] = { "SSID:", "Password:", "Host/IP:", "Port:", "Transport:" };

static const char* transport_options[] = { "ws", "tcp" };

void NetworkSettingsScene::onEntry(void* arg) {
    loadNetworkSettings();
    _current_field   = 0;
    _editing         = false;
    _keyboard_active = false;
    _keyboard_mode   = KEYBOARD_LOWERCASE;
    _cursor_pos      = 0;
    reDisplay();
}

void NetworkSettingsScene::onExit() {
    _editing         = false;
    _keyboard_active = false;
}

void NetworkSettingsScene::loadNetworkSettings() {
    // Load current network settings from storage
    NetStore::netLoad(_ssid, sizeof(_ssid), _password, sizeof(_password), _host, sizeof(_host), _port, _transport, sizeof(_transport));
}

void NetworkSettingsScene::saveNetworkSettings() {
    // Save current settings to /net.json
    bool success = NetStore::netSave(_ssid, _password, _host, _port, _transport);

    if (success) {
        // Also update transport configuration so the transport layer gets the new settings
        TransportConfig::setHost(_host);
        TransportConfig::setPort(_port);
        TransportConfig::setTransportType(
            (strcmp(_transport, "tcp") == 0) ? TransportConfig::TELNET : TransportConfig::WEBSOCKET
        );
        // Save transport config to persistent storage
        TransportConfig::saveConfig();
        
        // Invalidate transport config cache so next load reads updated values
        TransportConfig::invalidateCache();
        
        showTestResult(true, "Settings saved!");
        // Reload settings to ensure UI reflects what was actually saved
        loadNetworkSettings();
        reDisplay();
        
        // Force transport reconnection with new settings
        if (wifiReady()) {
            // Disconnect WiFi and force transport to reconnect
            NetConfig::disconnectWifi();
            delay_ms(1000);  // Brief delay before reconnect attempt
            wifiConnectAsync();
            // Force transport re-selection after WiFi reconnects
            delay_ms(2000);  // Give WiFi time to reconnect
            forceTransportReconnect(); // Force recreation with updated config
        }
    } else {
        showTestResult(false, "Save failed!");
    }
}

bool NetworkSettingsScene::testNetworkConnection() {
    // Test the current network settings without saving
    showTestResult(false, "Testing...");
    refreshDisplay();
    delay_ms(500);

    // Try to connect with current settings
    bool wifi_success = NetConfig::connectWifi(_ssid, _password);
    if (!wifi_success) {
        showTestResult(false, "WiFi failed");
        return false;
    }

    // Wait for WiFi to establish connection
    delay_ms(2000);
    
    // Test actual FluidNC connection 
    bool host_success = NetConfig::testFluidNCConnection(_host, _port);
    if (!host_success) {
        showTestResult(false, "FluidNC failed");
        return false;
    }

    showTestResult(true, "Connection OK!");
    return true;
}

void NetworkSettingsScene::onDialButtonPress() {
    if (_keyboard_active) {
        // Select keyboard key
        char        c   = getCurrentKeyboardChar();
        const char* const* layout = getCurrentKeyboardLayout();
        const char* key = layout[_keyboard_row * 10 + _keyboard_col];
        
        if (strcmp(key, "ENT") == 0) {
            commitEdit();
        } else if (strcmp(key, "DEL") == 0) {
            deleteChar();
        } else if (strcmp(key, "SAVE") == 0) {
            commitEdit();  // First commit the current edit to the field
            saveNetworkSettings();
        } else if (strcmp(key, "TEST") == 0) {
            testNetworkConnection();
        } else if (strcmp(key, "EXIT") == 0) {
            cancelEdit();
        } else if (strcmp(key, "123") == 0) {
            switchKeyboardMode(KEYBOARD_NUMBERS);
        } else if (strcmp(key, "ABC") == 0) {
            switchKeyboardMode(KEYBOARD_LOWERCASE);
        } else if (strcmp(key, "SHIFT") == 0) {
            switchKeyboardMode(KEYBOARD_UPPERCASE);
        } else if (strcmp(key, "shift") == 0) {
            switchKeyboardMode(KEYBOARD_LOWERCASE);
        } else if (c != 0) {
            insertChar(c);
        }
    } else if (_editing) {
        commitEdit();
    } else {
        // Go back to main menu
        pop_scene();
    }
}

void NetworkSettingsScene::onGreenButtonPress() {
    if (_keyboard_active) {
        commitEdit();  // Commit current edit to field
        stopEditing();
    } else if (_editing) {
        // Commit current edit
        commitEdit();
    } else {
        // Save settings
        saveNetworkSettings();
    }
}

void NetworkSettingsScene::onRedButtonPress() {
    if (_keyboard_active) {
        cancelEdit();
    } else if (_editing) {
        cancelEdit();
    } else {
        // Test connection
        testNetworkConnection();
    }
}

void NetworkSettingsScene::onTouchClick() {
    if (!_editing) {
        startEditing();
    }
}

void NetworkSettingsScene::onEncoder(int delta) {
    // Ignore very small deltas to prevent micro-movements
    if (abs(delta) < 1) {
        return;
    }
    
    if (_keyboard_active) {
        // Navigate through keyboard keys in linear fashion (left-to-right, top-to-bottom)
        // Absolute normalization to prevent jumping multiple keys
        int step = (delta > 0) ? 1 : -1;
        int current_pos = _keyboard_row * 10 + _keyboard_col;
        current_pos += step;

        // Find next valid key position
        const char* const* layout = getCurrentKeyboardLayout();
        int total_keys = 40;  // 4 rows * 10 cols
        for (int i = 0; i < total_keys; i++) {
            if (current_pos >= total_keys)
                current_pos = 0;
            if (current_pos < 0)
                current_pos = total_keys - 1;

            int new_row = current_pos / 10;
            int new_col = current_pos % 10;

            // Check if this position has a valid key
            if (strlen(layout[new_row * 10 + new_col]) > 0) {
                _keyboard_row = new_row;
                _keyboard_col = new_col;
                reDisplay();
                return;
            }

            // Move to next position and continue searching
            current_pos += step;
        }
    } else if (_editing) {
        // Special handling for transport field - cycle through options
        if (_current_field == FIELD_TRANSPORT) {
            // Absolute normalization for transport switching
            if (delta > 0) {
                _edit_buffer = (_edit_buffer == "ws") ? "tcp" : "ws";
            } else {
                _edit_buffer = (_edit_buffer == "tcp") ? "ws" : "tcp";
            }
            reDisplay();
        } else {
            moveCursor(delta);
        }
    } else {
        // Move between fields - absolute normalization to prevent jumping multiple fields
        int step = (delta > 0) ? 1 : -1;
        _current_field += step;
        if (_current_field >= FIELD_COUNT)
            _current_field = 0;
        if (_current_field < 0)
            _current_field = FIELD_COUNT - 1;
        reDisplay();
    }
}

void NetworkSettingsScene::startEditing() {
    _editing         = true;
    _keyboard_active = true;

    // Set edit buffer to current field value
    switch (_current_field) {
        case FIELD_SSID:
            _edit_buffer = _ssid;
            break;
        case FIELD_PASSWORD:
            _edit_buffer = _password;
            break;
        case FIELD_HOST:
            _edit_buffer = _host;
            break;
        case FIELD_PORT:
            _edit_buffer = std::to_string(_port);
            break;
        case FIELD_TRANSPORT:
            _edit_buffer = _transport;
            break;
    }

    _cursor_pos = _edit_buffer.length();
    reDisplay();
}

void NetworkSettingsScene::stopEditing() {
    _editing         = false;
    _keyboard_active = false;
    reDisplay();
}

void NetworkSettingsScene::commitEdit() {
    // Apply edit buffer to current field
    switch (_current_field) {
        case FIELD_SSID:
            // Allow clearing SSID field (for new network setup)
            strlcpy(_ssid, _edit_buffer.c_str(), sizeof(_ssid));
            break;
        case FIELD_PASSWORD:
            // Allow empty passwords for open networks
            strlcpy(_password, _edit_buffer.c_str(), sizeof(_password));
            break;
        case FIELD_HOST:
            // Allow clearing host field (user can enter new host)
            strlcpy(_host, _edit_buffer.c_str(), sizeof(_host));
            break;
        case FIELD_PORT: {
            int port = atoi(_edit_buffer.c_str());
            if (port > 0 && port <= 65535) {
                _port = port;
            } else {
                _port = 81;  // Default port if invalid
                showTestResult(false, "Invalid port, using 81");
            }
        } break;
        case FIELD_TRANSPORT:
            // Validate transport type
            if (_edit_buffer == "ws" || _edit_buffer == "tcp") {
                strlcpy(_transport, _edit_buffer.c_str(), sizeof(_transport));
            } else {
                strlcpy(_transport, "ws", sizeof(_transport));  // Default to websocket
                showTestResult(false, "Invalid transport, using ws");
            }
            break;
    }

    stopEditing();
}

void NetworkSettingsScene::cancelEdit() {
    _edit_buffer.clear();
    stopEditing();
}

void NetworkSettingsScene::moveCursor(int delta) {
    // Normalize delta to prevent jumping multiple characters
    int step = (delta > 0) ? 1 : -1;
    _cursor_pos += step;
    if (_cursor_pos < 0)
        _cursor_pos = 0;
    if (_cursor_pos > (int)_edit_buffer.length())
        _cursor_pos = _edit_buffer.length();
    reDisplay();
}

void NetworkSettingsScene::insertChar(char c) {
    if (_edit_buffer.length() < 63) {  // Leave room for null terminator
        _edit_buffer.insert(_cursor_pos, 1, c);
        _cursor_pos++;
        reDisplay();
    }
}

void NetworkSettingsScene::deleteChar() {
    if (_cursor_pos > 0 && !_edit_buffer.empty()) {
        _edit_buffer.erase(_cursor_pos - 1, 1);
        _cursor_pos--;
        reDisplay();
    }
}

void NetworkSettingsScene::moveKeyboardCursor(int row_delta, int col_delta) {
    _keyboard_row += row_delta;
    _keyboard_col += col_delta;

    // Wrap around
    if (_keyboard_row < 0)
        _keyboard_row = 3;
    if (_keyboard_row > 3)
        _keyboard_row = 0;
    if (_keyboard_col < 0)
        _keyboard_col = 9;
    if (_keyboard_col > 9)
        _keyboard_col = 0;

    // Skip empty cells
    const char* const* layout = getCurrentKeyboardLayout();
    if (strlen(layout[_keyboard_row * 10 + _keyboard_col]) == 0) {
        moveKeyboardCursor(row_delta, col_delta);
    }

    reDisplay();
}

char NetworkSettingsScene::getCurrentKeyboardChar() {
    const char* const* layout = getCurrentKeyboardLayout();
    const char* key = layout[_keyboard_row * 10 + _keyboard_col];
    if (strlen(key) == 1) {
        return key[0];
    } else if (strcmp(key, "ENT") == 0) {
        return '\n';
    } else if (strcmp(key, "DEL") == 0) {
        return '\b';
    } else if (strcmp(key, " ") == 0) {
        return ' ';
    }
    return 0;  // Special keys
}

const char* const* NetworkSettingsScene::getCurrentKeyboardLayout() {
    switch (_keyboard_mode) {
        case KEYBOARD_UPPERCASE:
            return (const char* const*)keyboard_layout_upper;
        case KEYBOARD_NUMBERS:
            return (const char* const*)keyboard_layout_numbers;
        case KEYBOARD_LOWERCASE:
        default:
            return (const char* const*)keyboard_layout_lower;
    }
}

void NetworkSettingsScene::switchKeyboardMode(KeyboardMode mode) {
    _keyboard_mode = mode;
    // Reset keyboard position to a safe location
    _keyboard_row = 0;
    _keyboard_col = 0;
    reDisplay();
}

void NetworkSettingsScene::drawField(int field_index, int y) {
    bool is_current = (field_index == _current_field);
    bool is_editing = is_current && _editing;

    // Field name
    text(field_names[field_index], 10, y, is_current ? GREEN : WHITE, TINY, middle_left);

    // Field value
    std::string value;
    bool        is_password = (field_index == FIELD_PASSWORD);

    if (is_editing) {
        value = _edit_buffer;
        if (is_password && _password_masked) {
            value = std::string(_edit_buffer.length(), '*');
        }
        // Add cursor
        if (_cursor_pos <= (int)value.length()) {
            value.insert(_cursor_pos, "|");
        }
    } else {
        switch (field_index) {
            case FIELD_SSID:
                value = _ssid;
                break;
            case FIELD_PASSWORD:
                value = is_password && _password_masked ? std::string(strlen(_password), '*') : _password;
                break;
            case FIELD_HOST:
                value = _host;
                break;
            case FIELD_PORT:
                value = std::to_string(_port);
                break;
            case FIELD_TRANSPORT:
                value = _transport;
                break;
        }
    }

    // Draw field value with background
    int bg_color   = is_editing ? BLUE : (is_current ? DARKGREY : BLACK);
    int text_color = is_editing ? WHITE : (is_current ? YELLOW : LIGHTGREY);

    canvas.fillRoundRect(75, y - 8, 155, 16, 2, bg_color);
    text(value.c_str(), 80, y, text_color, TINY, middle_left);
}

void NetworkSettingsScene::drawSoftKeyboard() {
    if (!_keyboard_active)
        return;

    const int kb_start_y  = 140;
    const int key_width   = 22;
    const int key_height  = 18;
    const int key_spacing = 2;

    const char* const* layout = getCurrentKeyboardLayout();

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 10; col++) {
            const char* key = layout[row * 10 + col];
            if (strlen(key) == 0)
                continue;

            int x = 5 + col * (key_width + key_spacing);
            int y = kb_start_y + row * (key_height + key_spacing);

            bool is_selected = (row == _keyboard_row && col == _keyboard_col);
            int  bg_color    = is_selected ? GREEN : DARKGREY;
            int  text_color  = is_selected ? BLACK : WHITE;

            canvas.fillRoundRect(x, y, key_width, key_height, 3, bg_color);
            canvas.drawRoundRect(x, y, key_width, key_height, 3, WHITE);

            // Handle special keys
            const char* display_text = key;
            if (strcmp(key, " ") == 0) {
                display_text = "SPC";
            } else if (strcmp(key, "SAVE") == 0) {
                bg_color   = GREEN;
                text_color = BLACK;
                canvas.fillRoundRect(x, y, key_width, key_height, 3, bg_color);
            } else if (strcmp(key, "TEST") == 0) {
                bg_color   = ORANGE;
                text_color = BLACK;
                canvas.fillRoundRect(x, y, key_width, key_height, 3, bg_color);
            } else if (strcmp(key, "EXIT") == 0) {
                bg_color   = RED;
                text_color = WHITE;
                canvas.fillRoundRect(x, y, key_width, key_height, 3, bg_color);
            } else if (strcmp(key, "123") == 0 || strcmp(key, "ABC") == 0) {
                bg_color   = BLUE;
                text_color = WHITE;
                canvas.fillRoundRect(x, y, key_width, key_height, 3, bg_color);
            } else if (strcmp(key, "SHIFT") == 0 || strcmp(key, "shift") == 0) {
                bg_color   = MAROON;
                text_color = WHITE;
                canvas.fillRoundRect(x, y, key_width, key_height, 3, bg_color);
            }

            text(display_text, x + key_width / 2, y + key_height / 2, text_color, TINY, middle_center);
        }
    }
}

void NetworkSettingsScene::showTestResult(bool success, const char* message) {
    // Show result in status area temporarily
    int color = success ? GREEN : RED;
    canvas.fillRoundRect(10, 110, 220, 20, 5, BLACK);
    centered_text(message, 120, color, TINY);
    refreshDisplay();
}

void NetworkSettingsScene::reDisplay() {
    background();
    drawStatus();

    // Title
    centered_text("Network Settings", 30, WHITE, TINY);

    // Draw input fields
    int field_y = 60;
    for (int i = 0; i < FIELD_COUNT; i++) {
        drawField(i, field_y);
        field_y += 20;
    }

    // Draw soft keyboard if active
    drawSoftKeyboard();

    // Draw button legends
    if (_keyboard_active) {
        drawButtonLegends("Cancel", "Done", "Select");
    } else if (_editing) {
        drawButtonLegends("Cancel", "Save", "Edit");
    } else {
        drawButtonLegends("Test", "Save", "Back");
    }

    refreshDisplay();
}

void NetworkSettingsScene::onStateChange(state_t old_state) {
    reDisplay();
}

NetworkSettingsScene networkSettingsScene;

#endif  // USE_WIFI_PENDANT