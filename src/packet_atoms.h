// packet_atoms.h - MIT License - CoreLathe.com
// Micro-parser for controlled embedded JSON/TLV formats
// Version: 1.0.0

#ifndef PACKET_ATOMS_H
#define PACKET_ATOMS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Error codes
typedef enum {
    JET_OK = 0,
    JET_KEY_MISSING,
    JET_MALFORMED,
    JET_TRUNCATED
} jet_err;

/* jet_tiny - Core field extractor
 * Compiled size: 106 bytes (ARM Cortex-M4 -Os), 164 bytes (x86-64 -Os)
 *
 * REQUIRES:
 *   - Fixed field order in JSON
 *   - Controlled format (you define sender & receiver)
 *
 * HANDLES:
 *   - Optional spaces after colons (skipped automatically)
 * 
 * PARAMS:
 *   j      - JSON string to parse
 *   needle - Search pattern (e.g., "\"temp\":")
 *   v      - Output buffer for extracted value
 *   vmax   - Size of output buffer
 * 
 * RETURNS:
 *   JET_OK          - Success
 *   JET_KEY_MISSING - Field not found
 *   JET_MALFORMED   - Empty value
 *   JET_TRUNCATED   - Value too large for buffer
 */
jet_err jet_tiny(const char *j, const char *needle, char *v, size_t vmax) {
    char *p = strstr(j, needle);
    if (!p) return JET_KEY_MISSING;
    
    p += strlen(needle);
    while (*p == ' ') p++;  // Skip optional spaces
    
    size_t n = 0;
    while (*p && *p != ',' && *p != '}' && n < vmax - 1) {
        v[n++] = *p++;
    }
    v[n] = '\0';
    
    if (n == 0) return JET_MALFORMED;
    if (*p && *p != ',' && *p != '}') return JET_TRUNCATED;
    
    return JET_OK;
}

/* jet - Convenience wrapper
 * Compiled size: 42 bytes (ARM Cortex-M4 -Os), 128 bytes (x86-64 -Os)
 *
 * Automatically constructs needle pattern from key name.
 * Note: Calls jet_tiny(), so total size includes both functions.
 *
 * PARAMS:
 *   j    - JSON string to parse
 *   k    - Key name (without quotes, max 60 characters)
 *   v    - Output buffer for extracted value
 *   vmax - Size of output buffer
 *
 * RETURNS:
 *   JET_MALFORMED - If key name exceeds 60 characters
 *   (other codes from jet_tiny)
 *
 * EXAMPLE:
 *   char temp[16];
 *   jet("{\"temp\":22.5}", "temp", temp, sizeof(temp));
 *   // temp now contains "22.5"
 */
jet_err jet(const char *j, const char *k, char *v, size_t vmax) {
    char needle[64];
    int n = snprintf(needle, sizeof(needle), "\"%s\":", k);
    if (n >= (int)sizeof(needle)) return JET_MALFORMED;  // Key too long
    return jet_tiny(j, needle, v, vmax);
}

/* tlv - Binary TLV walker
 * Compiled size: 42 bytes (ARM Cortex-M4 -Os), 55 bytes (x86-64 -Os)
 *
 * Walks Type-Length-Value encoded binary data.
 * 
 * FORMAT:
 *   [tag:1][len:1][value:len] ...
 * 
 * PARAMS:
 *   buf     - Buffer containing TLV data
 *   buf_len - Length of buffer
 *   tag     - Tag to search for
 *   len     - Output: length of found value
 * 
 * RETURNS:
 *   Pointer to value data, or NULL if not found
 */
uint8_t* tlv(uint8_t *buf, size_t buf_len, uint8_t tag, uint16_t *len) {
    uint8_t *end = buf + buf_len;
    
    while (buf + 2 <= end && *buf != tag) {
        size_t record_len = buf[1];
        if (buf + 2 + record_len > end) return NULL;
        buf += 2 + record_len;
    }
    
    if (buf + 2 > end) return NULL;
    
    // Validate that the claimed length fits in remaining buffer
    size_t claimed_len = buf[1];
    if (buf + 2 + claimed_len > end) return NULL;
    
    *len = buf[1];
    return buf + 2;
}

#endif // PACKET_ATOMS_H
