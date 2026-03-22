/*
 * PicoSound User Configuration
 * 
 * Customize pins and define your sounds here.
 */

#ifndef PICOSOUND_USR_CFG_H
#define PICOSOUND_USR_CFG_H

#include <PicoSound.h>

//=============================================================================
// HARDWARE CONFIGURATION
//=============================================================================
#define USER_SND_OUT    OUT_I2S
#define USER_PIN_BCK    14
#define USER_PIN_WS     15
#define USER_PIN_DATA   16
#define USER_PIN_SPKR   17

//=============================================================================
// MIXER SETTINGS
//=============================================================================
#define PICOSOUND_USER_NUM_CHANNELS     4
#define PICOSOUND_USER_MASTER_VOLUME   80

//=============================================================================
// LOUDNESS COMPENSATION
//=============================================================================
inline const float PICOSOUND_LOUDNESS_FACTOR[] = {
  1.0f,   // WAVE_NONE
  3.0f,   // WAVE_SINE
  0.9f,   // WAVE_SQUARE
  2.0f,   // WAVE_SAWTOOTH
  2.0f,   // WAVE_TRIANGLE
  0.7f,   // WAVE_NOISE
  1.0f,   // WAVE_WAV
  1.0f    // WAVE_EXPLOSION
};

//=============================================================================
// SOUND IDs
//=============================================================================
typedef enum {
  SND_NONE = 0,
  SND_BEEP,
  SND_LASER,
  SND_EXPLOSION,
  SND_MAX
} SoundID;

//=============================================================================
// SOUND DEFINITION TABLE
//=============================================================================
inline const SoundDefinition PICOSOUND_TABLE[] = {
  // SND_NONE
  {WAVE_NONE, 0, 0, 0, 0, 100, nullptr, 0, false, nullptr, false},
  
  // SND_BEEP - Simple 440Hz tone
  {WAVE_SQUARE, 440, 0, 100, 12000, 80, nullptr, 0, false, nullptr, false},
  
  // SND_LASER - Frequency sweep 1500→200Hz
  {WAVE_SQUARE, 1500, 200, 200, 12000, 70, nullptr, 0, false, nullptr, false},
  
  // SND_EXPLOSION - Realistic explosion
  {WAVE_EXPLOSION, 0, 0, 1800, 16000, 90, nullptr, 0, false, nullptr, false},
};

#endif
