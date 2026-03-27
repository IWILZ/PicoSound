# PicoSound Basic Example - PlatformIO

Complete working example with PlatformIO project structure.

## Hardware Required

- Raspberry Pi Pico
- 3 push buttons (GP18, GP19, GP20)
- MAX98357A I2S amplifier or speaker on PWM pin

## Quick Start PlatformIO

### 1. Hardware Setup

Connect 3 buttons to your Pico pins:
- GP18 → Button → GND (Beep)
- GP19 → Button → GND (Explosion)
- GP20 → Button → GND (Melody)

I2S Audio (MAX98357A):
- GP14 → BCK
- GP15 → WS
- GP16 → DIN

### 2. Create platformio.ini

```ini
[env:pico]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = pico
framework = arduino

; Use the earlephilhower core 
board_build.core = earlephilhower

; Serial monitor
monitor_speed = 115200

; Memory config to reserve space for WAVs on LittleFS
board_build.filesystem_size = 1m
board_build.filesystem = littlefs

; Libraries
lib_deps = 
    https://github.com/IWILZ/PicoSound.git
    https://github.com/IWILZ/PicoSem.git

; Build flags
build_flags = 
    -D PICO_BOARD=pico
    -I include
    
; Upload settings
upload_protocol = picotool
```

### 3. Create the program structure 

Create the following structure:
```
02_PIO_Basic/
├── platformio.ini              Config
├── include/
│   └── picosound_user_cfg.h    Your sounds
│
└── src/
    └── main.cpp                Main code
```
### 4. Open in PlatformIO

```bash
cd examples/01_PIO_Basic
pio run
```

Or open folder in `VSCode` with PlatformIO extension.

### 5. Upload

```bash
pio run --target upload
```

### 6. Test

Press buttons to hear sounds!

-------

## Customization

Edit `include/picosound_user_cfg.h` to:
- Change pin assignments
- Add your own sounds
- Adjust volume levels


## How It Works

### Core0 (Program Logic)
Runs your main program in `main.cpp` and sends audio commands to Core1 via `SendAudioCommand()`.

### Core1 (Audio Engine)
Runs independently in `setup1()` and `loop1()`. Handles all audio syntesis, streaming, mixing and I2S output.

### Configuration
All sound definitions are in `include/picosound_user_cfg.h`.  
This keeps your main code clean and makes sounds reusable across multiple files and for larger projects you can also use the other 2 config files by the templates, including them into `include/picosound_user_cfg.h`

```cpp
#include <PicoSound.h>    
#include "picosound_melodies.h"
#include "picosound_samples.h"
```


## Next Steps

- Add more sounds to the PICOSOUND_TABLE
- Try WAV files from LittleFS
- Experiment with different waveforms
- Build your own program!
