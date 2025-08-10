#include <unity.h>

#ifdef USE_WIFI_PENDANT

#include "net/net_store.h"
#include <LittleFS.h>

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

void test_netSave_netLoad_complete_settings() {
    // Test saving complete network settings
    bool saveResult = NetStore::netSave("test_ssid", "test_password", "test.host", 8080, "tcp");
    TEST_ASSERT_TRUE(saveResult);
    
    // Test loading the settings back
    char ssid[64], password[64], host[64], transport[16];
    int port;
    
    bool loadResult = NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                                       host, sizeof(host), port, transport, sizeof(transport));
    TEST_ASSERT_TRUE(loadResult);
    
    // Verify all values
    TEST_ASSERT_EQUAL_STRING("test_ssid", ssid);
    TEST_ASSERT_EQUAL_STRING("test_password", password);
    TEST_ASSERT_EQUAL_STRING("test.host", host);
    TEST_ASSERT_EQUAL_INT(8080, port);
    TEST_ASSERT_EQUAL_STRING("tcp", transport);
}

void test_netLoad_missing_file_defaults() {
    // Test loading when no file exists - should return defaults
    char ssid[64], password[64], host[64], transport[16];
    int port;
    
    bool loadResult = NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                                       host, sizeof(host), port, transport, sizeof(transport));
    TEST_ASSERT_FALSE(loadResult); // File doesn't exist
    
    // Verify default values are applied
    TEST_ASSERT_EQUAL_STRING("", ssid);
    TEST_ASSERT_EQUAL_STRING("", password);
    TEST_ASSERT_EQUAL_STRING("fluidnc.local", host);
    TEST_ASSERT_EQUAL_INT(81, port);
    TEST_ASSERT_EQUAL_STRING("ws", transport);
}

void test_netSave_with_defaults() {
    // Test saving with some default values
    bool saveResult = NetStore::netSave("my_ssid", "my_pass", nullptr, 0, nullptr);
    TEST_ASSERT_TRUE(saveResult);
    
    // Load and verify defaults are applied
    char ssid[64], password[64], host[64], transport[16];
    int port;
    
    bool loadResult = NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                                       host, sizeof(host), port, transport, sizeof(transport));
    TEST_ASSERT_TRUE(loadResult);
    
    TEST_ASSERT_EQUAL_STRING("my_ssid", ssid);
    TEST_ASSERT_EQUAL_STRING("my_pass", password);
    TEST_ASSERT_EQUAL_STRING("fluidnc.local", host); // Default applied
    TEST_ASSERT_EQUAL_INT(81, port); // Default applied
    TEST_ASSERT_EQUAL_STRING("ws", transport); // Default applied
}

void test_saveWifiCredentials_preserves_host_settings() {
    // First save complete settings
    NetStore::netSave("", "", "custom.host", 9090, "tcp");
    
    // Then save just WiFi credentials
    bool result = NetStore::saveWifiCredentials("new_ssid", "new_pass");
    TEST_ASSERT_TRUE(result);
    
    // Verify WiFi credentials updated but host settings preserved
    char ssid[64], password[64], host[64], transport[16];
    int port;
    
    NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                     host, sizeof(host), port, transport, sizeof(transport));
    
    TEST_ASSERT_EQUAL_STRING("new_ssid", ssid);
    TEST_ASSERT_EQUAL_STRING("new_pass", password);
    TEST_ASSERT_EQUAL_STRING("custom.host", host); // Preserved
    TEST_ASSERT_EQUAL_INT(9090, port); // Preserved
    TEST_ASSERT_EQUAL_STRING("tcp", transport); // Preserved
}

void test_saveFluidNCHost_preserves_wifi_settings() {
    // First save WiFi credentials
    NetStore::netSave("wifi_ssid", "wifi_pass", "", 0, "");
    
    // Then save host settings
    bool result = NetStore::saveFluidNCHost("new.host", 8888);
    TEST_ASSERT_TRUE(result);
    
    // Verify host updated but WiFi preserved
    char ssid[64], password[64], host[64], transport[16];
    int port;
    
    NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                     host, sizeof(host), port, transport, sizeof(transport));
    
    TEST_ASSERT_EQUAL_STRING("wifi_ssid", ssid); // Preserved
    TEST_ASSERT_EQUAL_STRING("wifi_pass", password); // Preserved
    TEST_ASSERT_EQUAL_STRING("new.host", host);
    TEST_ASSERT_EQUAL_INT(8888, port);
    TEST_ASSERT_EQUAL_STRING("ws", transport); // Default
}

void test_clear_removes_file() {
    // Save some settings
    NetStore::netSave("test", "test", "test", 1234, "test");
    
    // Clear them
    NetStore::clear();
    
    // Verify file is gone and defaults are returned
    char ssid[64], password[64], host[64], transport[16];
    int port;
    
    bool loadResult = NetStore::netLoad(ssid, sizeof(ssid), password, sizeof(password), 
                                       host, sizeof(host), port, transport, sizeof(transport));
    TEST_ASSERT_FALSE(loadResult); // No file
    
    // Should have defaults
    TEST_ASSERT_EQUAL_STRING("fluidnc.local", host);
    TEST_ASSERT_EQUAL_INT(81, port);
    TEST_ASSERT_EQUAL_STRING("ws", transport);
}

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();    // IMPORTANT LINE!
    
    RUN_TEST(test_netLoad_missing_file_defaults);
    RUN_TEST(test_netSave_netLoad_complete_settings);
    RUN_TEST(test_netSave_with_defaults);
    RUN_TEST(test_saveWifiCredentials_preserves_host_settings);
    RUN_TEST(test_saveFluidNCHost_preserves_wifi_settings);
    RUN_TEST(test_clear_removes_file);
    
    UNITY_END(); // stop unit testing
}

void loop() {
    // Empty. Things are done in setup() function.
}

#else

// Stub test when WiFi pendant is not enabled
void setup() {
    delay(2000);
    UNITY_BEGIN();
    // No tests to run - WiFi pendant disabled
    UNITY_END();
}

void loop() {}

#endif // USE_WIFI_PENDANT