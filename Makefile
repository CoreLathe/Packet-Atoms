# Makefile for Packet Atoms
# Supports: Linux, macOS, embedded cross-compilation

CC ?= gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -O2

# Directories
SRC_DIR = src
TEST_DIR = tests
EXAMPLE_DIR = examples

# Source files
HEADER = $(SRC_DIR)/packet_atoms.h
TORTURE_TEST = $(TEST_DIR)/torture_test.c
REAL_WORLD_TEST = $(TEST_DIR)/real_world_test.c
SIZE_TEST = $(TEST_DIR)/size_test.c
KEY_LENGTH_TEST = $(TEST_DIR)/key_length_test.c
EXAMPLE = $(EXAMPLE_DIR)/example_bme280.c

# Build targets
TARGET = torture_test
REAL_TEST = real_world_test
KEY_LENGTH = key_length_test
EXAMPLE_BIN = example_bme280

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM = Linux
endif
ifeq ($(UNAME_S),Darwin)
    PLATFORM = macOS
endif

.PHONY: all clean test test-real size help validate strict

all: test

# Build and run all tests
test: $(TARGET) $(REAL_TEST) $(KEY_LENGTH)
	@echo "=== Running torture tests on $(PLATFORM) ==="
	./$(TARGET)
	@echo ""
	@echo "=== Running real-world protocol tests ==="
	./$(REAL_TEST)
	@echo ""
	@echo "=== Running key length validation tests ==="
	./$(KEY_LENGTH)

# Build and run real-world tests only
test-real: $(REAL_TEST)
	./$(REAL_TEST)

# Build test binaries
$(TARGET): $(TORTURE_TEST) $(HEADER)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(TARGET) $(TORTURE_TEST)

$(REAL_TEST): $(REAL_WORLD_TEST) $(HEADER)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(REAL_TEST) $(REAL_WORLD_TEST)

$(KEY_LENGTH): $(KEY_LENGTH_TEST) $(HEADER)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $(KEY_LENGTH) $(KEY_LENGTH_TEST)

# Show code sizes
size: $(TARGET) size_test.o size_test_arm.o
	@echo "=== Code Size Analysis ==="
	@echo ""
	@echo "Test binary size:"
	@size $(TARGET)
	@echo ""
	@echo "Header file size:"
	@wc -c $(HEADER)
	@echo ""
	@echo "Actual compiled function sizes (x86-64, -Os):"
	@nm --print-size size_test.o | grep " T " | awk '$$4 ~ /^(jet_tiny|jet|tlv)$$/ {printf "  %-12s %d bytes\n", $$4, strtonum("0x"$$2)}' | sort
	@echo ""
	@echo "Actual compiled function sizes (ARM Cortex-M4, -Os):"
	@arm-none-eabi-nm --print-size size_test_arm.o | grep " T " | awk '$$4 ~ /^(jet_tiny|jet|tlv)$$/ {printf "  %-12s %d bytes\n", $$4, strtonum("0x"$$2)}' | sort

size_test.o: $(SIZE_TEST) $(HEADER)
	$(CC) -Os -I$(SRC_DIR) -c $(SIZE_TEST) -o size_test.o

size_test_arm.o: $(SIZE_TEST) $(HEADER)
	arm-none-eabi-gcc -Os -mcpu=cortex-m4 -mthumb -I$(SRC_DIR) -c $(SIZE_TEST) -o size_test_arm.o

# Strict compilation test (multiple toolchains)
strict: clean
	@echo "=== Testing with strict flags ==="
	$(CC) -Wall -Wextra -Werror -Wpedantic -std=c99 -O2 -I$(SRC_DIR) -o $(TARGET) $(TORTURE_TEST)
	./$(TARGET)
	@echo "✓ Passed strict compilation"

# Cross-compilation examples (requires toolchains installed)
cross-arm:
	@echo "=== Cross-compiling for ARM Cortex-M ==="
	arm-none-eabi-gcc -Wall -Wextra -Werror -std=c99 -mcpu=cortex-m4 -mthumb \
		-I$(SRC_DIR) -c $(TORTURE_TEST) -o torture_test_arm.o
	@echo "✓ ARM compilation successful"

cross-esp32:
	@echo "=== Cross-compiling for ESP32 ==="
	xtensa-esp32-elf-gcc -Wall -Wextra -Werror -std=c99 \
		-I$(SRC_DIR) -c $(TORTURE_TEST) -o torture_test_esp32.o
	@echo "✓ ESP32 compilation successful"

# Build example
example: $(EXAMPLE_BIN)

$(EXAMPLE_BIN): $(EXAMPLE) $(HEADER)
	$(CC) -Wall -Wextra -std=c99 -I$(SRC_DIR) -o $(EXAMPLE_BIN) $(EXAMPLE)

# Memory analysis
valgrind: $(TARGET)
	@echo "=== Running Valgrind memory check ==="
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET)

# Static analysis
analyze:
	@echo "=== Running static analysis ==="
	cppcheck --enable=all --error-exitcode=1 --suppress=missingIncludeSystem \
		-I$(SRC_DIR) $(HEADER) $(TORTURE_TEST) $(REAL_WORLD_TEST)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(REAL_TEST) $(KEY_LENGTH) $(EXAMPLE_BIN) *.o *.out

# Help
help:
	@echo "Packet Atoms Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build and run tests (default)"
	@echo "  make test     - Build and run tests"
	@echo "  make test-real- Run real-world protocol tests only"
	@echo "  make example  - Build example program"
	@echo "  make size     - Show code size analysis"
	@echo "  make strict   - Test with strict compiler flags"
	@echo "  make valgrind - Run memory leak detection"
	@echo "  make analyze  - Run static analysis (requires cppcheck)"
	@echo "  make clean    - Remove build artifacts"
	@echo ""
	@echo "Cross-compilation (requires toolchains):"
	@echo "  make cross-arm   - Compile for ARM Cortex-M"
	@echo "  make cross-esp32 - Compile for ESP32"