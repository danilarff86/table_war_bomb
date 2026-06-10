# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arduino-based "table war bomb" game prop. A countdown timer with a hidden "spy peek" mechanic. Targets the **Arduino Pro Mini 16MHz ATmega168** via PlatformIO.

**Hardware pins:**
- `A0` — button (INPUT, active LOW — connected between A0 and GND, no pull-up resistor)
- `A2`/`A3` — TM1637 4-digit 7-segment display (CLK/DIO)
- `A6` — floating analog pin used as entropy source for `randomSeed`
- `4` — vibration motor (NPN transistor switch — see wiring below)
- `6` — DFPlayer Mini UART TX (Arduino → DFPlayer RX, via 1 kΩ resistor)
- `7` — DFPlayer Mini UART RX (DFPlayer TX → Arduino)

## Wiring Notes

**DFPlayer Mini (YX5200-24SS):**
```
DFPlayer VCC  → 5V
DFPlayer GND  → GND
DFPlayer RX   → D6 via 1 kΩ resistor
DFPlayer TX   → D7 (direct)
DFPlayer SPK1 → Speaker +
DFPlayer SPK2 → Speaker −
```
Use a ≤ 32 GB SD card formatted as FAT32 (MBR). Sound files live in `sound/01/` in this repo and must be copied to the `01/` folder on the SD card root.

**Vibration motor (DualShock rumble motor or similar):**
```
Arduino D4 ──[1 kΩ]──► Base  (NPN, e.g. 2N2222)
                        Collector ──[1N4007 flyback]──► Motor (−)
5V ──────────────────────────────────────────────────► Motor (+)
                        Emitter → GND
```
The flyback diode cathode faces 5V. The Arduino pin only drives the base; the motor is powered from the 5V rail directly.

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
| `DF_RX_PIN` / `DF_TX_PIN` | SoftwareSerial pins for DFPlayer (7 / 6) |
| `SOUND_SPY` / `SOUND_BOOM` | SD card file indices in folder `/01/` (1 / 2) |
| `BUTTON_PIN`, `VIBRA_PIN`, `RANDOM_SEED_ANALOG_PIN` | Other pin constants |

Per-game locals (computed in `loop()`, passed to `playGame()`):

| Variable | Purpose |
|----------|---------|
| `count` | Starting countdown value (randomised 15–29 each round) |
| `spy` | Secret countdown value at which the "spy peek" fires |
| `spyflag` | Whether the spy peek is active this round |

**Game flow:**
1. `loop()` calls `blankDisplay()` then detects button press (A0 goes LOW).
2. On press: seeds RNG from `analogRead(A6) + millis()`, picks random `count` and `spy`, enters `playGame(count, spy, spyflag)`.
3. `playGame()` runs while button is held (A0 LOW); counts down one tick per second. At `spy` it plays `001.mp3` via `spyPeek()`, then continues.
4. At `count == 0`: `boom()` plays `002.mp3`, activates the vibra motor in three pulses, shows `LoSt` segment pattern, then `blinkLose()` flashes until button is released.
5. Releasing the button (A0 HIGH) exits `playGame()` at any point; `loop()` restarts.

**Key helpers:** `blankDisplay()`, `loseDisplay()`, `spyPeek()`, `boom()`, `blinkLose()`, `printDFPlayerDetail()`.

**Dead code:** `rstDisplay()` and `resetFunc` remain from a removed "reset" feature and are never called.

**Serial debug:** Enabled at 9600 baud. To suppress all serial output in production, search-replace `////Serial.` → `//Serial.` (as noted in comments). On boot the firmware prints DFPlayer volume/file/folder readbacks to confirm SD card is mounted correctly.

## Sound Files

```
sound/
└── 01/
    ├── 001.mp3   spy peek — three short beeps
    └── 002.mp3   explosion — rising alarm sweep
```

Tracked via Git LFS. Generate with ffmpeg if needed:
```bash
# 001.mp3 — spy peek beeps
ffmpeg -f lavfi \
  -i "aevalsrc=sin(2*PI*880*t)*(between(t,0.05,0.22)+between(t,0.42,0.59)+between(t,0.79,0.96)):s=44100:d=1.1:c=mono" \
  -acodec libmp3lame -ab 128k sound/01/001.mp3

# 002.mp3 — explosion sweep
ffmpeg -f lavfi \
  -i "aevalsrc=sin(2*PI*(300+700*t/8)*t)*exp(-t/5):s=44100:d=8.0:c=mono" \
  -acodec libmp3lame -ab 128k sound/01/002.mp3
```

## Code Style

Formatting is enforced by `.clang-format` (Google style, Allman braces, 4-space indent, spaces inside parens/brackets, 100-column limit). Run `clang-format -i src/main.cpp` to reformat.
