# PicoSound WAV Player Example

Play WAV files from LittleFS filesystem with button triggers.

## Hardware Required

- Raspberry Pi Pico
- 3 push buttons (GP18, GP19, GP20)
- MAX98357A I2S amplifier
- WAV files (16kHz, 16-bit, Mono)

## WAV File Requirements
Three examples are included but if you want to make your own please read these notes.

**IMPORTANT:** WAV files must be:
- Sample rate:  **16kHz**
- Bit depth:    **16-bit**
- Channels:     **Mono**

Convert your WAV files using Audacity:
1. File     → Open your audio file
2. Tracks   → Resample → 16000 Hz
3. Tracks   → Mix → Mix Stereo Down to Mono (if stereo)
4. File     → Export → Export as WAV
   - Encoding: Signed 16-bit PCM
5. Save as sound1.wav, sound2.wav, sound3.wav

------

## Quick Start PlatformIO

### 1. Hardware Setup

Connect 3 buttons to your Pico pins:

- GP18 → Button → GND (Sound 1)
- GP19 → Button → GND (Sound 2)
- GP20 → Button → GND (Sound 3 - looping)

I2S Audio (MAX98357A)

- GP14 → BCK
- GP15 → WS (LRCLK)
- GP16 → DIN
- GP0  → DS mute/unmute (optional)

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
; -> to change the default 16KHz sound sample rate, use the following line
;   -D PICOSOUND_DEFAULT_SAMPLE_RATE=22050
    
; Upload settings
upload_protocol = picotool
```

### 3. Create the program structure 

Create the following structure and place WAV files in `data/` folder:
```
02_PIO_WAV/
├── platformio.ini              Config
├── include/
│   └── picosound_user_cfg.h    Your sounds
│
├── src/
│    └── main.cpp                Main code
│
└── data/
    ├── sound1.wav
    ├── sound2.wav
    └── sound3.wav
```

### 4. Upload Filesystem

```bash
pio run --target uploadfs
```
or use **Upload Filesystem Image** in VSCode

### 5. Upload Program

```bash
pio run --target upload
```

### 5. Monitor Serial

```bash
pio device monitor
```

The program lists all files found on LittleFS and you can play with buttons!

------

## Customization

### Sound Definitions

```cpp
// Play WAV from LittleFS (one-shot)
{WAVE_WAV, 0, 0, 0, 0, 80, nullptr, 0, false, "/sound1.wav", true}
                                                 ↑             ↑
                                                 |             |
                                            File name    Use filesystem

// Play WAV from LittleFS (looping)
{WAVE_WAV, 0, 0, 0, 0, 70, nullptr, 0, true, "/sound3.wav", true}
                                         ↑
                                         |
                                    Loop = true
```

### Parameters

| Field | Value | Meaning |
|-------|-------|---------|
| waveType | WAVE_WAV | WAV file playback |
| freq_start | 0 | Not used for WAV |
| freq_end | 0 | Not used for WAV |
| duration | 0 | Read from WAV file |
| amplitude | 0 | Read from WAV file |
| volume | 0-100 | Playback volume |
| progmem_data | nullptr | Not used (filesystem) |
| progmem_length | 0 | Not used (filesystem) |
| loop | true/false | Loop playback? |
| filesystem_path | "/file.wav" | Path on LittleFS |
| use_filesystem | true | Read from LittleFS |


### Configuration
All sound definitions are in `include/picosound_user_cfg.h`.  
This keeps your main code clean and makes sounds reusable across multiple files and for larger projects you can also use the other 2 config files by the templates, including them into `include/picosound_user_cfg.h`

```cpp
#include <PicoSound.h>    
#include "picosound_melodies.h"
#include "picosound_samples.h"
```

## Troubleshooting

### "LittleFS mount failed"
**Solution:** Upload filesystem first
- Arduino IDE: Tools → Pico LittleFS Data Upload
- PlatformIO: `pio run --target uploadfs`

### "No WAV files found"
**Solution:** Check WAV files are in `data/` folder and filesystem uploaded

### No sound / crackling
**Solution:** Check WAV format:
- Must be 16kHz, 16-bit, Mono
- Use Audacity to convert/verify

### File not found during playback
**Solution:** 
- File path is case-sensitive: `"/Sound1.wav" ≠ "/sound1.wav"`
- Check serial monitor for file list
- Verify file exists on LittleFS

## File Size Limits

LittleFS partition size is set in `platformio.ini`:
```ini
board_build.filesystem_size = 1m  ; 1 MB for files
```

With 1MB:
- ~60 seconds of audio at 16kHz/16-bit/Mono
- Adjust size if you need more storage
- If you are planning to play a lot of sound, please consider a `Pi Pico with 16MB of Flash Memory`

## Next Steps

- Try different sound effects
- Combine WAV files with synthesized sounds
- Add volume control
- Build a sound board!
