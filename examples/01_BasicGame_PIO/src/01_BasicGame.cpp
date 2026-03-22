/*
 * PicoSound - Basic Example (PlaftformIO-Style)
 * Configure your hardware and sounds in picosound_user_cfg.h
 * 
 * This program uses 3 buttons to produce 3 different sounds
 */
#include <Arduino.h>
#include <PicoSound_AudioCore.h>
#include <PicoSound_DualCore.h>

// Button pins
#define BTN_BEEP      18
#define BTN_EXPLOSION 19
#define BTN_MELODY    20

// Enable next define if you are using an I2S module 
// (like a MAX98357A) controlled by a GPIO pin
// #define I2S_DS_PIN    0   // GPIO pin to mute/unmute MAX98357A

void setup() {
  delay(500);
  Serial.begin(115200);
  delay(2000);
  
  // Setup buttons
  pinMode(BTN_BEEP, INPUT_PULLUP);
  pinMode(BTN_EXPLOSION, INPUT_PULLUP);
  pinMode(BTN_MELODY, INPUT_PULLUP);

  if (USER_SND_OUT==OUT_I2S){
    #ifdef I2S_DS_PIN
    pinMode(I2S_DS_PIN, OUTPUT);      // MAX98357A DS control pin
    digitalWrite(I2S_DS_PIN, HIGH);   // enable MAX98357A
    #endif
  }

  Serial.println("PicoSound Basic Example");
  Serial.println("Press buttons to play sounds");
  Serial.println("  GP18 - Beep");
  Serial.println("  GP19 - Explosion");
  Serial.println("  GP20 - Melody");
}

void loop() {
  // Beep sound
  if (digitalRead(BTN_BEEP) == LOW) {
    SendAudioCommand(CMD_PLAY_SOUND, SND_BEEP, 80);
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

//=============================================================================
// CORE1: Audio engine
//=============================================================================
void setup1() {
  PicoSound_AudioCore_Setup1();
}

void loop1() {
  PicoSound_AudioCore_Loop1();
}
