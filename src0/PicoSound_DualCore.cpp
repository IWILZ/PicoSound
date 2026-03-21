/*
 * PicoSound_DualCore.cpp
 * 
 * Implementazione funzioni helper dual-core
 */

#include "PicoSound_DualCore.h"

//=============================================================================
// SEND AUDIO COMMAND
//=============================================================================
void SendAudioCommand(AudioCommandType cmdType, uint8_t soundID, 
                      uint8_t volume, WaveType wave,
                      float freq_start, float freq_end, uint16_t duration,
                      const Note* notes, uint8_t numNotes, uint8_t channel) {
  // Aspetta che Core1 sia pronto a ricevere
  while (!AudioSem.canISendTo(1)) {
    delay(1);
  }
  
  // Prepara comando
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
/*  
  Serial.println("SendAudioCommand() before setDataReadyFor() - type="+String(cmdType)+" soundID="+String(soundID));
  if (cmdType==CMD_PLAY_CUSTOM)
    Serial.println("CMD_PLAY_CUSTOM --> volume="+String(volume)+ " wave="+String(wave)+ " freq_start="+String(freq_start)+ 
      " freq_end="+String(freq_end)+ " duration="+String(duration)+ " notes=[pointer] numNotes="+String(numNotes)+ 
      " channel="+String(channel));
*/
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
// PLAY TONE (compatibilità tone())
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
// STOP ONLY "soundID" sound 
//=============================================================================
void StopSound(uint8_t soundID) {
  // Trova quale canale sta suonando questo suono
  uint8_t channel = PicoSound.findChannelPlayingSound(soundID);
//  uint8_t channel = findChannelPlayingSound(soundID);
  
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
