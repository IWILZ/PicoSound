# API Reference

## High-Level API (PicoSound_DualCore.h)

Recommended for most users. Handles Core0→Core1 communication automatically.

### PlayTone()
```cpp
void PlayTone(int frequency, int duration, uint8_t volume = 80);
```
Play a simple tone.

**Parameters:**
- `frequency` - Frequency in Hz
- `duration` - Duration in milliseconds
- `volume` - Volume 0-100 (default: 80)

**Example:**
```cpp
PlayTone(440, 200);  // Play A4 for 200ms
```

---

### PlayMelody()
```cpp
void PlayMelody(const Note* notes, uint8_t numNotes, 
                WaveType wave = WAVE_SQUARE, uint8_t volume = 80);
```
Play a sequence of notes.

**Parameters:**
- `notes` - Array of Note structures
- `numNotes` - Number of notes in array
- `wave` - Waveform type (default: WAVE_SQUARE)
- `volume` - Volume 0-100 (default: 80)

**Example:**
```cpp
const Note melody[] = {{440, 200}, {523, 200}};
PlayMelody(melody, 2, WAVE_SINE, 70);
```

---

### SendAudioCommand()
```cpp
void SendAudioCommand(AudioCommandType cmdType, uint8_t soundID = 0, 
                      uint8_t volume = 80, ...);
```
Send command to Core1 audio engine.

**Common commands:**
- `CMD_PLAY_SOUND` - Play sound from table
- `CMD_STOP_ALL` - Stop all channels
- `CMD_SET_MASTER_VOLUME` - Change master volume

**Example:**
```cpp
SendAudioCommand(CMD_PLAY_SOUND, SND_EXPLOSION, 90);
```

---

### StopAllSounds()
```cpp
void StopAllSounds();
```
Stop all active audio channels.

---

### SetMasterVolume()
```cpp
void SetMasterVolume(uint8_t volume);
```
Set master volume (0-100).

---

## Core1 Setup (PicoSound_AudioCore.h)

### PicoSound_AudioCore_Setup1()
```cpp
void PicoSound_AudioCore_Setup1();
```
Initialize audio engine on Core1. Call once in `setup1()`.

---

### PicoSound_AudioCore_Loop1()
```cpp
void PicoSound_AudioCore_Loop1();
```
Audio mixer loop. Call continuously in `loop1()`.

---

## Low-Level API (PicoSound.h)

Advanced users only. Direct access to audio engine.

### PICOSOUND::playSound()
```cpp
bool playSound(uint8_t soundID, uint8_t volume);
```
Play sound from PICOSOUND_TABLE.

---

### PICOSOUND::playCustomTone()
```cpp
bool playCustomTone(float frequency, uint16_t duration, 
                    WaveType waveType, uint16_t amplitude, uint8_t volume);
```
Generate tone with custom parameters.

---

### PICOSOUND::playMelody()
```cpp
bool playMelody(const Note* notes, uint8_t numNotes, 
                WaveType waveType, uint8_t volume);
```
Play note sequence.

---

### PICOSOUND::stopAll()
```cpp
void stopAll();
```
Stop all channels.

---

### PICOSOUND::setMasterVolume()
```cpp
void setMasterVolume(uint8_t volume);
```
Set master volume.

---

## Data Types

### Note
```cpp
typedef struct {
  uint16_t frequency;  // Hz (0 = rest)
  uint16_t duration;   // milliseconds
} Note;
```

### WaveType
```cpp
typedef enum {
  WAVE_NONE,
  WAVE_SINE,
  WAVE_SQUARE,
  WAVE_SAWTOOTH,
  WAVE_TRIANGLE,
  WAVE_NOISE,
  WAVE_WAV,
  WAVE_EXPLOSION
} WaveType;
```

### AudioCommandType
```cpp
typedef enum {
  CMD_NONE,
  CMD_PLAY_SOUND,
  CMD_PLAY_CUSTOM,
  CMD_PLAY_SWEEP,
  CMD_PLAY_MELODY_CUSTOM,
  CMD_STOP_ALL,
  CMD_SET_MASTER_VOLUME
} AudioCommandType;
```

---

## Sound Definition

Sounds are defined in `picosound_user_cfg.h`:

```cpp
inline const SoundDefinition PICOSOUND_TABLE[] = {
  // Synthesized tone
  {WAVE_SQUARE, 440, 0, 200, 12000, 80, 
   nullptr, 0, false, nullptr, false},
   
  // WAV from LittleFS
  {WAVE_WAV, 0, 0, 0, 0, 70, 
   nullptr, 0, true, "/sound.wav", true},
   
  // WAV from PROGMEM
  {WAVE_WAV, 0, 0, 0, 0, 80, 
   sample_data, sample_length, false, nullptr, false},
};
```

**Parameters:**
1. Waveform type
2. Start frequency (Hz, 0 for WAV)
3. End frequency (Hz, for sweeps)
4. Duration (ms, 0 for WAV)
5. Amplitude
6. Default volume (0-100)
7. WAV data pointer (or nullptr)
8. WAV data length (or 0)
9. Loop flag
10. Filesystem path (or nullptr)
11. Use filesystem flag
