/*
 * PicoSound - Audio Engine for Raspberry Pi Pico (RP2040)
 * 
 * Version: 1.0
 * Date: January 2026
 * Author: IWILZ
 * License: GPL-3.0
 * 
 * Features:
 * - Multi-channel mixer (configurable 1-8 channels)
 * - Dual-core architecture (Core0=Game, Core1=Audio)
 * - WAV streaming from LittleFS (unlimited size)
 * - Synthesized sounds (sine, square, sawtooth, triangle, noise)
 * - Custom explosion generator
 * - Melody sequencer
 * - Real-time frequency sweeps
 * - Automatic fade in/out
 * - I2S (MAX98357A) or PWM output
 * 
 * Dependencies:
 * - Arduino-Pico (Earle Philhower core)
 * - PicoSem library (for dual-core sync)
 * 
 * Usage:
 *   #include <PicoSound.h>
 *   #include <PicoSem.h>
 *   
 *   // In setup1(): PicoSound.init(&config);
 *   // In loop1():  PicoSound.loop();
 */

#ifndef PICOSOUND_H
#define PICOSOUND_H

#include <Arduino.h>
#include <I2S.h>
#include <LittleFS.h>

//=============================================================================
//=============================================================================
// picosound_user_cfg.h is included after struct and variables definition
//=============================================================================
//=============================================================================

//=============================================================================
// GLOBAL I2S INSTANCE accessible from outside (if needed)
// In this case you have to:
//    #include <I2S.h>
//    #include <PicoSound.h>
//=============================================================================
extern I2S g_i2s;

//=============================================================================
// PWM PARAMETERS
//=============================================================================
#ifndef PICOSOUND_PWM_CARRIER
  #define PICOSOUND_PWM_CARRIER 100000        // PWM carrier frequency (Hz)
#endif

//=============================================================================
// AUDIO PARAMETERS
//=============================================================================
#ifndef PICOSOUND_DEFAULT_SAMPLE_RATE
  #define PICOSOUND_DEFAULT_SAMPLE_RATE   16000   // Hz (16kHz for arcade)
#endif
#ifndef PICOSOUND_MAX_CHANNELS
  #define PICOSOUND_MAX_CHANNELS          8       // MAX Simultaneous channels
#endif
#ifndef PICOSOUND_DEFAULT_NUM_CHANNELS
  #define PICOSOUND_DEFAULT_NUM_CHANNELS  4       // Simultaneous channels
#endif
#ifndef PICOSOUND_DEFAULT_FADE_SAMPLES
//  #define PICOSOUND_DEFAULT_FADE_SAMPLES  800     // 50ms @ 16kHz
  #define PICOSOUND_DEFAULT_FADE_SAMPLES  640     // 40ms @ 16kHz
#endif
#ifndef PICOSOUND_DEFAULT_MASTER_VOLUME
  #define PICOSOUND_DEFAULT_MASTER_VOLUME 80      // 0-100
#endif

//=============================================================================
// ADVANCED SETTINGS
//=============================================================================
// WAV streaming buffer size (samples, not bytes)
#ifndef PICOSOUND_STREAM_BUFFER_SIZE
  #define PICOSOUND_STREAM_BUFFER_SIZE    512     // 1KB buffer
#endif

//=============================================================================
// ENUMERATIONS
//=============================================================================
/*
 * @brief Set the kind of output 
 */
typedef enum {
  OUT_I2S = 0,
  OUT_PWM = 1
} AudioOutMode;

/**
 * @brief Waveform types for synthesized sounds
 */
typedef enum {
  WAVE_NONE = 0,
  WAVE_SINE,        // Pure tone (smooth)
  WAVE_SQUARE,      // 8-bit retro sound
  WAVE_SAWTOOTH,    // Bright, sharp
  WAVE_TRIANGLE,    // Between sine and square
  WAVE_NOISE,       // White noise
  WAVE_WAV,         // WAV file playback
  WAVE_EXPLOSION    // Custom explosion effect
} WaveType;

/**
 * @brief Audio command types (Core0 → Core1)
 */
typedef enum {
  CMD_NONE = 0,
  CMD_PLAY_SOUND,           // Play sound from table
  CMD_STOP_CHANNEL,         // Stop specific channel
  CMD_STOP_ALL,             // Stop all channels
  CMD_SET_MASTER_VOLUME,    // Set global volume
  CMD_PLAY_CUSTOM,          // Play custom sound on-the-fly
  CMD_PLAY_MELODY_CUSTOM    // Play custom melody from Note array
} AudioCommandType;

