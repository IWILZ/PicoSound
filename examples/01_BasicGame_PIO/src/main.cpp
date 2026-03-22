/*
 * PicoSound - Basic Example (PlatformIO)
 * 
 * Demonstrates PicoSound usage with separate config file.
 * Sound definitions are in include/picosound_user_cfg.h
 */

#include <Arduino.h>
#include <PicoSound_AudioCore.h>
#include <PicoSound_DualCore.h>

// Button pins
#define BTN_BEEP      18
#define BTN_EXPLOSION 19
#define BTN_MELODY    20

// Simple melody
const Note powerup[] = {
  {262, 100}, {330, 100}, {392, 100}, {523, 200}
};

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  // Setup buttons
  pinMode(BTN_BEEP, INPUT_PULLUP);
  pinMode(BTN_EXPLOSION, INPUT_PULLUP);
  pinMode(BTN_MELODY, INPUT_PULLUP);
  
  Serial.println("\n=== PicoSound Basic Example ===");
  Serial.println("Press buttons to play sounds:");
  Serial.println("  GP18 - Beep");
  Serial.println("  GP19 - Explosion");
  Serial.println("  GP20 - Melody");
}

void loop() {
  // Beep sound
  if (digitalRead(BTN_BEEP) == LOW) {
    Serial.println("Playing: Beep");
    SendAudioCommand(CMD_PLAY_SOUND, SND_BEEP, 80);
    delay(200);
  }
  
  // Explosion sound
  if (digitalRead(BTN_EXPLOSION) == LOW) {
    Serial.println("Playing: Explosion");
    SendAudioCommand(CMD_PLAY_SOUND, SND_EXPLOSION, 90);
    delay(200);
  }
  
  // Melody
  if (digitalRead(BTN_MELODY) == LOW) {
    Serial.println("Playing: Melody");
    PlayMelody(powerup, 4, WAVE_TRIANGLE, 70);
    delay(200);
  }
}

//=============================================================================
// CORE1: Audio engine
//=============================================================================

void setup1() {
  PicoSound_AudioCore_Setup1();
}

void loop1() {
  PicoSound_AudioCore_Loop1();
}
