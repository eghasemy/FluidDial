# WiFi Connection Manager Implementation

## Overview

The WiFi Connection Manager provides non-blocking WiFi connectivity for FluidDial pendant devices. It implements the requirements specified in Phase 4 with automatic reconnection, UI responsiveness, and persistent configuration storage.

## Core Functions

### `wifiInit()`
Initializes the WiFi connection manager:
- Mounts the LittleFS filesystem
- Loads stored WiFi credentials
- Sets up WiFi in station mode
- **Usage**: Call once during device startup

### `wifiConnectAsync()`  
Attempts WiFi connection non-blocking:
- Starts connection attempt if credentials available
- Respects exponential backoff timing (5s to 60s)
- Returns immediately without blocking UI
- **Usage**: Called automatically or manually to trigger connection

### `wifiReady()`
Manages WiFi connection state:
- Tracks connection status changes
- Handles automatic reconnection on loss
- Calls `WiFi.setSleep(false)` on successful connection
- **Usage**: Call in main loop to monitor/maintain connection

## Usage Example

```cpp
#include "net/net_config.h"

void setup() {
    // Initialize WiFi system
    if (wifiInit()) {
        // Set WiFi credentials (saved persistently)
        NetConfig::connectWifi("YourNetworkName", "YourPassword");
        
        // Start connection attempt
        wifiConnectAsync();
    }
}

void loop() {
    // Monitor and maintain WiFi connection
    bool connected = wifiReady();
    
    if (connected) {
        // WiFi is connected - can communicate with FluidNC
        dbg_printf("WiFi connected: %s\n", NetConfig::getLocalIP());
    } else {
        // WiFi not connected - automatic reconnection in progress
        dbg_printf("WiFi status: %s\n", NetConfig::getWifiStatus());
    }
    
    // Your other loop code here...
}
```

## Integration with UI

The WiFi manager automatically integrates with the existing UI locking system:

- When WiFi is not connected, `ui_locked()` returns `true`
- This disables pendant controls until connection is established
- UI remains responsive during connection attempts
- No manual intervention required

## Connection Behavior

### Initial Connection
1. `wifiInit()` loads saved credentials
2. `wifiConnectAsync()` starts connection attempt  
3. `wifiReady()` monitors progress and enables sleep disable

### Reconnection After Loss
1. `wifiReady()` detects `WL_CONNECTION_LOST`
2. Automatically triggers reconnection attempt
3. Uses exponential backoff (5s, 10s, 20s, 40s, 60s max)
4. No device reboot required

### Status Monitoring
```cpp
// Check connection status
if (NetConfig::isWifiConnected()) {
    printf("Connected to: %s\n", NetConfig::getLocalIP());
} else {
    printf("Status: %s\n", NetConfig::getWifiStatus());
}
```

## Build Configuration

Enable WiFi pendant functionality by building with:
```bash
# M5Dial with WiFi
pio run -e m5dial_wifi_test

# CYD with WiFi  
pio run -e cyd_wifi
```

Standard builds without WiFi pendant features:
```bash
# Regular M5Dial (UART only)
pio run -e m5dial

# Regular CYD (UART only)
pio run -e cyddial
```

## Persistent Storage

WiFi credentials are automatically saved to LittleFS in `/net.json`:
```json
{
  "ssid": "NetworkName",
  "pass": "Password", 
  "host": "fluidnc.local",
  "port": 81,
  "transport": "ws"
}
```

## Testing

Run the test suite to verify functionality:
```bash
pio test -e m5dial_wifi_test
```

Tests cover:
- WiFi initialization
- Credential storage/loading
- Connection state management
- Status reporting
- UI integration

## Troubleshooting

Check debug output for connection issues:
```
WiFi: Initialization complete
WiFi: Loaded credentials for SSID: MyNetwork  
WiFi: Attempting connection to MyNetwork
WiFi: Connected to MyNetwork, IP: 192.168.1.100
```

Common status messages:
- `"Connecting..."` - Connection attempt in progress
- `"Connected"` - Successfully connected
- `"SSID Not Found"` - Network not available
- `"Connection Failed"` - Invalid credentials or other error
- `"Connection Lost"` - Lost connection, will auto-reconnect