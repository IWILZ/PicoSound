# PicoSound

![PicoSound Audio Library](https://raw.githubusercontent.com/IWILZ/PicoSound/main/docs/images/picosound_banner.jpg)

![Branch](https://img.shields.io/badge/Branch-Arduino%20Style-green)
[![RP2040](https://img.shields.io/badge/RP2040-Raspberry%20Pi%20Pico-C51A4A?logo=raspberrypi)](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html)
[![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D?logo=arduino)](https://arduino-pico.readthedocs.io/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange?logo=platformio)](https://registry.platformio.org/platforms/platformio/raspberrypi)
[![License](https://img.shields.io/badge/License-GPL--3.0-blue)](LICENSE)

Dual-core audio engine for Raspberry Pi Pico (RP2040).

PicoSound delivers glitch-free audio for any RP2040 application by dedicating 
Core1 exclusively to audio processing. While your main program runs on Core0 
(handling graphics, I/O, calculations, or game logic), Core1 manages the audio 
mixer and output independently.

This architecture eliminates audio dropouts caused by blocking operations like 
SPI transfers, flash writes, or heavy computations. Audio continues uninterrupted 
regardless of what Core0 is doing.

Supports both synthesized sounds (waveforms, melodies, effects) and streaming 
from WAV files. Not limited to games—suitable for any project requiring reliable 
audio output: synthesizers, musical instruments, notification systems, interactive 
installations, or educational tools.

Works seamlessly with both Arduino IDE and PlatformIO. The library uses 
standard C++ structure (.h/.cpp files) ensuring compatibility across 
development environments.

## Configuration Approach

> **This branch uses inline configuration** - sounds are defined directly in your sketch (.ino file).
> 
> For PlatformIO users preferring separate config files, see the [platformio-style branch](../../tree/platformio-style).

## Features

- Non-blocking audio on Core1
- 8-channel mixer
- I2S (MAX98357A) or PWM output
- Synthesized sounds (sine, square, sawtooth, triangle, noise, explosion)
- Melody sequencer
- WAV streaming (LittleFS or PROGMEM)
- Frequency sweeps
- Arduino IDE and PlatformIO compatible

## Architecture

```
Core0: Program logic     Core1: Audio engine
        ↓                       ↓
SendAudioCommand() ----→ PicoSound.loop()
  (via PicoSem)          (mixer + output)
```

Audio runs independently. No blocking, no glitches.

## Quick Start (Arduino IDE)

### 1. Install

Download: Code → Download ZIP  
Arduino IDE: Sketch → Include Library → Add .ZIP Library

### 2. Code

```cpp
// ========== CONFIGURATION (before includes) ==========
#define USER_SND_OUT    OUT_I2S
#define USER_PIN_BCK    14
#define USER_PIN_WS     15
#define USER_PIN_DATA   16

typedef enum {
  SND_NONE = 0,
  SND_BEEP,
  SND_LASER,
} SoundID;

// ========== INCLUDES ==========
#include <PicoSound_AudioCore.h>
#include <PicoSound_DualCore.h>

// ========== SOUND TABLE ==========
const SoundDefinition PICOSOUND_TABLE[] = {
  {WAVE_NONE, 0, 0, 0, 0, 100, nullptr, 0, false, nullptr, false},
  {WAVE_SQUARE, 440, 0, 100, 12000, 80, nullptr, 0, false, nullptr, false},
  {WAVE_SQUARE, 1500, 200, 200, 12000, 70, nullptr, 0, false, nullptr, false},
};

// ========== PROGRAM ==========
void setup() {
  // Your setup here
}

void loop() {
  SendAudioCommand(CMD_PLAY_SOUND, SND_BEEP, 80);
  delay(1000);
}

void setup1() {
  PicoSound_AudioCore_Setup1();
}
void loop1() {
  PicoSound_AudioCore_Loop1();
}  
```

See [examples](examples/) for complete code.

## Quick Start (PlatformIO)

This branch works also with PlatformIO but for cleaner code organization with separate config files, use the [platformio-style branch](../../tree/platformio-style).

## API

High-level functions (recommended):

```cpp
PlayTone(frequency, duration, volume);
PlayMelody(melody_array, length, waveform, volume);
SendAudioCommand(CMD_PLAY_SOUND, sound_id, volume);
StopAllSounds();
SetMasterVolume(volume);
```

See [docs/API.md](docs/API.md) for complete reference.

## Hardware Setup

### I2S (MAX98357A)
```
Pico GP14 → BCK
Pico GP15 → WS (LRCLK)
Pico GP16 → DIN
MAX98357A SD → 3.3V (or GPIO for mute control)
```

### PWM (Speaker/Amplifier)
```
Pico GP17 → Speaker (+) via capacitor
GND → Speaker (-)
```

See [docs/HARDWARE.md](docs/HARDWARE.md) for detailed schematics.

## Configuration Styles Comparison

| Approach | Best For | Config Location |
|----------|----------|-----------------|
| **Inline** (this branch) | Arduino IDE | In `.ino` file |
| **Separate files** ([other branch](../../tree/platformio-style)) | PlatformIO | `include/picosound_user_cfg.h` |

### Inline Configuration (This Branch)

**Advantages:**
- ✅ Works perfectly in Arduino IDE
- ✅ Configuration per-project
- ✅ Survives library updates
- ✅ Self-documenting code

**Disadvantages:**
- ⚠️ Large sound tables in main file
- ⚠️ Less modular for complex projects

### Separate Config Files ([PlatformIO Branch](../../tree/platformio-style))

**Advantages:**
- ✅ Cleaner code organization
- ✅ Reusable config across files
- ✅ Better for large projects

**Disadvantages:**
- ❌ Arduino IDE doesn't support this well
- ⚠️ PlatformIO/VSCode only

## Which Branch Should I Use?

| Your Environment | Recommended Branch |
|-----------------|-------------------|
| Arduino IDE | **main** (this branch) |
| PlatformIO | [platformio-style](../../tree/platformio-style) |
| VSCode | [platformio-style](../../tree/platformio-style) |

## Examples

- [01_BasicExample](examples/01_BasicExample/) - Simple button-triggered sounds

## Requirements

- Raspberry Pi Pico (RP2040)
- Arduino-Pico core (Earle Philhower)
- [PicoSem library](https://github.com/IWILZ/PicoSem)

## License

GPL-3.0

## Author

IWILZ (2026)
