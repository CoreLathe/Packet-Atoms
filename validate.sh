#!/bin/bash
# validate.sh - Complete validation suite for Packet Atoms
# Run before launch to verify everything works

set -e  # Exit on error

echo "╔════════════════════════════════════════════════════════╗"
echo "║  Packet Atoms - Complete Validation Suite             ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

pass() {
    echo -e "${GREEN}✓${NC} $1"
}

fail() {
    echo -e "${RED}✗${NC} $1"
    exit 1
}

warn() {
    echo -e "${YELLOW}⚠${NC} $1"
}

# 1. Clean build
echo "1. Clean Build Test"
make clean > /dev/null 2>&1 || true
make test > /dev/null 2>&1 || fail "Build failed"
pass "Clean build successful"
echo ""

# 2. Strict compilation
echo "2. Strict Compilation Test"
make clean > /dev/null 2>&1 || true
make strict > /dev/null 2>&1 || fail "Strict compilation failed"
pass "Strict compilation passed (-Wall -Wextra -Werror -Wpedantic)"
echo ""

# 3. Cross-compilation (if toolchains available)
echo "3. Cross-Compilation Tests"
if command -v arm-none-eabi-gcc &> /dev/null; then
    make cross-arm > /dev/null 2>&1 || fail "ARM cross-compilation failed"
    pass "ARM Cortex-M cross-compilation successful"
else
    warn "ARM toolchain not installed (optional)"
fi

if command -v xtensa-esp32-elf-gcc &> /dev/null; then
    make cross-esp32 > /dev/null 2>&1 || fail "ESP32 cross-compilation failed"
    pass "ESP32 cross-compilation successful"
else
    warn "ESP32 toolchain not installed (optional)"
fi
echo ""

# 4. Static analysis (if available)
echo "4. Static Analysis"
if command -v cppcheck &> /dev/null; then
    make analyze > /dev/null 2>&1 || fail "Static analysis found issues"
    pass "Static analysis passed (cppcheck)"
else
    warn "cppcheck not installed (optional)"
fi
echo ""

# 5. Memory leak detection (if available)
echo "5. Memory Leak Detection"
if command -v valgrind &> /dev/null; then
    make valgrind > /dev/null 2>&1 || fail "Memory leaks detected"
    pass "No memory leaks detected (valgrind)"
else
    warn "valgrind not installed (optional)"
fi
echo ""

# 6. Code size analysis
echo "6. Code Size Analysis"
make size
echo ""

# 7. Run all tests
echo "7. Test Suite Execution"
make clean > /dev/null 2>&1 || true
make test || fail "Tests failed"
echo ""

# 8. Verify examples compile
echo "8. Example Compilation"
make example > /dev/null 2>&1 || fail "Example compilation failed"
pass "Example compiles successfully"
echo ""

# Summary
echo "╔════════════════════════════════════════════════════════╗"
echo "║  ✓ ALL VALIDATION CHECKS PASSED                       ║"
echo "║                                                        ║"
echo "║  Ready for launch:                                    ║"
echo "║    • Clean build: ✓                                   ║"
echo "║    • Strict compilation: ✓                            ║"
echo "║    • Torture tests (20 vectors): ✓                    ║"
echo "║    • Real-world protocols: ✓                          ║"
echo "║    • Examples: ✓                                      ║"
echo "║                                                        ║"
echo "║  Optional checks completed:                           ║"
if command -v arm-none-eabi-gcc &> /dev/null; then
    echo "║    • ARM cross-compile: ✓                             ║"
fi
if command -v xtensa-esp32-elf-gcc &> /dev/null; then
    echo "║    • ESP32 cross-compile: ✓                           ║"
fi
if command -v cppcheck &> /dev/null; then
    echo "║    • Static analysis: ✓                               ║"
fi
if command -v valgrind &> /dev/null; then
    echo "║    • Memory leak check: ✓                             ║"
fi
echo "╚════════════════════════════════════════════════════════╝"
echo ""
