# Packet Atoms

**106-164 byte JSON field extractor for controlled embedded formats**

MIT Licensed | Cross-compiled for ARM Cortex-M4 | Tested on Linux x86-64

---

## What It Does

Extract JSON/TLV fields on resource-constrained MCUs without malloc or complex parsers.

```c
#include "packet_atoms.h"

char temp[16];
jet("{\"temp\":22.5,\"hum\":65}", "temp", temp, sizeof(temp));
// temp = "22.5"
```

## Actual Compiled Sizes

**ARM Cortex-M4 (-Os optimization):**
- `jet_tiny`: **106 bytes** - Core field extractor
- `jet`: **42 bytes** - Convenience wrapper (calls jet_tiny)
- `tlv`: **42 bytes** - Binary TLV walker
- **Total: 190 bytes** for all three functions

**x86-64 (-Os optimization):**
- `jet_tiny`: 164 bytes
- `jet`: 128 bytes
- `tlv`: 55 bytes

**Size comparison:**
- cJSON: 3.2 kB + malloc
- JSMN: 800 B + 18 lines token iteration
- **Packet Atoms: 106-164 B + 2 lines**

---

## What Works (Free Tier)

```c
✅ {"temp":22.5,"hum":65}       // Works
✅ {"temp": 22.5}               // Spaces after colon OK (skipped automatically)
✅ {"x":123,"y":456}            // Multiple fields
✅ {"x":-45.6}                  // Negative numbers
✅ {"x":1.2e10}                 // Scientific notation

⚠️ {"hum":65,"temp":22}        // Field order matters (strstr finds first match)
⚠️ {"relax":1,"x":2}           // Substring collision ("x" in "relax")
❌ {"x":"hot"}                  // Strings not supported (extracts with quotes)
❌ {"x":[1,2,3]}                // Arrays (extracts garbage)
❌ {"x":{"y":1}}                // Nested objects (extracts garbage)
```

**Use when:** You control message format (sensor ↔ cloud, custom protocols)  
**Don't use:** Parsing third-party APIs with arbitrary structure

---

## API Reference

### `jet_err jet(const char *j, const char *k, char *v, size_t vmax)`

Extract field value by key name.

**Parameters:**
- `j` - JSON string to parse
- `k` - Key name (without quotes, **max 60 characters**)
- `v` - Output buffer for value
- `vmax` - Size of output buffer

**Returns:**
- `JET_OK` - Success
- `JET_KEY_MISSING` - Field not found
- `JET_MALFORMED` - Empty/invalid value, or **key exceeds 60 characters**
- `JET_TRUNCATED` - Value too large for buffer

**Key Length Limit:**
- Maximum key name: **60 characters**
- Reason: Fixed 64-byte stack buffer avoids malloc overhead
- For longer keys: Use `jet_tiny()` with custom buffer size

**Example:**
```c
char temp[16];
jet_err err = jet("{\"temp\":22.5}", "temp", temp, sizeof(temp));
if (err == JET_OK) {
    float t = atof(temp);  // Convert to float
}
```

### `jet_err jet_tiny(const char *j, const char *needle, char *v, size_t vmax)`

Low-level extractor using custom search pattern. **No key length limit.**

**Use when:**
- Key names exceed 60 characters
- Custom search patterns needed
- Maximum control over buffer allocation

**Example:**
```c
char val[16];
jet_tiny("{\"temp\":22.5}", "\"temp\":", val, sizeof(val));
```

**Long key example:**
```c
char needle[256];  // Custom buffer size
snprintf(needle, sizeof(needle), "\"%s\":", very_long_key_name);
jet_tiny(json, needle, value, sizeof(value));
```

### `uint8_t* tlv(uint8_t *buf, size_t buf_len, uint8_t tag, uint16_t *len)`

Walk Type-Length-Value binary data.

**Format:** `[tag:1][len:1][value:len] ...`

**Example:**
```c
uint8_t data[] = {0x01, 0x04, 0xAA, 0xBB, 0xCC, 0xDD};
uint16_t len;
uint8_t *val = tlv(data, sizeof(data), 0x01, &len);
// val points to {0xAA, 0xBB, 0xCC, 0xDD}, len = 4
```

---

## Quick Start

### 1. Copy Header
```bash

curl -O https://raw.githubusercontent.com/CoreLathe/Packet-Atoms/main/src/packet_atoms.h

# Or just copy src/packet_atoms.h to your project
```

### 2. Include in Your Project
```c
#include "packet_atoms.h"  // Adjust path as needed
```

### 3. Extract Fields
```c
char temp[16], hum[16];
const char *json = "{\"temp\":22.5,\"hum\":65}";

if (jet(json, "temp", temp, sizeof(temp)) == JET_OK) {
    printf("Temperature: %s°C\n", temp);
}

if (jet(json, "hum", hum, sizeof(hum)) == JET_OK) {
    printf("Humidity: %s%%\n", hum);
}
```

---

## Testing

### Compile and Run Tests
```bash
make test          # Run all tests
make test-real     # Real-world protocols only
make example       # Build example program
```

**Test coverage:**
- 20 test vectors
- Edge cases (truncation, malformed, missing keys)
- Known limitations (nested objects, arrays, strings)
- TLV malicious input handling
- Real-world sensor formats

---

## Examples

