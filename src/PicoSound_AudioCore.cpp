/*
 * PicoSound_AudioCore.cpp
 * 
 * Core1 setup and loop for PicoSound audio engine
 * Shared across all projects using PicoSound
 */

#include "PicoSound_AudioCore.h"

//=============================================================================
// GLOBAL AUDIO VARIABLES (shared between Core0 and Core1)
//=============================================================================
#define PICO_EMPTY true
PicoSem AudioSem(PICO_EMPTY, PICO_EMPTY);
AudioCommand audioCmd;
PICOSOUND PicoSound;

//=============================================================================
// CORE1 SETUP
//=============================================================================
void PicoSound_AudioCore_Setup1() {
  delay(1000);
  
  // Initialize LittleFS for WAV files
  if (!LittleFS.begin()) {
    Serial.println("[Core1] WARNING: LittleFS mount failed!");
    Serial.println("[Core1] WAV files will not be available");
  }
  
  // Configure PicoSound
  PicoSound_Config cfg;
  cfg.mode = USER_SND_OUT;
  
  if (cfg.mode == OUT_I2S) {
    cfg.pinBCK = USER_PIN_BCK;
    cfg.pinWS = USER_PIN_WS;
    cfg.pinData = USER_PIN_DATA;
  } else {
    cfg.pinPWM = USER_PIN_SPKR;
  }
  
  cfg.numChannels = PICOSOUND_USER_NUM_CHANNELS;
  cfg.masterVolume = PICOSOUND_USER_MASTER_VOLUME;
  
  // Initialize audio engine
  PicoSound.init(&cfg);
  
  // Signal Core0 that audio system is ready
  AudioSem.setDataEmptyFor(1);
  
  Serial.println("[Core1] PicoSound ready");
}

//=============================================================================
// CORE1 LOOP
//=============================================================================
void PicoSound_AudioCore_Loop1() {
  // Check if Core0 sent an audio command
  if (AudioSem.anyDataFor(1)) {
    PicoSound.processCommand(&audioCmd);
    AudioSem.setDataReadBy(1);
  }
  
  // Run audio mixer
  PicoSound.loop();
}