//=============================================================================
// STRUCTURES
//=============================================================================
/**
 * @brief Configuration parameters (pass to PicoSound_init)
 */
typedef struct {
  // Hardware pins (I2S)
  uint8_t pinBCK;          // Bit clock pin
  uint8_t pinWS;           // Word select pin
  uint8_t pinData;         // Data pin
  uint8_t pinPWM;          // PWM pin
  AudioOutMode mode;       // PWM or I2S
  uint8_t numChannels;     // Number of mixer channels (default: 4)
  uint8_t masterVolume;    // Initial master volume 0-100 (default: 80)
} PicoSound_Config;

/**
 * @brief Musical note (for melodies)
 */
typedef struct {
  uint16_t frequency;  // Hz (0 = silence/rest)
  uint16_t duration;   // milliseconds
} Note;

/**
 * @brief Melody definition
 */
typedef struct {
  const Note* notes;
  uint8_t numNotes;
  WaveType waveType;
  uint8_t volume;
} Melody;

/**
 * @brief Sound definition (entry in sound table)
 */
typedef struct {
  WaveType type;
  float freq_start;
  float freq_end;
  uint16_t duration_ms;
  int16_t amplitude;
  uint8_t default_volume;
  const unsigned char* wavData;  // PROGMEM data
  uint32_t wavLength;            // bytes
  bool loop;
  const char* wavFilePath;       // LittleFS path
  bool useFS;                    // true=LittleFS, false=PROGMEM
} SoundDefinition;

// Sound table is defined in "picosound_cfg.h"
extern const SoundDefinition PICOSOUND_TABLE[];

/**
 * @brief Audio command (Core0 → Core1 via PicoSem)
 */
typedef struct {
  AudioCommandType type;
  uint8_t soundID;      // for CMD_PLAY_SOUND
  uint8_t channel;      // for CMD_STOP_CHANNEL
  uint8_t volume;       // 0-100 (0=use default)
  // Data for CMD_PLAY_MELODY_CUSTOM
  const Note* notes;    // Pointer to Note array in picosound_cfg.h
  uint8_t numNotes;     // Number of notes in array
  // For CMD_PLAY_CUSTOM
  WaveType waveType;
  float freq_start;
  float freq_end;
  uint16_t duration;
} AudioCommand;

/**
 * @brief Channel status (shared memory Core0 ← Core1)
 * READ-ONLY for Core0!
 */
typedef struct {
  bool active;           // Channel in use?
  uint8_t channelNumber; // Channel index (0-7)
  uint8_t soundID;       // Which sound is playing
  WaveType waveType;     // Waveform type
} ChannelStatus;

//=============================================================================
// INTERNAL STRUCTURES
//=============================================================================
// Internal audio channel state
typedef struct {
  bool enabled;
  WaveType waveType;
  // Synthesized sounds
  float frequency;
  int16_t amplitude;
  float phase;
  // Frequency sweep
  float freq_start;
  float freq_end;
  uint32_t sweep_duration_samples;
  uint32_t sweep_position;
  // WAV playback
  const int16_t* wavData;
  uint32_t wavLength;
  uint32_t wavPos;
  bool loop;
  // LittleFS streaming
  File wavFile;
  bool streamFromFS;
  uint32_t streamBufferPos;
  uint32_t streamBufferSize;
  int16_t streamBuffer[512];
  // Volume
  uint8_t perc_vol;
  // Duration control
  uint32_t total_duration_samples;
  uint32_t playback_position;
  // Melody sequencer
  const Note* melody;
  uint8_t melody_numNotes;
  uint8_t melody_currentNote;
  uint32_t melody_noteStartPos;
  // Explosion parameters
  uint32_t explosion_iteration;
  uint32_t explosion_max_iterations;
  uint16_t explosion_period;
  uint8_t explosion_repeat;
  uint8_t explosion_repeat_count;
} AudioChannel_Internal;


// ==================================================================
// PLATFORMIO is automatically defined if we are using PlatformIO IDE
// ==================================================================
#ifdef PLATFORMIO

//=============================================================================
// picosound_user_cfg.h must be included after struct and variables definition
//=============================================================================
#if defined(__has_include)
 #if __has_include("picosound_user_cfg.h")
   #include "picosound_user_cfg.h"
 #else
   #error "PicoSound: Copy templates/picosound_user_cfg_template.h to src/picosound_user_cfg.h or include/picosound_user_cfg.h"
 #endif