### BME280 Sensor
```c
const char *sensor = "{\"temp\":22.5,\"hum\":65,\"pres\":1013}";
char temp[16], hum[16], pres[16];

jet(sensor, "temp", temp, sizeof(temp));
jet(sensor, "hum", hum, sizeof(hum));
jet(sensor, "pres", pres, sizeof(pres));

printf("T=%s°C H=%s%% P=%shPa\n", temp, hum, pres);
```

### GPS Coordinates
```c
const char *gps = "{\"lat\":37.7749,\"lon\":-122.4194}";
char lat[16], lon[16];

jet(gps, "lat", lat, sizeof(lat));
jet(gps, "lon", lon, sizeof(lon));

printf("Location: %s, %s\n", lat, lon);
```

### AWS IoT Shadow
```c
const char *shadow = "{\"state\":{\"reported\":{\"temp\":22}}}";
char temp[16];

jet(shadow, "temp", temp, sizeof(temp));
printf("Reported temp: %s\n", temp);
```

---

## Validation & Testing

### Tested Platforms
- ✅ **ARM Cortex-M4** - Cross-compiled and size-verified
- ✅ **Linux x86-64** - Full test suite passing
- ✅ **Strict compilation** - `-Wall -Wextra -Werror -Wpedantic`

### Test Coverage
- **20 torture test vectors** - Edge cases, errors, limitations
- **10 real-world protocol tests** - AWS IoT, Azure IoT, BME280, GPS, Modbus, weather stations
- **Security testing** - TLV malicious input protection
- **Cross-compilation** - ARM Cortex-M verified

### Run Tests
```bash
make test          # Run all tests
make test-real     # Real-world protocols only
./validate.sh      # Complete validation suite
make size          # Show actual compiled sizes
```

### Validation Results
```
✓ 20 torture test vectors passed
✓ 10 real-world protocol tests passed
✓ ARM Cortex-M4 cross-compilation successful
✓ Strict compilation passed
✓ No memory leaks (valgrind)
```

---

## Limitations

### Known Issues (By Design)

1. **Key length limit (jet only)** - 60 character maximum
   - Use `jet_tiny()` for longer keys
2. **Field order matters** - Uses `strstr()`, finds first match
3. **Substring collisions** - `"x"` matches in `"relax"`
4. **No string support** - Extracts with quotes included
5. **No nested objects/arrays** - Extracts garbage

### Workarounds

**Long keys (>60 chars):** Use `jet_tiny()` directly
```c
char needle[256];  // Size as needed
snprintf(needle, sizeof(needle), "\"%s\":", long_key);
jet_tiny(json, needle, value, sizeof(value));
```

**Field order:** Design protocol with unique prefixes
```c
// Bad:  {"x":1,"relax":2}  // "x" matches in "relax"
// Good: {"relax":2,"x":1}  // "relax" comes first
```

**Strings:** Strip quotes manually
```c
char val[16];
jet(json, "name", val, sizeof(val));
if (val[0] == '"') {
    memmove(val, val+1, strlen(val));  // Remove leading quote
    val[strlen(val)-1] = '\0';         // Remove trailing quote
}
```

---

## Roadmap

### Week 0 (Free Tier - Available Now)
- ✅ packet_atoms.h (jet_tiny, jet, tlv) - 106-164 bytes compiled
- ✅ Torture test suite (20 vectors)
- ✅ Real-world protocol tests (AWS IoT, Azure IoT, BME280, GPS, Modbus, weather)
- ✅ Complete validation suite
- ✅ ARM Cortex-M4 cross-compilation verified
- ✅ Working examples with actual size measurements

### Week 4 (Pro Tier - If Validated)
**Build if:** 300+ stars OR 20+ email signups

- jet_pro (395 B): Arbitrary field order, whitespace
- jet_str (280 B): Quoted strings with escapes
- 5 additional examples
- **Price:** $49/yr early access, then $99/yr

[Sign up for Pro early access](http://www.corelathe.com/contact)

### Month 3+ (Custom Projects)
- Custom parser projects: $1,500 each
- 5-7 days after spec approval
- Includes: parser + tests + integration guide

---

## Why Not JSMN/cJSON?

| Parser | Size (ARM) | API Complexity | Tradeoff |
|--------|------------|----------------|----------|
| **cJSON** | 3.2 kB | Easy | Requires malloc |
| **JSMN** | 800 B | 18 lines per field | Token array iteration |
| **Packet Atoms** | **106 B** | 2 lines | Controlled formats only |

**Code comparison:**
```c
// JSMN: 18 lines
jsmn_parser p;
jsmntok_t t[128];
jsmn_init(&p);
int r = jsmn_parse(&p, json, strlen(json), t, 128);
// ... 15 more lines to extract "temp"

// Packet Atoms: 2 lines
char temp[16];
jet(json, "temp", temp, sizeof(temp));
```

**Size advantage:** 7.5x smaller than JSMN, 30x smaller than cJSON

---

## License

MIT License - See LICENSE file

**No warranty. Test thoroughly before production use.**

---

## Contributing

Issues and PRs welcome for:
- Bug fixes
- Documentation improvements
- Additional test cases
- Platform-specific examples

**Not accepting:**
- Feature requests for arbitrary JSON (use cJSON/JSMN)
- Breaking API changes

---

## Support

- GitHub Issues: Bug reports and questions
- Email: sales@corelathe.com
- Website: http://www.corelathe.com
  
---

**Built for embedded developers who control their protocols.**
