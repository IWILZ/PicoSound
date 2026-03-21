/*
 * PicoSound - Basic Example
 * 
 * Simple game with button-triggered sounds.
 * Demonstrates core PicoSound functionality.
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
  
  // Setup buttons
  pinMode(BTN_BEEP, INPUT_PULLUP);
  pinMode(BTN_EXPLOSION, INPUT_PULLUP);
  pinMode(BTN_MELODY, INPUT_PULLUP);
  
  Serial.println("PicoSound Basic Example");
  Serial.println("Press buttons to play sounds");
}

void loop() {
  // Beep sound
  if (digitalRead(BTN_BEEP) == LOW) {
    PlayTone(440, 100);
    delay(200);
  }
  
  // Explosion sound
  if (digitalRead(BTN_EXPLOSION) == LOW) {
    SendAudioCommand(CMD_PLAY_SOUND, SND_EXPLOSION, 90);
    delay(200);
  }
  
  // Melody
  if (digitalRead(BTN_MELODY) == LOW) {
    PlayMelody(powerup, 4, WAVE_TRIANGLE, 70);
    delay(200);
  }
}

// Core1: Audio engine
void setup1() {
  PicoSound_AudioCore_Setup1();
}

void loop1() {
  PicoSound_AudioCore_Loop1();
}
