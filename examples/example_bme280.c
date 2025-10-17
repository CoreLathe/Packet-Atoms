// example_bme280.c - BME280 sensor JSON parsing example
// Compile: gcc -Wall -Wextra -std=c99 -o example_bme280 example_bme280.c
// Run: ./example_bme280

#include "packet_atoms.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // Simulated BME280 sensor JSON response
    const char *sensor_json = "{\"temp\":22.5,\"hum\":65,\"pres\":1013.25}";
    
    char temp[16], hum[16], pres[16];
    
    printf("Parsing BME280 sensor data...\n");
    printf("JSON: %s\n\n", sensor_json);
    
    // Extract temperature
    if (jet(sensor_json, "temp", temp, sizeof(temp)) == JET_OK) {
        float temp_c = atof(temp);
        printf("Temperature: %.1f°C (%.1f°F)\n", temp_c, temp_c * 9.0/5.0 + 32.0);
    } else {
        printf("Error: Failed to extract temperature\n");
        return 1;
    }
    
    // Extract humidity
    if (jet(sensor_json, "hum", hum, sizeof(hum)) == JET_OK) {
        printf("Humidity:    %s%%\n", hum);
    } else {
        printf("Error: Failed to extract humidity\n");
        return 1;
    }
    
    // Extract pressure
    if (jet(sensor_json, "pres", pres, sizeof(pres)) == JET_OK) {
        printf("Pressure:    %s hPa\n", pres);
    } else {
        printf("Error: Failed to extract pressure\n");
        return 1;
    }
    
    printf("\n✓ Successfully parsed all fields\n");
    printf("Code size: ~170 bytes (jet_tiny only)\n");
    
    return 0;
}