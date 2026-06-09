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

The entire firmware lives in one file. Key constants (top of file):

| Constant | Purpose |
|----------|---------|
| `X` | Odds of disabling spy mode (`4` → 25% chance no peek) |
| `MAX_COUNT` / `MIN_COUNT` | Countdown range bounds (30 / 15) |
| `COUNTDOWN_DELAY` | Milliseconds per tick (1000) |
| Pin constants | `BUTTON_PIN`, `VIBRA_PIN`, `BUZZER_PIN`, `RANDOM_SEED_ANALOG_PIN` |

Per-game locals (computed in `loop()`, passed to `playGame()`):

| Variable | Purpose |
|----------|---------|
| `count` | Starting countdown value (randomised 15–29 each round) |
| `spy` | Secret countdown value at which the "spy peek" buzz fires |
| `spyflag` | Whether the spy peek is active this round |

**Game flow:**
1. `loop()` calls `blankDisplay()` then detects button press (A0 goes LOW).
2. On press: seeds RNG from `analogRead(A6) + millis()`, picks random `count` and `spy`, enters `playGame(count, spy, spyflag)`.
3. `playGame()` runs while button is held (A0 LOW); counts down one tick per second. At `spy` it fires three short buzzes via `spyPeek()`, then continues.
4. At `count == 0`: `boom()` activates vibra + buzzer in three pulses, shows `LoSt` segment pattern, then `blinkLose()` flashes until button is released.
5. Releasing the button (A0 HIGH) exits `playGame()` at any point; `loop()` restarts.

**Key helpers:** `blankDisplay()`, `loseDisplay()`, `spyPeek()`, `boom()`, `blinkLose()`.

**Dead code:** `rstDisplay()` and `resetFunc` remain from a removed "reset" feature and are never called.

**Serial debug:** Enabled at 9600 baud. To suppress all serial output in production, search-replace `////Serial.` → `//Serial.` (as noted in comments).

## Code Style

Formatting is enforced by `.clang-format` (Google style, Allman braces, 4-space indent, spaces inside parens/brackets, 100-column limit). Run `clang-format -i src/main.cpp` to reformat.
