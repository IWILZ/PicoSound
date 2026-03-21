/*
 * AudioCore.cpp
 * 
 * Implementazione Setup e Loop per Core1 (Audio Engine)
 * Condiviso tra tutti i giochi e il Multigame
 */

#include "PicoSound_AudioCore.h"
#include "Gen_IO.h"
#include "Gen_DEBUG_define.h"

//=============================================================================
// DEFINIZIONE VARIABILI GLOBALI AUDIO
//=============================================================================
#define PICO_EMPTY true
PicoSem AudioSem(PICO_EMPTY, PICO_EMPTY);
AudioCommand audioCmd;
PICOSOUND PicoSound;

//=============================================================================
// SETUP CORE1
//=============================================================================
void PicoSound_AudioCore_Setup1() {

#ifdef DEBUG  
  delay(1000);
  unsigned long msec = millis();
  while (millis()-msec < 5000 && !Serial) {delay(10);}    // wait for Serial ready for 5" max
  msec=millis()-msec;
  delay(100);
  Serial.println("\n[Core1] started in "+String(msec+1000)+" msec");
#else
  delay(500);
#endif  

  // Initialize LittleFS for WAV files
  if (!LittleFS.begin()) {
    Serial.println("[Core1] WARNING: LittleFS mount failed!");
    Serial.println("[Core1] WAV files will not be available");
  } else {
    Serial.println("[Core1] LittleFS mounted successfully");
  }
  
  // Configure PicoSound
  PicoSound_Config cfg;
 // cfg.mode = OUT_PWM;           // PWM output (not I2S)
 // cfg.pinPWM = SPKR;            // Pin defined in Gen_IO.h
 // cfg.numChannels = 4;          // 4 simultaneous sounds
 // cfg.masterVolume = 80;        // Master volume 0-100

  cfg.mode=USER_SND_OUT;
  if (cfg.mode==OUT_I2S){            //  
    cfg.pinWS=USER_PIN_WS;           // Word select pin
    cfg.pinData=USER_PIN_DATA;       // Data pin
    cfg.pinBCK=USER_PIN_BCK;         // Bit clock pin
    cfg.masterVolume = PICOSOUND_USER_MASTER_VOLUME;           // 0-100
  }
  else{
    cfg.pinPWM = USER_PIN_SPKR;      // Pin PWM
    cfg.masterVolume = 80;           // 0-100
  }
  cfg.numChannels = PICOSOUND_USER_NUM_CHANNELS;  // NUMBER OF simultaneous sounds

  // Initialize audio engine
  Serial.println("[Core1] Initializing PicoSound...");
  PicoSound.init(&cfg);
  
  // Signal Core0 that audio system is ready
  AudioSem.setDataEmptyFor(1);
  
  Serial.println("[Core1] PicoSound ready!");
}

//=============================================================================
// LOOP CORE1
//=============================================================================
void PicoSound_AudioCore_Loop1() {
  // Check if Core0 sent an audio command
  if (AudioSem.anyDataFor(1)) {
    // Process the command
    PicoSound.processCommand(&audioCmd);
    // Signal that we processed it
    AudioSem.setDataReadBy(1);
  }
  // Run the audio mixer (generates samples and outputs to PWM)
  PicoSound.loop();
}
