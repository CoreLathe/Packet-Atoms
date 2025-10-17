// Test for key length validation in jet()
#include <stdio.h>
#include <string.h>
#include "../src/packet_atoms.h"

int main(void) {
    char value[32];
    jet_err err;
    
    // Test 1: Normal key (5 chars) - should work
    const char *json1 = "{\"temp\":22.5}";
    err = jet(json1, "temp", value, sizeof(value));
    if (err != JET_OK) {
        printf("FAIL: Normal key failed (err=%d)\n", err);
        return 1;
    }
    printf("PASS: Normal key (5 chars)\n");
    
    // Test 2: Long but valid key (57 chars) - should work
    // needle will be: "key":  (57 + 3 = 60 chars, fits in 64-byte buffer)
    const char *json2 = "{\"temperature_sensor_reading_celsius_from_external_device_1\":22.5}";
    err = jet(json2, "temperature_sensor_reading_celsius_from_external_device_1", value, sizeof(value));
    if (err != JET_OK) {
        printf("FAIL: 57-char key failed (err=%d)\n", err);
        return 1;
    }
    printf("PASS: Long valid key (57 chars)\n");
    
    // Test 3: Maximum valid key (60 chars) - should work
    // needle will be: "key":  (60 + 3 = 63 chars, fits in 64-byte buffer with null)
    const char *json3 = "{\"temperature_sensor_reading_celsius_from_external_device_0001\":22.5}";
    err = jet(json3, "temperature_sensor_reading_celsius_from_external_device_0001", value, sizeof(value));
    if (err != JET_OK) {
        printf("FAIL: 60-char key failed (err=%d)\n", err);
        return 1;
    }
    printf("PASS: Maximum valid key (60 chars)\n");
    
    // Test 4: Too long key (61 chars) - should return JET_MALFORMED
    // needle would be: "key":  (61 + 3 = 64 chars, snprintf returns 64 >= 64)
    const char *too_long_key = "temperature_sensor_reading_celsius_from_external_device_00001";
    err = jet("{}", too_long_key, value, sizeof(value));
    if (err != JET_MALFORMED) {
        printf("FAIL: 61-char key should return JET_MALFORMED, got %d\n", err);
        return 1;
    }
    printf("PASS: Too long key (61 chars) returns JET_MALFORMED\n");
    
    // Test 5: Very long key (100 chars) - should return JET_MALFORMED
    const char *very_long_key = "this_is_an_extremely_long_key_name_that_definitely_exceeds_the_buffer_limit_and_should_be_rejected_immediately";
    err = jet("{}", very_long_key, value, sizeof(value));
    if (err != JET_MALFORMED) {
        printf("FAIL: 100-char key should return JET_MALFORMED, got %d\n", err);
        return 1;
    }
    printf("PASS: Very long key (100 chars) returns JET_MALFORMED\n");
    
    printf("\nAll key length tests passed!\n");
    return 0;
}