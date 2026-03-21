/*
 * PicoSound - WAV Samples Template
 * 
 * Optional: Include WAV samples in PROGMEM for instant playback.
 * 
 * IMPORTANT: Use only 16kHz / 16-bit / Mono WAV files
 * 
 * Convert WAV to C array using:
 * https://guilhermerodrigues680.github.io/wav2c-online/
 * 
 * Then add to PICOSOUND_TABLE as:
 * {WAVE_WAV, 0, 0, 0, 0, volume, sample_data, sample_length, false, nullptr, false}
 */

#ifndef PICOSOUND_SAMPLES_H
#define PICOSOUND_SAMPLES_H

#include <Arduino.h>

//=============================================================================
// SAMPLE DATA (16kHz / 16-bit / Mono)
//=============================================================================

// Example: Small beep sound (replace with your own)
const int SAMPLE_BEEP_LEN = 160;  // 10ms @ 16kHz
const unsigned char SAMPLE_BEEP[] PROGMEM = {
  0, 0, 100, 50, 150, 100, 180, 120, 190, 130, 
  180, 120, 150, 100, 100, 50, 0, 0,
  // ... (truncated for template)
};

/*
 * Usage in picosound_user_cfg.h:
 * 
 * #include "picosound_samples.h"
 * 
 * // In PICOSOUND_TABLE:
 * {WAVE_WAV, 0, 0, 0, 0, 80, 
 *  SAMPLE_BEEP, SAMPLE_BEEP_LEN, false, nullptr, false},
 */

#endif