#else
 #include "picosound_user_cfg.h"
#endif

#endif // PLATFORMIO

// ==================================================================

#ifndef PLATFORMIO

    // Hardware configuration (set in sketch if needed)
    #ifndef USER_SND_OUT
      #define USER_SND_OUT    OUT_I2S
      #define USER_PIN_BCK    14
      #define USER_PIN_WS     15
      #define USER_PIN_DATA   16
      #define USER_PIN_SPKR   17
    #endif

    // Mixer settings (set in sketch if needed)
    #ifndef PICOSOUND_USER_NUM_CHANNELS
      #define PICOSOUND_USER_NUM_CHANNELS    4
    #endif

    #ifndef PICOSOUND_USER_MASTER_VOLUME
      #define PICOSOUND_USER_MASTER_VOLUME   80
    #endif

    // Loudness compensation (override in sketch if needed)
    #ifndef PICOSOUND_LOUDNESS_FACTOR
      inline const float PICOSOUND_LOUDNESS_FACTOR[] = {
        1.0f,   // WAVE_NONE
        2.0f,   // WAVE_SINE
        0.9f,   // WAVE_SQUARE
        1.5f,   // WAVE_SAWTOOTH
        1.5f,   // WAVE_TRIANGLE
        0.7f,   // WAVE_NOISE
        1.0f,   // WAVE_WAV
        1.0f    // WAVE_EXPLOSION
      };
    #endif

    //=============================================================================
    // SOUND IDs
    // User must define SoundID enum in picosound_user_cfg.h
    //=============================================================================
    // Example in picosound_user_cfg.h:
    //   typedef enum {
    //     SND_NONE = 0,
    //     SND_BEEP,
    //     SND_LASER,
    //     SND_MAX      // <- Always add SND_MAX as last entry
    //   } SoundID;

    // Ensure SND_NONE is always defined (fallback)
    #ifndef SND_NONE
      #define SND_NONE 0
    #endif

    // Ensure SND_MAX is defined (fallback to large value)
    #ifndef SND_MAX
      #define SND_MAX 255
    //  #warning "SND_MAX not defined in picosound_user_cfg.h. Using default 255."
    #endif

    // Sound table must be defined by user in sketch
    // See examples for reference
    extern const SoundDefinition PICOSOUND_TABLE[];

#endif    //  PLATFORMIO
// ==================================================================

//=============================================================================
// GLOBAL SHARED MEMORY (Core0 ← Core1)
//=============================================================================
inline ChannelStatus PicoSound_channelStatus[PICOSOUND_MAX_CHANNELS];  // Max number of mixer channels

