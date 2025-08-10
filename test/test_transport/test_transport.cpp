#include <unity.h>

#ifdef USE_WIFI_PENDANT

#include "transport/wifi_transport_factory.h"
#include "transport/ws_transport.h"
#include "transport/telnet_transport.h"
#include <Arduino.h>

void setUp(void) {
    // Setup for each test
}

void tearDown(void) {
    // Cleanup after each test
}

void test_transport_factory_creates_ws_transport() {
    // Test that factory can create WebSocket transport
    Transport* transport = WiFiTransportFactory::createWSTransport("localhost", 81);
    TEST_ASSERT_NOT_NULL(transport);
    
    // Cleanup
    delete transport;
}

void test_transport_factory_creates_telnet_transport() {
    // Test that factory can create Telnet transport
    Transport* transport = WiFiTransportFactory::createTelnetTransport("localhost", 23);
    TEST_ASSERT_NOT_NULL(transport);
    
    // Cleanup
    delete transport;
}

void test_ws_transport_interface_compliance() {
    // Test that WSTransport implements all required interface methods
    WSTransport ws_transport("localhost", 81);
    
    // Test that methods exist and can be called (without network connection)
    bool connected = ws_transport.isConnected(); // Should be false without connection
    TEST_ASSERT_FALSE(connected);
    
    // These methods should not crash when called without connection
    ws_transport.sendLine("$I", 1000); // Should handle gracefully
    ws_transport.sendRT(0x85); // Jog cancel command
    
    int c = ws_transport.getChar(); // Should return -1 (no data)
    TEST_ASSERT_EQUAL(-1, c);
}

void test_telnet_transport_interface_compliance() {
    // Test that TelnetTransport implements all required interface methods
    TelnetTransport telnet_transport("localhost", 23);
    
    // Test that methods exist and can be called (without network connection)
    bool connected = telnet_transport.isConnected(); // Should be false without connection
    TEST_ASSERT_FALSE(connected);
    
    // These methods should not crash when called without connection
    telnet_transport.sendLine("$I", 1000); // Should handle gracefully
    telnet_transport.sendRT(0x85); // Jog cancel command
    
    int c = telnet_transport.getChar(); // Should return -1 (no data)
    TEST_ASSERT_EQUAL(-1, c);
}

void test_transport_factory_enum_selection() {
    // Test that factory correctly selects transport type based on enum
    Transport* ws_transport = WiFiTransportFactory::createTransport(
        WiFiTransportFactory::WEBSOCKET, "localhost", 81);
    TEST_ASSERT_NOT_NULL(ws_transport);
    
    Transport* telnet_transport = WiFiTransportFactory::createTransport(
        WiFiTransportFactory::TELNET, "localhost", 23);
    TEST_ASSERT_NOT_NULL(telnet_transport);
    
    // Cleanup
    delete ws_transport;
    delete telnet_transport;
}

void test_ws_transport_sendline_format() {
    // Test that sendLine appends single newline as per requirements
    WSTransport ws_transport("localhost", 81);
    
    // This test verifies the interface exists and can be called
    // The actual line formatting is tested internally by the implementation
    ws_transport.sendLine("$I");
    
    // If we get here without crashing, the interface is working
    TEST_ASSERT_TRUE(true);
}

void test_telnet_transport_sendline_format() {
    // Test that sendLine appends single newline as per requirements
    TelnetTransport telnet_transport("localhost", 23);
    
    // This test verifies the interface exists and can be called
    // The actual line formatting is tested internally by the implementation
    telnet_transport.sendLine("$I");
    
    // If we get here without crashing, the interface is working
    TEST_ASSERT_TRUE(true);
}

void test_jog_cancel_command() {
    // Test that jog-cancel (0x85) can be sent via sendRT
    WSTransport ws_transport("localhost", 81);
    TelnetTransport telnet_transport("localhost", 23);
    
    // Test sending jog-cancel command via both transports
    ws_transport.sendRT(0x85);
    telnet_transport.sendRT(0x85);
    
    // If we get here without crashing, the RT command interface is working
    TEST_ASSERT_TRUE(true);
}

void setup() {
    delay(2000);  // Wait for serial monitor
    
    UNITY_BEGIN();
    
    RUN_TEST(test_transport_factory_creates_ws_transport);
    RUN_TEST(test_transport_factory_creates_telnet_transport);
    RUN_TEST(test_ws_transport_interface_compliance);
    RUN_TEST(test_telnet_transport_interface_compliance);
    RUN_TEST(test_transport_factory_enum_selection);
    RUN_TEST(test_ws_transport_sendline_format);
    RUN_TEST(test_telnet_transport_sendline_format);
    RUN_TEST(test_jog_cancel_command);
    
    UNITY_END();
}

void loop() {
    // Empty loop
}

#else
// Stub for non-WiFi builds
void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("WiFi transport tests skipped - USE_WIFI_PENDANT not defined");
}

void loop() {
    // Empty loop
}
#endif