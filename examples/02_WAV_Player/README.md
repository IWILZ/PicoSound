# PicoSound WAV Player Example

Play WAV files from LittleFS filesystem with button triggers.

## Hardware Required

- Raspberry Pi Pico
- 3 push buttons (GP18, GP19, GP20)
- MAX98357A I2S amplifier
- WAV files (16kHz, 16-bit, Mono)

## WAV File Requirements

**IMPORTANT:** WAV files must be:
- Sample rate: **16kHz**
- Bit depth: **16-bit**
- Channels: **Mono**

Convert your WAV files using Audacity:
1. File → Open your audio file
2. Tracks → Resample → 16000 Hz
3. Tracks → Mix → Mix Stereo Down to Mono (if stereo)
4. File → Export → Export as WAV
   - Encoding: Signed 16-bit PCM
5. Save as sound1.wav, sound2.wav, sound3.wav

## Setup (Arduino IDE)

### 1. Prepare WAV Files

Create a `data/` folder in your sketch directory:
```
02_WAV_Player/
├── 02_WAV_Player.ino
└── data/
    ├── sound1.wav  (16kHz, 16-bit, Mono)
    ├── sound2.wav  (16kHz, 16-bit, Mono)
    └── sound3.wav  (16kHz, 16-bit, Mono)
```

### 2. Upload Filesystem

To upload data/ folder to the Pico filesystem, use **Pico LittleFS Data Upload** or press CTRL-SHIFT-P and select **Upload LittleFS to Pico/ESP8266/ESP32**

### 3. Upload Sketch

Upload the sketch normally.

### 4. Test

Press buttons to play sounds!

## Setup (PlatformIO)

### 1. Prepare WAV Files

Place WAV files in `data/` folder:
```
02_WAV_Player_PIO/
├── platformio.ini
├── include/
├── src/
└── data/
    ├── sound1.wav
    ├── sound2.wav
    └── sound3.wav
```

### 2. Upload Filesystem

```bash
pio run --target uploadfs
```

### 3. Upload Program

```bash
pio run --target upload
```

### 4. Monitor Serial

```bash
pio device monitor
```

The program lists all files found on LittleFS.

## Hardware Setup

### Buttons
- GP18 → Button → GND (Sound 1)
- GP19 → Button → GND (Sound 2)
- GP20 → Button → GND (Sound 3 - looping)

### I2S Audio (MAX98357A)
- GP14 → BCK
- GP15 → WS (LRCLK)
- GP16 → DIN

## How It Works

### Sound Definitions

```cpp
// Play WAV from LittleFS (one-shot)
{WAVE_WAV, 0, 0, 0, 0, 80, nullptr, 0, false, "/sound1.wav", true}
                                                 ↑             ↑
                                                 |             |
                                          File path      Use filesystem

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

## Troubleshooting

### "LittleFS mount failed"
**Solution:** Upload filesystem first
- Arduino IDE: Tools → Pico LittleFS Data Upload
- PlatformIO: `pio run --target uploadfs`

### "No WAV files found"
**Solution:** Check WAV files are in data/ folder and filesystem uploaded

### No sound / crackling
**Solution:** Check WAV format:
- Must be 16kHz, 16-bit, Mono
- Use Audacity to convert/verify

### File not found during playback
**Solution:** 
- File path is case-sensitive: "/Sound1.wav" ≠ "/sound1.wav"
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

## Next Steps

- Try different sound effects
- Combine WAV files with synthesized sounds
- Add volume control
- Build a sound board!
