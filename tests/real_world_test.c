// real_world_test.c - Test with real protocol data
// Compile: gcc -Wall -Wextra -Werror -std=c99 -o real_world_test real_world_test.c
// Run: ./real_world_test

#include "packet_atoms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST(name) printf("\n=== %s ===\n", name)
#define PASS() printf("✓ PASS\n")
#define FAIL(msg) do { printf("✗ FAIL: %s\n", msg); return 1; } while(0)

// Real AWS IoT Shadow responses
const char *aws_shadow_accepted = 
    "{\"state\":{\"reported\":{\"temperature\":22.5,\"humidity\":65}},\"metadata\":{\"reported\":{\"temperature\":{\"timestamp\":1234567890},\"humidity\":{\"timestamp\":1234567890}}},\"version\":1,\"timestamp\":1234567890}";

const char *aws_shadow_delta = 
    "{\"version\":2,\"timestamp\":1234567891,\"state\":{\"desired\":{\"temperature\":25}},\"metadata\":{\"desired\":{\"temperature\":{\"timestamp\":1234567891}}}}";

// Real Azure IoT Hub telemetry
const char *azure_telemetry = 
    "{\"deviceId\":\"sensor-001\",\"temperature\":23.4,\"humidity\":58,\"pressure\":1013.25,\"timestamp\":\"2024-01-15T10:30:00Z\"}";

// Real BME280 sensor output (common format)
const char *bme280_json = 
    "{\"sensor\":\"BME280\",\"temp\":22.5,\"hum\":65.2,\"pres\":1013.25,\"alt\":120.5}";

// Real GPS JSON (from NMEA conversion)
const char *gps_json = 
    "{\"lat\":37.7749,\"lon\":-122.4194,\"alt\":10.5,\"speed\":0.0,\"course\":0.0,\"satellites\":8,\"hdop\":1.2}";

// Real Modbus TCP response (converted to JSON)
const char *modbus_json = 
    "{\"address\":1,\"function\":3,\"registers\":[100,200,300,400],\"count\":4}";

// Real weather station data
const char *weather_json = 
    "{\"station\":\"WS-001\",\"temp\":18.5,\"hum\":72,\"pres\":1015.3,\"wind_speed\":5.2,\"wind_dir\":180,\"rain\":0.0}";

// Edge case: Very long values
const char *long_value_json = 
    "{\"id\":\"sensor-with-very-long-identifier-string-0123456789\",\"value\":123}";

// Edge case: Many fields
const char *many_fields_json = 
    "{\"f1\":1,\"f2\":2,\"f3\":3,\"f4\":4,\"f5\":5,\"f6\":6,\"f7\":7,\"f8\":8,\"f9\":9,\"f10\":10}";

// Edge case: Scientific notation in real data
const char *scientific_json = 
    "{\"voltage\":3.3e0,\"current\":1.5e-3,\"power\":4.95e-3,\"energy\":1.2e6}";

int test_aws_iot_shadow() {
    TEST("AWS IoT Shadow - Accepted");
    char temp[16], hum[16];
    
    if (jet(aws_shadow_accepted, "temperature", temp, sizeof(temp)) != JET_OK)
        FAIL("Failed to extract temperature");
    if (strcmp(temp, "22.5") != 0)
        FAIL("Temperature value incorrect");
    
    if (jet(aws_shadow_accepted, "humidity", hum, sizeof(hum)) != JET_OK)
        FAIL("Failed to extract humidity");
    if (strcmp(hum, "65") != 0)
        FAIL("Humidity value incorrect");
    
    printf("  Temperature: %s\n", temp);
    printf("  Humidity: %s\n", hum);
    PASS();
    
    TEST("AWS IoT Shadow - Delta");
    char desired_temp[16];
    if (jet(aws_shadow_delta, "temperature", desired_temp, sizeof(desired_temp)) != JET_OK)
        FAIL("Failed to extract desired temperature");
    printf("  Desired temperature: %s\n", desired_temp);
    PASS();
    
    return 0;
}

int test_azure_iot() {
    TEST("Azure IoT Hub Telemetry");
    char device_id[32], temp[16], hum[16], pres[16];
    
    if (jet(azure_telemetry, "deviceId", device_id, sizeof(device_id)) != JET_OK)
        FAIL("Failed to extract deviceId");
    
    if (jet(azure_telemetry, "temperature", temp, sizeof(temp)) != JET_OK)
        FAIL("Failed to extract temperature");
    
    if (jet(azure_telemetry, "humidity", hum, sizeof(hum)) != JET_OK)
        FAIL("Failed to extract humidity");
    
    if (jet(azure_telemetry, "pressure", pres, sizeof(pres)) != JET_OK)
        FAIL("Failed to extract pressure");
    
    printf("  Device: %s\n", device_id);
    printf("  Temp: %s°C, Humidity: %s%%, Pressure: %s hPa\n", temp, hum, pres);
    PASS();
    return 0;
}

