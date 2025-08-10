#include <unity.h>

#ifdef USE_WIFI_PENDANT

#include "System.h"
#include "transport/transport.h"
#include "net/net_config.h"

// Mock the wifiReady function for testing
static bool mock_wifi_ready = false;

// We need to provide a mock implementation since we can't easily mock the static class method
namespace {
    bool original_wifi_ready() {
        // This would be the original implementation, but for testing we use our mock
        return mock_wifi_ready;
    }
}

// External declarations from SystemArduino.cpp
extern Transport* transport;
extern void selectTransport();

void setUp(void) {
    // Setup for each test - reset transport state
    mock_wifi_ready = false;
}

void tearDown(void) {
    // Cleanup after each test
}

void test_transport_selection_interface_exists() {
    // Test: Verify that selectTransport function exists and can be called
    mock_wifi_ready = false;
    
    // This should not crash
    selectTransport();
    
    // Transport should be set to something
    TEST_ASSERT_NOT_NULL(transport);
}

void test_serial_transport_interface_compliance() {
    // Test that Serial transport implements all required interface methods
    mock_wifi_ready = false;
    selectTransport();
    
    TEST_ASSERT_NOT_NULL(transport);
    
    // Test that methods exist and can be called
    bool connected = transport->isConnected(); // Should be true for serial
    TEST_ASSERT_TRUE(connected);
    
    // These methods should not crash when called
    transport->sendLine("$I", 1000);
    transport->sendRT(0x85); // Jog cancel command
    transport->resetFlowControl();
    
    // getChar and putChar depend on UART being initialized, but shouldn't crash
    // We'll just verify the interface exists by calling them
    transport->putChar('T');
    int c = transport->getChar(); // May return -1 if no data, but shouldn't crash
}

void test_transport_loop_interface() {
    // Test that transport->loop() can be called without issues
    mock_wifi_ready = false;
    selectTransport();
    
    TEST_ASSERT_NOT_NULL(transport);
    
    // This should not crash
    transport->loop();
    
    // Call it multiple times to ensure it's safe
    transport->loop();
    transport->loop();
}

void test_transport_selection_multiple_calls() {
    // Test that calling selectTransport multiple times is safe
    mock_wifi_ready = false;
    
    selectTransport();
    Transport* first_transport = transport;
    TEST_ASSERT_NOT_NULL(first_transport);
    
    selectTransport();
    Transport* second_transport = transport;
    TEST_ASSERT_NOT_NULL(second_transport);
    
    // Should still have a valid transport
    TEST_ASSERT_TRUE(transport->isConnected());
}

void setup() {
    delay(2000);  // Wait for serial monitor
    
    UNITY_BEGIN();
    
    RUN_TEST(test_transport_selection_interface_exists);
    RUN_TEST(test_serial_transport_interface_compliance);
    RUN_TEST(test_transport_loop_interface);
    RUN_TEST(test_transport_selection_multiple_calls);
    
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
    Serial.println("Transport selection tests skipped - USE_WIFI_PENDANT not defined");
}

void loop() {
    // Empty loop
}
#endif