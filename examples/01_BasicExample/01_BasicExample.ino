/*
 * PicoSound - Basic Example (Arduino-Style)
 * 
 * Configuration is defined inline in this sketch.
 * This approach works best with Arduino IDE.
 */

//=============================================================================
// PICOSOUND CONFIGURATION (define BEFORE includes)
//=============================================================================

// Hardware pins
#define USER_SND_OUT    OUT_I2S
#define USER_PIN_BCK    14
#define USER_PIN_WS     15
#define USER_PIN_DATA   16

// Mixer settings
#define PICOSOUND_USER_NUM_CHANNELS    4
#define PICOSOUND_USER_MASTER_VOLUME   80

// Sound IDs for this project
typedef enum {
  SND_NONE = 0,
  SND_BEEP,
  SND_LASER,
  SND_EXPLOSION,
  SND_MAX
} SoundID;

//=============================================================================
// NOW include PicoSound
//=============================================================================
#include <Arduino.h>
#include <PicoSound_AudioCore.h>
#include <PicoSound_DualCore.h>

//=============================================================================
// SOUND DEFINITIONS
//=============================================================================

// Simple melody
const Note powerup[] = {
  {262, 100}, {330, 100}, {392, 100}, {523, 200}
};

// Sound table
const SoundDefinition PICOSOUND_TABLE[] = {
  // SND_NONE
  {WAVE_NONE, 0, 0, 0, 0, 100, nullptr, 0, false, nullptr, false},
  
  // SND_BEEP - Simple 440Hz tone
  {WAVE_SQUARE, 440, 0, 100, 12000, 80, nullptr, 0, false, nullptr, false},
  
  // SND_LASER - Frequency sweep 1500→200Hz
  {WAVE_SQUARE, 1500, 200, 200, 12000, 70, nullptr, 0, false, nullptr, false},
  
  // SND_EXPLOSION - Realistic explosion
  {WAVE_EXPLOSION, 0, 0, 1800, 16000, 90, nullptr, 0, false, nullptr, false},
};

//=============================================================================
// MAIN PROGRAM
//=============================================================================
// Button pins
#define BTN_BEEP      18
#define BTN_EXPLOSION 19
#define BTN_MELODY    20

// Enable next define if you are using an I2S module 
// (like a MAX98357A) controlled by a GPIO pin
// see: https://github.com/IWILZ/PicoSound/blob/main/docs/HARDWARE.md
// ----------------------------------------------------------------------------
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
