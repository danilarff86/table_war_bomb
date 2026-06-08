# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arduino-based "table war bomb" game prop. A countdown timer with a hidden "spy peek" mechanic. Targets the **Arduino Pro Mini 16MHz ATmega168** via PlatformIO.

**Hardware pins:**
- `A0` — button (INPUT, active LOW)
- `A2`/`A3` — TM1637 4-digit 7-segment display (CLK/DIO)
- `A6` — floating analog pin used as entropy source for `randomSeed`
- `4` — vibration motor
- `6` — buzzer

## Commands

```bash
# Build firmware
pio run

# Build and upload to connected board
pio run --target upload

# Serial monitor (9600 baud)
pio device monitor

# Clean build artifacts
pio run --target clean
```

## Game Logic (src/main.cpp)

The entire firmware lives in one file. Key globals:

| Variable | Purpose |
|----------|---------|
| `count` | Starting countdown value (randomised 15–30 on each game start) |
| `spy` | Secret countdown value at which the "spy peek" buzz fires |
| `spyflag` | Whether the spy peek is active this round (skipped with 1-in-`X` probability) |
| `X` | Odds of disabling spy mode (currently `4` → 25% chance no peek) |

**Game flow:**
1. `loop()` waits for button release (A0 LOW → HIGH transition).
2. On release: seeds RNG from `analogRead(A6) + millis()`, picks random `count` and `spy`, enters `playGame()`.
3. `playGame()` counts down each second; at `spy` it fires three short buzzes (spy peek signal), then continues.
4. At `count == 0`: activates vibra + buzzer ("explosion"), shows `LoSt` segment pattern, then flashes until button is pressed again.

**Serial debug:** Enabled at 9600 baud. To suppress all serial output in production, search-replace `////Serial.` → `//Serial.` (as noted in comments).

## Code Style

Formatting is enforced by `.clang-format` (Google style, Allman braces, 4-space indent, spaces inside parens/brackets, 100-column limit). Run `clang-format -i src/main.cpp` to reformat.