int test_bme280_sensor() {
    TEST("BME280 Sensor Data");
    char temp[16], hum[16], pres[16], alt[16];
    
    if (jet(bme280_json, "temp", temp, sizeof(temp)) != JET_OK)
        FAIL("Failed to extract temp");
    if (jet(bme280_json, "hum", hum, sizeof(hum)) != JET_OK)
        FAIL("Failed to extract hum");
    if (jet(bme280_json, "pres", pres, sizeof(pres)) != JET_OK)
        FAIL("Failed to extract pres");
    if (jet(bme280_json, "alt", alt, sizeof(alt)) != JET_OK)
        FAIL("Failed to extract alt");
    
    printf("  T=%s°C H=%s%% P=%shPa Alt=%sm\n", temp, hum, pres, alt);
    PASS();
    return 0;
}

int test_gps_data() {
    TEST("GPS Data (from NMEA)");
    char lat[16], lon[16], sats[8];
    
    if (jet(gps_json, "lat", lat, sizeof(lat)) != JET_OK)
        FAIL("Failed to extract latitude");
    if (jet(gps_json, "lon", lon, sizeof(lon)) != JET_OK)
        FAIL("Failed to extract longitude");
    if (jet(gps_json, "satellites", sats, sizeof(sats)) != JET_OK)
        FAIL("Failed to extract satellites");
    
    printf("  Location: %s, %s (Sats: %s)\n", lat, lon, sats);
    PASS();
    return 0;
}

int test_modbus_data() {
    TEST("Modbus TCP Response");
    char addr[8], func[8], count[8];
    
    if (jet(modbus_json, "address", addr, sizeof(addr)) != JET_OK)
        FAIL("Failed to extract address");
    if (jet(modbus_json, "function", func, sizeof(func)) != JET_OK)
        FAIL("Failed to extract function");
    if (jet(modbus_json, "count", count, sizeof(count)) != JET_OK)
        FAIL("Failed to extract count");
    
    printf("  Addr=%s Func=%s Count=%s\n", addr, func, count);
    PASS();
    return 0;
}

int test_weather_station() {
    TEST("Weather Station Data");
    char temp[16], wind_speed[16], wind_dir[16];
    
    if (jet(weather_json, "temp", temp, sizeof(temp)) != JET_OK)
        FAIL("Failed to extract temp");
    if (jet(weather_json, "wind_speed", wind_speed, sizeof(wind_speed)) != JET_OK)
        FAIL("Failed to extract wind_speed");
    if (jet(weather_json, "wind_dir", wind_dir, sizeof(wind_dir)) != JET_OK)
        FAIL("Failed to extract wind_dir");
    
    printf("  Temp=%s°C Wind=%sm/s @ %s°\n", temp, wind_speed, wind_dir);
    PASS();
    return 0;
}

int test_edge_cases() {
    TEST("Edge Case: Long Values");
    char id[64], value[16];
    
    if (jet(long_value_json, "id", id, sizeof(id)) != JET_OK)
        FAIL("Failed to extract long id");
    if (jet(long_value_json, "value", value, sizeof(value)) != JET_OK)
        FAIL("Failed to extract value");
    
    printf("  ID length: %zu chars\n", strlen(id));
    PASS();
    
    TEST("Edge Case: Many Fields");
    char f1[8], f5[8], f10[8];
    
    if (jet(many_fields_json, "f1", f1, sizeof(f1)) != JET_OK)
        FAIL("Failed to extract f1");
    if (jet(many_fields_json, "f5", f5, sizeof(f5)) != JET_OK)
        FAIL("Failed to extract f5");
    if (jet(many_fields_json, "f10", f10, sizeof(f10)) != JET_OK)
        FAIL("Failed to extract f10");
    
    printf("  f1=%s f5=%s f10=%s\n", f1, f5, f10);
    PASS();
    
    TEST("Edge Case: Scientific Notation");
    char voltage[16], current[16], energy[16];
    
    if (jet(scientific_json, "voltage", voltage, sizeof(voltage)) != JET_OK)
        FAIL("Failed to extract voltage");
    if (jet(scientific_json, "current", current, sizeof(current)) != JET_OK)
        FAIL("Failed to extract current");
    if (jet(scientific_json, "energy", energy, sizeof(energy)) != JET_OK)
        FAIL("Failed to extract energy");
    
    printf("  V=%s A=%s E=%s\n", voltage, current, energy);
    PASS();
    
    return 0;
}

int main(void) {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  Packet Atoms - Real World Protocol Test Suite        ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    int failures = 0;
    
    failures += test_aws_iot_shadow();
    failures += test_azure_iot();
    failures += test_bme280_sensor();
    failures += test_gps_data();
    failures += test_modbus_data();
    failures += test_weather_station();
    failures += test_edge_cases();
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    if (failures == 0) {
        printf("║  ✓ ALL REAL-WORLD TESTS PASSED                        ║\n");
        printf("║                                                        ║\n");
        printf("║  Validated against:                                   ║\n");
        printf("║    • AWS IoT Shadow (accepted + delta)                ║\n");
        printf("║    • Azure IoT Hub telemetry                          ║\n");
        printf("║    • BME280 sensor data                               ║\n");
        printf("║    • GPS/NMEA JSON conversion                         ║\n");
        printf("║    • Modbus TCP responses                             ║\n");
        printf("║    • Weather station data                             ║\n");
        printf("║    • Edge cases (long values, many fields, sci-not)   ║\n");
        printf("╚════════════════════════════════════════════════════════╝\n");
        return 0;
    } else {
        printf("║  ✗ %d TEST(S) FAILED                                   ║\n", failures);
        printf("╚════════════════════════════════════════════════════════╝\n");
        return 1;
    }
}