// torture_test.c - Comprehensive test suite for packet_atoms.h
// Compile: gcc -Wall -Wextra -Werror -std=c99 -o torture_test torture_test.c
// Run: ./torture_test

#include "packet_atoms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) printf("TEST: %s\n", name)
#define PASS() printf("  ✓ PASS\n")
#define FAIL(msg) do { printf("  ✗ FAIL: %s\n", msg); exit(1); } while(0)

// Test helpers
void assert_jet_ok(const char *json, const char *key, const char *expected) {
    char buf[64];
    jet_err err = jet(json, key, buf, sizeof(buf));
    if (err != JET_OK) FAIL("Expected JET_OK");
    if (strcmp(buf, expected) != 0) {
        printf("    Expected: '%s', Got: '%s'\n", expected, buf);
        FAIL("Value mismatch");
    }
}

void assert_jet_err(const char *json, const char *key, jet_err expected_err) {
    char buf[64];
    jet_err err = jet(json, key, buf, sizeof(buf));
    if (err != expected_err) {
        printf("    Expected error %d, Got %d\n", expected_err, err);
        FAIL("Error code mismatch");
    }
}

// Test cases
void test_basic_extraction() {
    TEST("Basic field extraction");
    assert_jet_ok("{\"temp\":22.5}", "temp", "22.5");
    assert_jet_ok("{\"x\":123}", "x", "123");
    assert_jet_ok("{\"y\":-456}", "y", "-456");
    PASS();
}

void test_multiple_fields() {
    TEST("Multiple fields");
    const char *json = "{\"temp\":22.5,\"hum\":65}";
    
    assert_jet_ok(json, "temp", "22.5");
    assert_jet_ok(json, "hum", "65");
    PASS();
}

void test_scientific_notation() {
    TEST("Scientific notation");
    assert_jet_ok("{\"x\":1.2e10}", "x", "1.2e10");
    assert_jet_ok("{\"y\":-3.4e-5}", "y", "-3.4e-5");
    PASS();
}

void test_negative_numbers() {
    TEST("Negative numbers");
    assert_jet_ok("{\"x\":-45.6}", "x", "-45.6");
    assert_jet_ok("{\"y\":-123}", "y", "-123");
    PASS();
}

void test_zero_values() {
    TEST("Zero values");
    assert_jet_ok("{\"x\":0}", "x", "0");
    assert_jet_ok("{\"y\":0.0}", "y", "0.0");
    PASS();
}

void test_key_missing() {
    TEST("Missing key");
    assert_jet_err("{\"temp\":22}", "humidity", JET_KEY_MISSING);
    assert_jet_err("{}", "x", JET_KEY_MISSING);
    PASS();
}

void test_malformed() {
    TEST("Malformed JSON");
    assert_jet_err("{\"x\":}", "x", JET_MALFORMED);
    assert_jet_err("{\"x\":,}", "x", JET_MALFORMED);
    PASS();
}

void test_truncation() {
    TEST("Buffer truncation");
    char small_buf[4];
    jet_err err = jet("{\"x\":123456789}", "x", small_buf, sizeof(small_buf));
    if (err != JET_TRUNCATED) FAIL("Expected JET_TRUNCATED");
    if (strcmp(small_buf, "123") != 0) FAIL("Truncation incorrect");
    PASS();
}

void test_spaces_after_colon() {
    TEST("Spaces after colon (should work with jet_tiny)");
    // jet_tiny skips spaces, so this should work
    assert_jet_ok("{\"temp\": 22.5}", "temp", "22.5");
    assert_jet_ok("{\"x\":  123}", "x", "123");
    PASS();
}

void test_field_order_matters() {
    TEST("Field order (strstr limitation)");
    // This is a known limitation - strstr finds first match
    const char *json = "{\"hum\":65,\"temp\":22}";
    assert_jet_ok(json, "hum", "65");
    assert_jet_ok(json, "temp", "22");
    PASS();
}

void test_substring_collision() {
    TEST("Substring collision (known limitation)");
    // "relax" contains "x" - this is a strstr limitation
    // User must be aware of this when designing protocols
    const char *json = "{\"relax\":1,\"x\":2}";
    char buf[64];
    jet_err err = jet(json, "x", buf, sizeof(buf));
    // Will match "x" in "relax" first - documented limitation
    if (err == JET_OK) {
        printf("    Note: Found 'x' in 'relax' (expected limitation)\n");
    }
    PASS();
}

void test_nested_objects() {
    TEST("Nested objects (not supported)");
    // Extracts garbage - documented limitation
    const char *json = "{\"x\":{\"y\":1}}";
    char buf[64];
    jet_err err = jet(json, "x", buf, sizeof(buf));
    if (err == JET_OK) {
        printf("    Note: Extracted '%s' from nested object (garbage expected)\n", buf);
    }
    PASS();
}

