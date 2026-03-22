# PicoSound Basic Example - PlatformIO

Complete working example with PlatformIO project structure.

## Hardware Required

- Raspberry Pi Pico
- 3 push buttons (GP18, GP19, GP20)
- MAX98357A I2S amplifier or speaker on PWM pin

## Quick Start

### 1. Open in PlatformIO

```bash
cd examples/01_BasicGame_PIO
pio run
```

Or open folder in VSCode with PlatformIO extension.

### 2. Hardware Setup

Connect buttons:
- GP18 → Button → GND (Beep)
- GP19 → Button → GND (Explosion)
- GP20 → Button → GND (Melody)

I2S Audio (MAX98357A):
- GP14 → BCK
- GP15 → WS
- GP16 → DIN

### 3. Upload

```bash
pio run --target upload
```

### 4. Test

Press buttons to hear sounds!

## Customization

Edit `include/picosound_user_cfg.h` to:
- Change pin assignments
- Add your own sounds
- Adjust volume levels

## File Structure

```
01_BasicGame_PIO/
├── platformio.ini              Config
├── include/
│   └── picosound_user_cfg.h    Your sounds
└── src/
    └── main.cpp                Main code
```

## How It Works

### Core0 (Game Logic)
Runs your main program in `main.cpp`. Sends audio commands to Core1 via `SendAudioCommand()`.

### Core1 (Audio Engine)
Runs independently in `setup1()` and `loop1()`. Handles all audio mixing and I2S output.

### Configuration
All sound definitions are in `include/picosound_user_cfg.h`. This keeps your main code clean and makes sounds reusable across multiple files.

## Next Steps

- Add more sounds to the PICOSOUND_TABLE
- Try WAV files from LittleFS
- Experiment with different waveforms
- Build your own game!
