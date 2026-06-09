# Table War — Bomb Prop (Updated Electronics)

An Arduino-based countdown timer game prop for the [Table War board game](https://www.printables.com/model/75025-table-war-board-game) by peaklin. This repo contains updated electronics: the original buzzer is replaced by a **DFPlayer Mini** MP3 module for richer sound, and the motor is driven through an **NPN transistor** for safe switching.

Press and hold the button to start the countdown. Release early to defuse. If time runs out — boom.

## Hardware

| Component | Details |
|-----------|---------|
| MCU | Arduino Pro Mini 16 MHz (ATmega168) |
| Display | TM1637 4-digit 7-segment |
| Audio | DFPlayer Mini (YX5200-24SS) + 8 Ω speaker |
| Motor | Vibration motor (e.g. DualShock rumble) via NPN transistor |
| Button | Momentary, active LOW |

## Wiring

```
Button      A0  ── button ── GND   (INPUT, active LOW)
Display     A2  ── TM1637 CLK
            A3  ── TM1637 DIO
RNG seed    A6  ── floating (entropy source)
Motor       D4  ──[1 kΩ]──► NPN base (e.g. 2N2222)
                    Collector ──[1N4007]──► Motor (−)
            5V  ──────────────────────────► Motor (+)
                    Emitter → GND
DFPlayer    D6  ──[1 kΩ]──► DFPlayer RX
            D7  ◄────────── DFPlayer TX
            5V  ──────────► DFPlayer VCC
            GND ──────────► DFPlayer GND
            SPK1/SPK2 ────► Speaker +/−
```

## SD Card Setup

Format a ≤ 32 GB card as FAT32 (MBR). Create a folder `01/` on the root and copy the two sound files:

```
01/
├── 001.mp3   spy peek — three short beeps
└── 002.mp3   explosion — rising alarm sweep
```

Sound files are included in `sound/01/` (tracked via Git LFS).

## Build & Flash

Requires [PlatformIO](https://platformio.org/).

```bash
pio run                    # build
pio run --target upload    # build and flash
pio device monitor         # serial monitor (9600 baud)
```

## Game Logic

1. Display blanks; waiting for button press.
2. Button held (A0 LOW) → RNG seeds from floating pin + `millis()`, picks a random countdown (15–29 s) and a secret "spy" tick.
3. Countdown runs at 1 s/tick. At the spy tick, there is a 75% chance (configurable via `X`) that `001.mp3` plays — a hint to the holder that time is running short.
4. At zero: `002.mp3` plays, motor pulses three times, display shows `LoSt`, then blinks until the button is released.
5. Releasing the button at any point resets to idle.

Key constants in `src/main.cpp`:

| Constant | Default | Purpose |
|----------|---------|---------|
| `X` | `4` | 1-in-X chance spy mode is skipped |
| `MAX_COUNT` | `30` | Upper bound of countdown range |
| `MIN_COUNT` | `15` | Lower bound of countdown range |
| `COUNTDOWN_DELAY` | `1000` | Milliseconds per tick |

## Credits

Based on the [Table War board game](https://www.printables.com/model/75025-table-war-board-game) by **peaklin** on Printables. This repo contains a derivative electronics design with updated firmware.

## License

MIT — see [LICENSE](LICENSE).
