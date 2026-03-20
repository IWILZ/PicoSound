# PicoSound

Dual-core audio engine for Raspberry Pi Pico (RP2040).

PicoSound delivers glitch-free audio for any RP2040 application by dedicating 
Core1 exclusively to audio processing. While your main program runs on Core0 
(handling graphics, I/O, calculations, or game logic), Core1 manages the audio 
mixer and output independently.

This architecture eliminates audio dropouts caused by blocking operations like 
SPI transfers, flash writes, or heavy computations. Audio continues uninterrupted 
regardless of what Core0 is doing.

Supports both synthesized sounds (waveforms, melodies, effects) and streaming 
from WAV files. Not limited to games, but suitable for any project requiring reliable 
audio output: synthesizers, musical instruments, notification systems, interactive 
installations, or educational tools.

## Features

- Non-blocking audio on Core1
- 8-channel mixer
- I2S (MAX98357A) or PWM output
- Synthesized sounds (sine, square, sawtooth, triangle, noise, explosion)
- Melody sequencer
- WAV streaming (LittleFS or PROGMEM)
- Frequency sweeps

## Architecture

```
Core0: Game logic          Core1: Audio engine
  ↓                           ↓
SendAudioCommand() ----→ PicoSound.loop()
  (via PicoSem)              (mixer + output)
```

Audio runs independently. No blocking, no glitches.

## Quick Start

### 1. Install

Copy `PicoSound/` to your Arduino libraries folder or PlatformIO `lib/` directory.

### 2. Configure

Copy `src/templates/picosound_user_cfg_template.h` to your project's `include/` folder as `picosound_user_cfg.h`.

Edit pin assignments and add your sounds.

### 3. Code

```cpp
#include <PicoSound_AudioCore.h>
#include <PicoSound_DualCore.h>

void setup() {
  // Your setup
}

void loop() {
  // Game logic
  if (buttonPressed) {
    PlayTone(440, 200);
  }
}

void setup1() {
  PicoSound_AudioCore_Setup1();
}

void loop1() {
  PicoSound_AudioCore_Loop1();
}
```

That's it. Audio runs automatically on Core1.

## API

High-level functions (recommended):

```cpp
PlayTone(frequency, duration, volume);
PlayMelody(melody_array, length, waveform, volume);
SendAudioCommand(CMD_PLAY_SOUND, sound_id, volume);
StopAllSounds();
SetMasterVolume(volume);
```

See [API Reference](docs/API.md) for details.

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

See [Hardware Guide](docs/HARDWARE.md) for schematics.

## Examples

- [01_BasicGame](examples/01_BasicGame/) - Minimal game with sound
- [02_Melodies](examples/02_Melodies/) - Playing note sequences
- [03_WAV_LittleFS](examples/03_WAV_LittleFS/) - Streaming from flash
- [04_WAV_PROGMEM](examples/04_WAV_PROGMEM/) - Embedded samples

## Requirements

- Raspberry Pi Pico (RP2040)
- Arduino-Pico core (Earle Philhower)
- [PicoSem library](https://github.com/example/PicoSem)

## License

GPL-3.0

## Author

IWILZ (2026)
