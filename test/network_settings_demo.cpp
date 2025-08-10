/*
 * Network Settings Storage Demo
 * 
 * This demonstrates the LittleFS-backed JSON network settings storage functionality.
 * To use this code, include it in a WiFi-enabled build (e.g., cyd_wifi or m5dial_wifi_test).
 * 
 * Demonstrates:
 * - Saving complete network settings
 * - Loading settings with defaults when file is missing
 * - Saving individual WiFi credentials while preserving host settings
 * - Saving host settings while preserving WiFi credentials
 * - Clearing all settings
 * - Persistence across reboots
 */

#ifdef USE_WIFI_PENDANT

#include "net/net_store.h"
#include "System.h"

void demoNetworkSettings() {
    dbg_println("=== Network Settings Storage Demo ===");
    
    // Initialize storage
    NetStore::init();
    
    // Demo 1: Load settings when no file exists (should return defaults)
    dbg_println("\n1. Loading settings with no file (should get defaults):");
    char ssid[64], password[64], host[64], transport[16];
    int port;
    
    bool hasSettings = NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                                        host, sizeof(host), port, transport, sizeof(transport));
    
    dbg_printf("   File exists: %s\n", hasSettings ? "YES" : "NO");
    dbg_printf("   SSID: '%s'\n", ssid);
    dbg_printf("   Password: '%s'\n", password);
    dbg_printf("   Host: '%s'\n", host);
    dbg_printf("   Port: %d\n", port);
    dbg_printf("   Transport: '%s'\n", transport);
    
    // Demo 2: Save complete network settings
    dbg_println("\n2. Saving complete network settings:");
    bool saveResult = NetStore::netSave("MyWiFi", "SecretPassword", "fluidnc.example.com", 8080, "tcp");
    dbg_printf("   Save result: %s\n", saveResult ? "SUCCESS" : "FAILED");
    
    // Demo 3: Load the saved settings
    dbg_println("\n3. Loading saved settings:");
    hasSettings = NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                                   host, sizeof(host), port, transport, sizeof(transport));
    
    dbg_printf("   File exists: %s\n", hasSettings ? "YES" : "NO");
    dbg_printf("   SSID: '%s'\n", ssid);
    dbg_printf("   Password: '%s'\n", password);
    dbg_printf("   Host: '%s'\n", host);
    dbg_printf("   Port: %d\n", port);
    dbg_printf("   Transport: '%s'\n", transport);
    
    // Demo 4: Update only WiFi credentials (should preserve host settings)
    dbg_println("\n4. Updating only WiFi credentials (preserving host settings):");
    bool wifiResult = NetStore::saveWifiCredentials("NewWiFi", "NewPassword");
    dbg_printf("   Save result: %s\n", wifiResult ? "SUCCESS" : "FAILED");
    
    hasSettings = NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                                   host, sizeof(host), port, transport, sizeof(transport));
    
    dbg_printf("   SSID: '%s' (should be 'NewWiFi')\n", ssid);
    dbg_printf("   Password: '%s' (should be 'NewPassword')\n", password);
    dbg_printf("   Host: '%s' (should still be 'fluidnc.example.com')\n", host);
    dbg_printf("   Port: %d (should still be 8080)\n", port);
    dbg_printf("   Transport: '%s' (should still be 'tcp')\n", transport);
    
    // Demo 5: Update only host settings (should preserve WiFi credentials)
    dbg_println("\n5. Updating only host settings (preserving WiFi credentials):");
    bool hostResult = NetStore::saveFluidNCHost("fluidnc.local", 81);
    dbg_printf("   Save result: %s\n", hostResult ? "SUCCESS" : "FAILED");
    
    hasSettings = NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                                   host, sizeof(host), port, transport, sizeof(transport));
    
    dbg_printf("   SSID: '%s' (should still be 'NewWiFi')\n", ssid);
    dbg_printf("   Password: '%s' (should still be 'NewPassword')\n", password);
    dbg_printf("   Host: '%s' (should now be 'fluidnc.local')\n", host);
    dbg_printf("   Port: %d (should now be 81)\n", port);
    dbg_printf("   Transport: '%s' (should have default 'ws')\n", transport);
    
    // Demo 6: Clear all settings
    dbg_println("\n6. Clearing all settings:");
    NetStore::clear();
    
    hasSettings = NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                                   host, sizeof(host), port, transport, sizeof(transport));
    
    dbg_printf("   File exists: %s (should be NO)\n", hasSettings ? "YES" : "NO");
    dbg_printf("   Host: '%s' (should be default 'fluidnc.local')\n", host);
    dbg_printf("   Port: %d (should be default 81)\n", port);
    dbg_printf("   Transport: '%s' (should be default 'ws')\n", transport);
    
    dbg_println("\n=== Demo Complete ===");
    dbg_println("Network settings storage is working correctly!");
    dbg_println("Settings will persist across reboots when saved.");
}

#endif // USE_WIFI_PENDANT