class PICOSOUND
{
/*******************************************************************************/
/* ----------------------------------- PUBLIC ---------------------------------*/
/*******************************************************************************/
public:
//=============================================================================
// API - INITIALIZATION & MAIN LOOP
//=============================================================================
/**
 * @brief Initialize audio system (I2S, LittleFS, channels)
 * CALL THIS IN setup1() (Core1)
 * 
 * @param config Pointer to configuration struct (NULL = use defaults)
 * 
 * Example with defaults:
 *   PicoSound_init(NULL);
 * 
 * Example with custom config:
 *   PicoSound_Config cfg;
 *   cfg.mode = OUT_I2S;
 *   cfg.pinBCK = 14;
 *   cfg.pinWS = 15;
 *   cfg.pinData = 16;
 *   cfg.numChannels = 6;
 *   cfg.masterVolume = 80;
 *   PicoSound_init(&cfg);
 */
void init(const PicoSound_Config* config);

/**
 * @brief Audio main loop (mixer + I2S output)
 * CALL THIS CONTINUOUSLY IN loop1() (Core1)
 */
void loop();

//=============================================================================
// API - PLAYBACK CONTROL (called from Core1 after commands)
//=============================================================================
/**
 * @brief Play sound by ID from sound table
 * @param soundID Sound identifier (SND_LASER_SHOT, etc.)
 * @param volume Volume 0-100 (0=use default from table)
 * @return true if started successfully, false if no free channels
 */
bool playSound(uint8_t soundID, uint8_t volume);

/**
 * @brief Play sound on specific channel
 * @param soundID Sound identifier
 * @param channel Channel number (0-7)
 * @param volume Volume 0-100
 * @return true if started successfully
 */
bool playSoundOnChannel(uint8_t soundID, uint8_t channel, uint8_t volume);

/**
 * @brief Stop specific channel
 * @param channel Channel number (0-7)
 */
void stopChannel(uint8_t channel);

/**
 * @brief Stop all channels
 */
void stopAll();

/**
 * @brief Set master volume
 * @param vol Volume 0-100
 */
void setMasterVolume(uint8_t vol);

/**
 * @brief Get current master volume
 * @return Volume 0-100
 */
uint8_t getMasterVolume();

//=============================================================================
// API - CUSTOM SOUNDS (on-the-fly generation)
//=============================================================================
/**
 * @brief Play custom tone (single frequency)
 * @param type Waveform type (WAVE_SINE, WAVE_SQUARE, etc.)
 * @param frequency Frequency in Hz
 * @param duration_ms Duration in milliseconds
 * @param volume Volume 0-100
 * @return true if started successfully
 */
bool playCustomTone(WaveType type, float frequency, uint16_t duration_ms, uint8_t volume);

/**
 * @brief Play custom frequency sweep
 * @param type Waveform type
 * @param freq_start Starting frequency (Hz)
 * @param freq_end Ending frequency (Hz)
 * @param duration_ms Duration in milliseconds
 * @param volume Volume 0-100
 * @return true if started successfully
 */
bool playCustomSweep(WaveType type, float freq_start, float freq_end, uint16_t duration_ms, uint8_t volume);

/**
 * @brief Play custom noise burst
 * @param duration_ms Duration in milliseconds
 * @param volume Volume 0-100
 * @return true if started successfully
 */
bool playCustomNoise(uint16_t duration_ms, uint8_t volume);

/**
 * @brief Play custom explosion effect
 * @param duration_ms Duration in milliseconds
 * @param volume Volume 0-100
 * @return true if started successfully
 */
bool playCustomExplosion(uint16_t duration_ms, uint8_t volume);

/**
 * @brief Play melody (sequence of notes)
 * @param notes Array of notes
 * @param numNotes Number of notes in array
 * @param waveType Waveform to use
 * @param volume Volume 0-100
 * @return true if started successfully
 */
bool playMelody(const Note* notes, uint8_t numNotes, WaveType waveType, uint8_t volume);

//=============================================================================
// API - QUERY STATUS (safe for Core0 to call)
//=============================================================================
/**
 * @brief Get number of free channels
 * @return Number of available channels (0-8)
 */
uint8_t getFreeChannels();

/**
 * @brief Check if specific channel is active
 * @param channel Channel number (0-7)
 * @return true if channel is playing something
 */
bool isChannelActive(uint8_t channel);

/**
 * @brief Find which channel is playing a specific sound
 * @param soundID Sound identifier to search for
 * @return Channel number (0-7) or 255 if not found
 */
uint8_t findChannelPlayingSound(uint8_t soundID);

/**
 * @brief Check if a specific sound is currently playing
 * @param soundID Sound identifier
 * @return true if sound is playing on any channel
 */
bool isSoundPlaying(uint8_t soundID);

//=============================================================================
// API - COMMAND PROCESSING (Core1 internal, but exposed for PicoSem usage)
//=============================================================================
/**
 * @brief Process audio command from Core0
 * Called internally by loop() when command received via PicoSem
 * @param cmd Pointer to AudioCommand structure
 */
void processCommand(const AudioCommand* cmd);

/*******************************************************************************/
/* ----------------------------------- PRIVATE ------------------------------- */
/*******************************************************************************/
private:
static int findFreeChannel();
static void updateSharedStatus(int ch);
static void resetChannel(int ch);
static void initChannel(int ch, WaveType type, float freq, int16_t amp, uint8_t vol);
static void initChannelWAV(int ch, const unsigned char* wavArray, uint32_t lengthInBytes, uint8_t vol, bool loopEnable);
static void setupChannelSweep(int ch, float freq_start, float freq_end, uint32_t duration_ms);
static void setupChannelDuration(int ch, uint32_t duration_ms);
static void stopChannelInternal(int ch);
static bool openWAVStream(AudioChannel_Internal* ch, const char* path, bool loopEnable);
static int16_t readWAVStreamSample(AudioChannel_Internal* ch);
static void updateChannelSweep(AudioChannel_Internal* ch);
static void updateMelodySequencer(AudioChannel_Internal* ch);
static int16_t getChannelSample(AudioChannel_Internal* ch);
static int16_t mixChannels();

}; // class PICOSOUND

#endif // PICOSOUND_H
