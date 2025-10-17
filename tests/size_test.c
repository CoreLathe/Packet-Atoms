// size_test.c - Measure actual compiled size of each function
// Compile with: gcc -Os -c size_test.c -o size_test.o
// Then: size size_test.o

#include "packet_atoms.h"

// Test function that only uses jet_tiny
void test_jet_tiny_only(void) {
    const char *json = "{\"temp\":22.5}";
    char buf[16];
    jet_tiny(json, "\"temp\":", buf, sizeof(buf));
}

// Test function that only uses jet
void test_jet_only(void) {
    const char *json = "{\"temp\":22.5}";
    char buf[16];
    jet(json, "temp", buf, sizeof(buf));
}

// Test function that only uses tlv
void test_tlv_only(void) {
    uint8_t data[] = {0x01, 0x04, 0xAA, 0xBB, 0xCC, 0xDD};
    uint16_t len;
    tlv(data, sizeof(data), 0x01, &len);
}

// Test all three functions
void test_all(void) {
    const char *json = "{\"temp\":22.5}";
    char buf[16];
    
    jet_tiny(json, "\"temp\":", buf, sizeof(buf));
    jet(json, "temp", buf, sizeof(buf));
    
    uint8_t data[] = {0x01, 0x04, 0xAA, 0xBB, 0xCC, 0xDD};
    uint16_t len;
    tlv(data, sizeof(data), 0x01, &len);
}