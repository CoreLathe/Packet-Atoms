# Contributing to Packet Atoms

Thank you for your interest in contributing!

## What We Accept

### ✅ Welcome Contributions
- **Bug fixes** - Code correctness issues
- **Documentation improvements** - Clarity, examples, typos
- **Test cases** - Additional edge cases or real-world scenarios
- **Platform examples** - ESP32, STM32, nRF52 integration guides
- **Build system improvements** - Better cross-compilation support

### ❌ Not Accepting
- **Feature requests for arbitrary JSON** - Use cJSON or JSMN instead
- **Breaking API changes** - Stability is a feature
- **Scope creep** - This is intentionally minimal

## Design Philosophy

**Packet Atoms is designed for controlled formats only.**

- You control both sender and receiver
- Predictable field order
- Known limitations are documented, not bugs
- Size matters more than features

## How to Contribute

### 1. Bug Reports
```
Title: [Bug] Brief description
Body:
- Expected behavior
- Actual behavior
- Minimal reproduction code
- Platform (ARM/x86, compiler version)
```

### 2. Pull Requests

**Before submitting:**
1. Run `./validate.sh` - All tests must pass
2. Add test case if fixing a bug
3. Update README.md if changing behavior
4. Keep changes minimal and focused

**PR template:**
```
## What
Brief description of change

## Why
Problem being solved

## Testing
- [ ] `make test` passes
- [ ] Added test case (if applicable)
- [ ] Updated documentation (if applicable)
```

### 3. Code Style
- Follow existing style (K&R-ish)
- No dependencies beyond libc
- Compile with `-Wall -Wextra -Werror`
- Keep functions small and readable

## Testing Requirements

All PRs must:
- Pass `make test` (30 tests)
- Pass `make strict` (pedantic compilation)
- Cross-compile for ARM (`make cross-arm`)

## Documentation

If your change affects users:
- Update README.md
- Add example if introducing new usage
- Update VALIDATION_REPORT.md if changing tests

## Questions?

- Open a GitHub Discussion for design questions
- Open an Issue for bugs or unclear documentation
- Email support@packetatoms.dev for private inquiries

## License

By contributing, you agree your contributions will be licensed under MIT.

---

**Remember:** This library is intentionally minimal. "No" is often the right answer.