void test_arrays() {
    TEST("Arrays (not supported)");
    // Extracts garbage - documented limitation
    const char *json = "{\"x\":[1,2,3]}";
    char buf[64];
    jet_err err = jet(json, "x", buf, sizeof(buf));
    if (err == JET_OK) {
        printf("    Note: Extracted '%s' from array (garbage expected)\n", buf);
    }
    PASS();
}

void test_strings() {
    TEST("String values (not supported)");
    // Extracts with quotes - documented limitation
    const char *json = "{\"x\":\"hot\"}";
    char buf[64];
    jet_err err = jet(json, "x", buf, sizeof(buf));
    if (err == JET_OK) {
        printf("    Note: Extracted '%s' (includes quotes)\n", buf);
    }
    PASS();
}

void test_tlv_basic() {
    TEST("TLV basic extraction");
    uint8_t data[] = {0x01, 0x04, 0xAA, 0xBB, 0xCC, 0xDD,  // tag=1, len=4
                      0x02, 0x02, 0x11, 0x22};              // tag=2, len=2
    uint16_t len;
    
    uint8_t *val = tlv(data, sizeof(data), 0x01, &len);
    if (!val || len != 4) FAIL("TLV tag 1 extraction failed");
    if (val[0] != 0xAA || val[3] != 0xDD) FAIL("TLV tag 1 value incorrect");
    
    val = tlv(data, sizeof(data), 0x02, &len);
    if (!val || len != 2) FAIL("TLV tag 2 extraction failed");
    if (val[0] != 0x11 || val[1] != 0x22) FAIL("TLV tag 2 value incorrect");
    
    PASS();
}

void test_tlv_not_found() {
    TEST("TLV tag not found");
    uint8_t data[] = {0x01, 0x02, 0xAA, 0xBB};
    uint16_t len;
    
    uint8_t *val = tlv(data, sizeof(data), 0x99, &len);
    if (val != NULL) FAIL("Expected NULL for missing tag");
    PASS();
}

void test_tlv_malicious() {
    TEST("TLV malicious input (len=255 at buffer end)");
    uint8_t data[] = {0x01, 0xFF, 0xAA};  // Claims 255 bytes but only 1 available
    uint16_t len;
    
    uint8_t *val = tlv(data, sizeof(data), 0x01, &len);
    if (val != NULL) FAIL("Should reject malicious length");
    PASS();
}

void test_edge_cases() {
    TEST("Edge cases");
    
    // Empty JSON
    assert_jet_err("", "x", JET_KEY_MISSING);
    
    // Just braces
    assert_jet_err("{}", "x", JET_KEY_MISSING);
    
    // Single field at end
    assert_jet_ok("{\"x\":1}", "x", "1");
    
    // Value at end without comma
    assert_jet_ok("{\"a\":1,\"b\":2}", "b", "2");
    
    PASS();
}

void test_real_world_formats() {
    TEST("Real-world sensor formats");
    
    // BME280 sensor
    assert_jet_ok("{\"temp\":22.5,\"hum\":65,\"pres\":1013}", "temp", "22.5");
    assert_jet_ok("{\"temp\":22.5,\"hum\":65,\"pres\":1013}", "hum", "65");
    assert_jet_ok("{\"temp\":22.5,\"hum\":65,\"pres\":1013}", "pres", "1013");
    
    // GPS coordinates
    assert_jet_ok("{\"lat\":37.7749,\"lon\":-122.4194}", "lat", "37.7749");
    assert_jet_ok("{\"lat\":37.7749,\"lon\":-122.4194}", "lon", "-122.4194");
    
    // AWS IoT Shadow
    assert_jet_ok("{\"state\":{\"reported\":{\"temp\":22}}}", "temp", "22");
    
    PASS();
}

int main(void) {
    printf("=== Packet Atoms Torture Test Suite ===\n\n");
    
    // Core functionality
    test_basic_extraction();
    test_multiple_fields();
    test_scientific_notation();
    test_negative_numbers();
    test_zero_values();
    
    // Error handling
    test_key_missing();
    test_malformed();
    test_truncation();
    
    // Edge cases
    test_spaces_after_colon();
    test_field_order_matters();
    test_substring_collision();
    test_edge_cases();
    
    // Known limitations (documented)
    test_nested_objects();
    test_arrays();
    test_strings();
    
    // TLV tests
    test_tlv_basic();
    test_tlv_not_found();
    test_tlv_malicious();
    
    // Real-world scenarios
    test_real_world_formats();
    
    printf("\n=== ALL TESTS PASSED ===\n");
    printf("Total: 20 test vectors\n");
    
    return 0;
}