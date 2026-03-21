/*
 * PicoSound_DualCore.cpp
 * 
 * Dual-core helper functions implementation
 */

#include "PicoSound_DualCore.h"

//=============================================================================
// SEND AUDIO COMMAND
//=============================================================================
void SendAudioCommand(AudioCommandType cmdType, uint8_t soundID, 
                      uint8_t volume, WaveType wave,
                      float freq_start, float freq_end, uint16_t duration,
                      const Note* notes, uint8_t numNotes, uint8_t channel) {
  // Wait for Core1 to be ready
  while (!AudioSem.canISendTo(1)) {
    delay(1);
  }
  
  // Prepare command
  audioCmd.type = cmdType;
  audioCmd.soundID = soundID;
  audioCmd.volume = volume;
  audioCmd.waveType = wave;
  audioCmd.freq_start = freq_start;
  audioCmd.freq_end = freq_end;
  audioCmd.duration = duration;
  audioCmd.notes = notes;
  audioCmd.numNotes = numNotes;
  audioCmd.channel = channel;
  
  // Send command to Core1
  AudioSem.setDataReadyFor(1);
}

//=============================================================================
// PLAY MELODY
//=============================================================================
void PlayMelody(const Note* notes, uint8_t numNotes, WaveType wave, uint8_t volume) {
  SendAudioCommand(CMD_PLAY_MELODY_CUSTOM, 0, volume, wave, 
                   0, 0, 0, notes, numNotes);
}

//=============================================================================
// PLAY TONE (Arduino tone() compatibility)
//=============================================================================
void PlayTone(int frequency, int duration, uint8_t volume) {
  SendAudioCommand(CMD_PLAY_CUSTOM, 0, volume, WAVE_SQUARE, 
                   frequency, 0, duration);
}

//=============================================================================
// STOP ALL SOUNDS
//=============================================================================
void StopAllSounds() {
  SendAudioCommand(CMD_STOP_ALL);
}

//=============================================================================
// STOP SPECIFIC SOUND BY ID
//=============================================================================
void StopSound(uint8_t soundID) {
  // Find which channel is playing this sound
  uint8_t channel = PicoSound.findChannelPlayingSound(soundID);
  
  if (channel != 255) {
    SendAudioCommand(CMD_STOP_CHANNEL, 0, 0, WAVE_NONE, 
                     0, 0, 0, nullptr, 0, channel);
  }
}

//=============================================================================
// SET MASTER VOLUME
//=============================================================================
void SetMasterVolume(uint8_t volume) {
  SendAudioCommand(CMD_SET_MASTER_VOLUME, 0, volume);
}
