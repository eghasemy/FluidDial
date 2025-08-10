#include <unity.h>

#ifdef USE_WIFI_PENDANT

#include "net/net_config.h"
#include "net/net_store.h"
#include <LittleFS.h>
#include <Arduino.h>

void setUp(void) {
    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        // Format if mount fails
        LittleFS.format();
        LittleFS.begin(true);
    }
    
    // Clear any existing network configuration
    NetStore::clear();
}

void tearDown(void) {
    // Clean up after each test
    NetStore::clear();
}

void test_wifiInit_returns_true() {
    // Test that wifiInit() initializes successfully
    bool result = wifiInit();
    TEST_ASSERT_TRUE(result);
}

void test_wifiConnectAsync_without_credentials_returns_false() {
    // Initialize WiFi manager
    wifiInit();
    
    // Test that wifiConnectAsync() returns false when no credentials are saved
    bool result = wifiConnectAsync();
    TEST_ASSERT_FALSE(result);
}

void test_wifiConnectAsync_with_credentials_returns_true() {
    // Initialize WiFi manager
    wifiInit();
    
    // Save test credentials
    NetConfig::connectWifi("test_network", "test_password");
    
    // Test that wifiConnectAsync() returns true when credentials are available
    bool result = wifiConnectAsync();
    TEST_ASSERT_TRUE(result);
}

void test_wifiReady_initially_returns_false() {
    // Initialize WiFi manager
    wifiInit();
    
    // Test that wifiReady() returns false initially (not connected)
    bool result = wifiReady();
    TEST_ASSERT_FALSE(result);
}

void test_netconfig_getWifiStatus_returns_string() {
    // Initialize WiFi manager
    wifiInit();
    
    // Test that getWifiStatus() returns a valid string
    const char* status = NetConfig::getWifiStatus();
    TEST_ASSERT_NOT_NULL(status);
    TEST_ASSERT_TRUE(strlen(status) > 0);
}

void test_netconfig_getLocalIP_returns_default() {
    // Initialize WiFi manager
    wifiInit();
    
    // Test that getLocalIP() returns default IP when not connected
    const char* ip = NetConfig::getLocalIP();
    TEST_ASSERT_EQUAL_STRING("0.0.0.0", ip);
}

void test_wifi_credentials_persistence() {
    // Initialize WiFi manager
    wifiInit();
    
    // Save credentials
    bool saveResult = NetConfig::connectWifi("persistent_network", "persistent_password");
    TEST_ASSERT_TRUE(saveResult);
    
    // Reinitialize to test persistence
    wifiInit();
    
    // Should be able to attempt connection with saved credentials
    bool connectResult = wifiConnectAsync();
    TEST_ASSERT_TRUE(connectResult);
}

void setup() {
    // Wait for serial port to connect
    delay(2000);

    UNITY_BEGIN();
    
    RUN_TEST(test_wifiInit_returns_true);
    RUN_TEST(test_wifiConnectAsync_without_credentials_returns_false);
    RUN_TEST(test_wifiConnectAsync_with_credentials_returns_true);
    RUN_TEST(test_wifiReady_initially_returns_false);
    RUN_TEST(test_netconfig_getWifiStatus_returns_string);
    RUN_TEST(test_netconfig_getLocalIP_returns_default);
    RUN_TEST(test_wifi_credentials_persistence);
    
    UNITY_END();
}

void loop() {
    // Empty
}

#else

// Stub test when WiFi pendant is not enabled
#include <Arduino.h>

void setup() {
    delay(2000);
    UNITY_BEGIN();
    // No tests to run - WiFi pendant disabled
    UNITY_END();
}

void loop() {}

#endif // USE_WIFI_PENDANT