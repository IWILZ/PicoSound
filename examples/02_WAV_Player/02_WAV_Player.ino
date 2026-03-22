/*
 * PicoSound - WAV Player Example (Arduino IDE)
 * 
 * Demonstrates playing WAV files from LittleFS filesystem.
 * 
 * SETUP:
 * 1. Upload WAV files to LittleFS using "Pico LittleFS Data Upload" tool
 * 2. WAV files must be: 16kHz, 16-bit, Mono
 * 3. Place files in /data/ folder before upload
 * 
 * HARDWARE:
 * - Button on GP18 (play sound1.wav)
 * - Button on GP19 (play sound2.wav)
 * - Button on GP20 (play sound3.wav)
 * - I2S audio output on GP14/15/16
 */

//=============================================================================
// PICOSOUND CONFIGURATION (before includes)
//=============================================================================

#define USER_SND_OUT    OUT_I2S
#define USER_PIN_BCK    14
#define USER_PIN_WS     15
#define USER_PIN_DATA   16

// Sound IDs for WAV files
typedef enum {
  SND_NONE = 0,
  SND_BEEP,         // Synthesized fallback
  SND_WAV_1,        // sound1.wav from LittleFS
  SND_WAV_2,        // sound2.wav from LittleFS
  SND_WAV_3,        // sound3.wav from LittleFS
  SND_MAX
} SoundID;

//=============================================================================
// INCLUDES
//=============================================================================
#include <Arduino.h>
#include <LittleFS.h>
#include <PicoSound_AudioCore.h>
#include <PicoSound_DualCore.h>

//=============================================================================
// SOUND TABLE
//=============================================================================
const SoundDefinition PICOSOUND_TABLE[] = {
  // SND_NONE
  {WAVE_NONE, 0, 0, 0, 0, 100, nullptr, 0, false, nullptr, false},
  
  // SND_BEEP - Fallback synthesized sound
  {WAVE_SQUARE, 440, 0, 100, 12000, 80, nullptr, 0, false, nullptr, false},
  
  // SND_WAV_1 - Play sound1.wav from LittleFS
  {WAVE_WAV, 0, 0, 0, 0, 80, nullptr, 0, false, "/sound1.wav", true},
  
  // SND_WAV_2 - Play sound2.wav from LittleFS
  {WAVE_WAV, 0, 0, 0, 0, 80, nullptr, 0, false, "/sound2.wav", true},
  
  // SND_WAV_3 - Play sound3.wav from LittleFS (looping)
  {WAVE_WAV, 0, 0, 0, 0, 70, nullptr, 0, true, "/sound3.wav", true},
};

//=============================================================================
// BUTTON PINS
//=============================================================================
#define BTN_SOUND1   18
#define BTN_SOUND2   19
#define BTN_SOUND3   20

//=============================================================================
// SETUP
//=============================================================================
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== PicoSound WAV Player Example ===");
  
  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("ERROR: LittleFS mount failed!");
    Serial.println("Did you upload WAV files using 'Pico LittleFS Data Upload'?");
    while (1) {
      delay(1000);
    }
  }
  
  Serial.println("LittleFS mounted successfully");
  
  // List files in root directory
  Serial.println("\nFiles on LittleFS:");
  Dir dir = LittleFS.openDir("/");
  bool foundWav = false;
  while (dir.next()) {
    Serial.print("  ");
    Serial.print(dir.fileName());
    Serial.print(" (");
    Serial.print(dir.fileSize());
    Serial.println(" bytes)");
    if (dir.fileName().endsWith(".wav")) {
      foundWav = true;
    }
  }
  
  if (!foundWav) {
    Serial.println("\nWARNING: No WAV files found!");
    Serial.println("Upload WAV files (16kHz, 16-bit, Mono) to /data/ folder");
    Serial.println("Then use 'Tools > Pico LittleFS Data Upload'");
  }
  
  // Setup buttons
  pinMode(BTN_SOUND1, INPUT_PULLUP);
  pinMode(BTN_SOUND2, INPUT_PULLUP);
  pinMode(BTN_SOUND3, INPUT_PULLUP);
  
  Serial.println("\nReady! Press buttons:");
  Serial.println("  GP18 - Play sound1.wav");
  Serial.println("  GP19 - Play sound2.wav");
  Serial.println("  GP20 - Play sound3.wav (looping)");
  Serial.println("  (Press GP20 again to stop loop)");
}

//=============================================================================
// MAIN LOOP
//=============================================================================
void loop() {
  // Button 1 - Play sound1.wav
  if (digitalRead(BTN_SOUND1) == LOW) {
    Serial.println("Playing: sound1.wav");
    SendAudioCommand(CMD_PLAY_SOUND, SND_WAV_1, 80);
    delay(300);
  }
  
  // Button 2 - Play sound2.wav
  if (digitalRead(BTN_SOUND2) == LOW) {
    Serial.println("Playing: sound2.wav");
    SendAudioCommand(CMD_PLAY_SOUND, SND_WAV_2, 80);
    delay(300);
  }
  
  // Button 3 - Toggle sound3.wav (looping)
  static bool looping = false;
  if (digitalRead(BTN_SOUND3) == LOW) {
    if (!looping) {
      Serial.println("Playing: sound3.wav (loop)");
      SendAudioCommand(CMD_PLAY_SOUND, SND_WAV_3, 70);
      looping = true;
    } else {
      Serial.println("Stopping loop");
      StopAllSounds();
      looping = false;
    }
    delay(300);
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
