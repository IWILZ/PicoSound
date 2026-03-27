/*
 * PicoSound.cpp - Audio Engine Implementation
 * 
 * Version: 1.0
 * Date: January 2026
 * Author: IWILZ
 * License: GPL-3.0
 */
  
#include <Arduino.h>
#include "PicoSound.h"

//=============================================================================
// GLOBAL VARIABLES
//=============================================================================
// Configuration (set via PicoSound_init)
static PicoSound_Config g_config;
static bool g_initialized = false;
// Audio channels
static AudioChannel_Internal g_channels[PICOSOUND_MAX_CHANNELS];      // Max channels
static uint8_t g_activeNumChannels = PICOSOUND_DEFAULT_NUM_CHANNELS;  // Actual number in use
// Master volume
static uint8_t g_masterVolume = 80;
// I2S interface accessible from outside
I2S g_i2s(OUTPUT);


//=============================================================================
// INTERNAL HELPERS
//=============================================================================
int PICOSOUND::findFreeChannel() {
  for (int i = 0; i < g_activeNumChannels; i++) {
    if (!g_channels[i].enabled) return i;
  }
  return -1;
}
//=============================================================================
void PICOSOUND::updateSharedStatus(int ch) {
  if (ch >= 0 && ch < g_activeNumChannels) {
    PicoSound_channelStatus[ch].active = g_channels[ch].enabled;
    PicoSound_channelStatus[ch].channelNumber = ch;
    // soundID is set externally when sound starts
    PicoSound_channelStatus[ch].waveType = g_channels[ch].waveType;
  }
}
//=============================================================================
void PICOSOUND::resetChannel(int ch) {
  // Disable immediately for safety during reset
  g_channels[ch].enabled = false;
  // Base fields
  g_channels[ch].waveType = WAVE_NONE;
  g_channels[ch].perc_vol = 0;
  g_channels[ch].frequency = 0;
  g_channels[ch].amplitude = 0;
  g_channels[ch].phase = 0.0f;
  // WAV / PROGMEM fields
  g_channels[ch].wavData = nullptr;
  g_channels[ch].wavLength = 0;
  g_channels[ch].wavPos = 0;
  g_channels[ch].loop = false;
  g_channels[ch].streamFromFS = false;
  // Sweep / Envelope fields
  g_channels[ch].freq_start = 0;
  g_channels[ch].freq_end = 0;
  g_channels[ch].sweep_duration_samples = 0;
  g_channels[ch].sweep_position = 0;
  g_channels[ch].total_duration_samples = 0;
  g_channels[ch].playback_position = 0;
  // Melody fields
  g_channels[ch].melody = nullptr;
  g_channels[ch].melody_numNotes = 0;
  g_channels[ch].melody_currentNote = 0;
  g_channels[ch].melody_noteStartPos = 0;
  // Special FX fields
  g_channels[ch].explosion_iteration = 0;
  g_channels[ch].explosion_max_iterations = 0;
  // Shared status
  updateSharedStatus(ch);
}
//=============================================================================
void PICOSOUND::initChannel(int ch, WaveType type, float freq, int16_t amp, uint8_t vol) {
  resetChannel(ch); // Clean everything before starting
  g_channels[ch].enabled = true;
  g_channels[ch].waveType = type;
  g_channels[ch].frequency = freq;
  g_channels[ch].freq_start = freq;
  g_channels[ch].amplitude = amp;
  g_channels[ch].perc_vol = vol;
}
//=============================================================================
void PICOSOUND::initChannelWAV(int ch, const unsigned char* wavArray, uint32_t lengthInBytes, uint8_t vol, bool loopEnable) {
  resetChannel(ch); // Clean everything before starting
  g_channels[ch].enabled = true;
  g_channels[ch].waveType = WAVE_WAV;
  g_channels[ch].wavData = (const int16_t*)wavArray;
  g_channels[ch].wavLength = lengthInBytes / 2;
  g_channels[ch].perc_vol = vol;
  g_channels[ch].loop = loopEnable;
  g_channels[ch].streamFromFS = false;
}
//=============================================================================
void PICOSOUND::setupChannelSweep(int ch, float freq_start, float freq_end, uint32_t duration_ms) {
  g_channels[ch].freq_start = freq_start;
  g_channels[ch].freq_end = freq_end;
  g_channels[ch].frequency = freq_start;
  g_channels[ch].sweep_duration_samples = (duration_ms * PICOSOUND_DEFAULT_SAMPLE_RATE) / 1000;
  g_channels[ch].sweep_position = 0;
}
//=============================================================================
void PICOSOUND::setupChannelDuration(int ch, uint32_t duration_ms) {
  g_channels[ch].total_duration_samples = (duration_ms * PICOSOUND_DEFAULT_SAMPLE_RATE) / 1000;
  g_channels[ch].playback_position = 0;
}
//=============================================================================
void PICOSOUND::stopChannelInternal(int ch) {
  if (g_channels[ch].streamFromFS && g_channels[ch].wavFile) {
    g_channels[ch].wavFile.close();
  }
  g_channels[ch].enabled = false;
  g_channels[ch].streamFromFS = false;
  g_channels[ch].total_duration_samples = 0;
  g_channels[ch].playback_position = 0;
  g_channels[ch].sweep_position = 0;
  g_channels[ch].wavPos = 0;
  g_channels[ch].melody = nullptr;
  g_channels[ch].melody_numNotes = 0;
  g_channels[ch].melody_currentNote = 0;
  g_channels[ch].melody_noteStartPos = 0;
  PicoSound_channelStatus[ch].active = false;
  PicoSound_channelStatus[ch].soundID = SND_NONE;
}

//=============================================================================
// LITTLEFS STREAMING
//=============================================================================
bool PICOSOUND::openWAVStream(AudioChannel_Internal* ch, const char* path, bool loopEnable) {
  ch->wavFile = LittleFS.open(path, "r");
  if (!ch->wavFile) {
    Serial.print("[PicoSound] ERROR: File not found: ");
    Serial.println(path);
    return false;
  }
  uint32_t fileSize = ch->wavFile.size();
#ifdef PICOSOUD_DEBUG  
  Serial.print("[PicoSound] Opening WAV stream: ");
  Serial.print(path);
  Serial.print(" (");
  Serial.print(fileSize);
  Serial.println(" bytes)");
#endif  
  ch->wavFile.seek(44);  // Skip WAV header
  ch->streamFromFS = true;
  ch->wavLength = (fileSize - 44) / 2;
  ch->wavPos = 0;
  ch->streamBufferPos = 0;
  ch->streamBufferSize = 0;
  ch->loop = loopEnable;
#ifdef PICOSOUD_DEBUG  
  Serial.print("[PicoSound] Stream ready: ");
  Serial.print(ch->wavLength);
  Serial.print(" samples (");
  Serial.print((float)ch->wavLength / PICOSOUND_DEFAULT_SAMPLE_RATE);
  Serial.println(" sec)");
#endif  
  return true;
}
//=============================================================================
int16_t PICOSOUND::readWAVStreamSample(AudioChannel_Internal* ch) {
  if (ch->streamBufferPos >= ch->streamBufferSize) {
    uint32_t bytesToRead = sizeof(ch->streamBuffer);
    uint32_t bytesRead = ch->wavFile.read((uint8_t*)ch->streamBuffer, bytesToRead);
    ch->streamBufferSize = bytesRead / 2;
    ch->streamBufferPos = 0;
    if (ch->streamBufferSize == 0) {
      if (ch->loop) {
        ch->wavFile.seek(44);
        ch->wavPos = 0;
        bytesRead = ch->wavFile.read((uint8_t*)ch->streamBuffer, bytesToRead);
        ch->streamBufferSize = bytesRead / 2;
        ch->streamBufferPos = 0;
      } else {
        ch->wavFile.close();
        ch->enabled = false;
        ch->streamFromFS = false;
        return 0;
      }
    }
  }
  int16_t sample = ch->streamBuffer[ch->streamBufferPos];
  ch->streamBufferPos++;
  ch->wavPos++;
  return sample;
}

//=============================================================================
// SOUND GENERATION
//=============================================================================
void PICOSOUND::updateChannelSweep(AudioChannel_Internal* ch) {
  if (ch->freq_end == 0 || ch->sweep_duration_samples == 0) return;
  if (ch->sweep_position < ch->sweep_duration_samples) {
    float progress = (float)ch->sweep_position / (float)ch->sweep_duration_samples;
    ch->frequency = ch->freq_start + (ch->freq_end - ch->freq_start) * progress;
    ch->sweep_position++;
  } else {
    ch->frequency = ch->freq_end;
  }
}
//=============================================================================
void PICOSOUND::updateMelodySequencer(AudioChannel_Internal* ch) {
  if (ch->melody == nullptr) return;
  uint32_t elapsed_ms = (ch->playback_position - ch->melody_noteStartPos) * 1000 / PICOSOUND_DEFAULT_SAMPLE_RATE;
  Note currentNote = ch->melody[ch->melody_currentNote];
  if (elapsed_ms >= currentNote.duration) {
    ch->melody_currentNote++;
    if (ch->melody_currentNote >= ch->melody_numNotes) {
      ch->enabled = false;
      ch->melody = nullptr;
      ch->melody_numNotes = 0;
      ch->melody_currentNote = 0;
      ch->melody_noteStartPos = 0;
      return;
    }
    Note nextNote = ch->melody[ch->melody_currentNote];
    ch->frequency = nextNote.frequency;
    ch->phase = 0;
    ch->melody_noteStartPos = ch->playback_position;
  }
}
//=============================================================================
int16_t PICOSOUND::getChannelSample(AudioChannel_Internal* ch) {
  if (!ch->enabled) return 0;
  if (ch->total_duration_samples > 0) {
    if (ch->playback_position >= ch->total_duration_samples) {
      ch->enabled = false;
      return 0;
    }
    ch->playback_position++;
  }
  int16_t sample = 0;
  if (ch->melody != nullptr) {
    updateMelodySequencer(ch);
  }
  if (ch->waveType != WAVE_WAV && ch->waveType != WAVE_NOISE && ch->waveType != WAVE_EXPLOSION) {
    updateChannelSweep(ch);
  }
  switch(ch->waveType) {
    case WAVE_NONE:
      sample = 0;
      break;
    case WAVE_SINE:
      sample = (int16_t)(ch->amplitude * sin(2.0f * PI * ch->phase));
      ch->phase += ch->frequency / PICOSOUND_DEFAULT_SAMPLE_RATE;
      if (ch->phase >= 1.0f) ch->phase -= 1.0f;
      break;
    case WAVE_SQUARE:
      sample = (ch->phase < 0.5f) ? ch->amplitude : -ch->amplitude;
      ch->phase += ch->frequency / PICOSOUND_DEFAULT_SAMPLE_RATE;
      if (ch->phase >= 1.0f) ch->phase -= 1.0f;
      break;
    case WAVE_SAWTOOTH:
      sample = (int16_t)(ch->amplitude * (2.0f * ch->phase - 1.0f));
      ch->phase += ch->frequency / PICOSOUND_DEFAULT_SAMPLE_RATE;
      if (ch->phase >= 1.0f) ch->phase -= 1.0f;
      break;
    case WAVE_TRIANGLE:
      if (ch->phase < 0.5f)
        sample = (int16_t)(ch->amplitude * (4.0f * ch->phase - 1.0f));
      else
        sample = (int16_t)(ch->amplitude * (3.0f - 4.0f * ch->phase));
      ch->phase += ch->frequency / PICOSOUND_DEFAULT_SAMPLE_RATE;
      if (ch->phase >= 1.0f) ch->phase -= 1.0f;
      break;
    case WAVE_NOISE:
//      noInterrupts();  // Protect random()
      sample = (int16_t)random(-ch->amplitude, ch->amplitude);
//      interrupts();
      break;
    case WAVE_EXPLOSION:
      {
        if (ch->explosion_max_iterations == 0) {
          ch->explosion_max_iterations = 150;
          ch->explosion_iteration = 0;
          ch->explosion_repeat_count = 0;
          ch->explosion_repeat = 0;
          ch->phase = 0.0f;
        }
        sample = 0;
        if (ch->explosion_repeat_count < ch->explosion_repeat) {
          uint32_t k = ch->explosion_iteration;
          uint32_t base_period_us = 1000 + (k * 24);
          int32_t impulse_duration_us = (base_period_us / 4);
          if (k > 0) impulse_duration_us += (k - 1) * 12;
          uint32_t impulse_samples = (impulse_duration_us * 16) / 1000;
          if (impulse_samples < 2) impulse_samples = 2;
          uint32_t pos = (uint32_t)ch->phase;
          if (pos < impulse_samples * 2) {
            if (pos < impulse_samples) {
              sample = ch->amplitude;
            } else {
              sample = -ch->amplitude;
            }
          } else {
            ch->explosion_repeat_count++;
            ch->phase = 0.0f;
          }
          ch->phase += 1.0f;
        } else {
          ch->explosion_iteration++;
          if (ch->explosion_iteration >= ch->explosion_max_iterations) {
            ch->enabled = false;
            break;
          }
          ch->explosion_repeat = random(3) * 3;
          ch->explosion_repeat_count = 0;
          ch->phase = 0.0f;
        }
        float global_fade = 1.0f - ((float)ch->explosion_iteration / (float)ch->explosion_max_iterations);
        sample = (int16_t)(sample * global_fade);
      }
      break;
    case WAVE_WAV:
      if (ch->streamFromFS) {
        sample = readWAVStreamSample(ch);
        if (ch->wavPos < PICOSOUND_DEFAULT_FADE_SAMPLES) {
          float fadeIn = (float)ch->wavPos / PICOSOUND_DEFAULT_FADE_SAMPLES;
          sample = (int16_t)(sample * fadeIn);
        }
        if (ch->wavPos > ch->wavLength - PICOSOUND_DEFAULT_FADE_SAMPLES) {
          float fadeOut = (float)(ch->wavLength - ch->wavPos) / PICOSOUND_DEFAULT_FADE_SAMPLES;
          sample = (int16_t)(sample * fadeOut);
        }
      } else if (ch->wavData != nullptr && ch->wavPos < ch->wavLength) {
        uint32_t bytePos = ch->wavPos * 2;
        uint8_t lowByte = pgm_read_byte((const uint8_t*)ch->wavData + bytePos);
        uint8_t highByte = pgm_read_byte((const uint8_t*)ch->wavData + bytePos + 1);
        sample = (int16_t)(lowByte | (highByte << 8));
        if (ch->wavPos < PICOSOUND_DEFAULT_FADE_SAMPLES) {
          float fadeIn = (float)ch->wavPos / PICOSOUND_DEFAULT_FADE_SAMPLES;
          sample = (int16_t)(sample * fadeIn);
        }
        if (ch->wavPos > ch->wavLength - PICOSOUND_DEFAULT_FADE_SAMPLES) {
          float fadeOut = (float)(ch->wavLength - ch->wavPos) / PICOSOUND_DEFAULT_FADE_SAMPLES;
          sample = (int16_t)(sample * fadeOut);
        }
        ch->wavPos++;
        if (ch->wavPos >= ch->wavLength) {
          if (ch->loop) {
            ch->wavPos = 0;
          } else {
            ch->enabled = false;
          }
        }
      }
      break;
  }
  float loudnessFactor = PICOSOUND_LOUDNESS_FACTOR[ch->waveType];
  sample = (int16_t)(sample * loudnessFactor);
  sample = (sample * ch->perc_vol) / 100;
  return sample;
}

//=============================================================================
int16_t PICOSOUND::mixChannels() {
  int32_t mix = 0;
  int activeChannels = 0;
  for (int i = 0; i < g_activeNumChannels; i++) {
    if (g_channels[i].enabled && g_channels[i].perc_vol > 0) {
      mix += getChannelSample(&g_channels[i]);
      activeChannels++;
      if (!g_channels[i].enabled) {
        updateSharedStatus(i);
      }
    }
  }
  if (activeChannels > 0) {
    mix = mix / activeChannels;
  }
  mix = (mix * g_masterVolume) / 100;
  if (mix > 32767) mix = 32767;
  if (mix < -32768) mix = -32768;
  return (int16_t)mix;
}

//=============================================================================
// PUBLIC API - INITIALIZATION
//=============================================================================
void PICOSOUND::init(const PicoSound_Config* config) {
  if (config == nullptr) {
    // Use defaults from picosound_cfg.h
    g_config.pinBCK = USER_PIN_BCK;
    g_config.pinWS = USER_PIN_WS;
    g_config.pinData = USER_PIN_DATA;
#ifdef PICOSOUND_USER_NUM_CHANNELS
    g_config.numChannels = PICOSOUND_USER_NUM_CHANNELS; 
#else    
    g_config.numChannels = PICOSOUND_DEFAULT_NUM_CHANNELS; 
#endif
    g_config.masterVolume = PICOSOUND_DEFAULT_MASTER_VOLUME;
    g_config.mode = OUT_I2S;
  } else {
    g_config = *config;
  }
  // Limit check: not less than 1, not more than 8
  g_activeNumChannels = g_config.numChannels;
  if (g_activeNumChannels > PICOSOUND_MAX_CHANNELS) {
    Serial.println("[PicoSound] Warning: Using max channels: "+String(PICOSOUND_MAX_CHANNELS));
    g_activeNumChannels = PICOSOUND_MAX_CHANNELS;
  } else if (g_activeNumChannels == 0) {
     g_activeNumChannels = 1; 
  }
  // Sample rate locked at 16kHz
  const uint16_t fixedSampleRate = PICOSOUND_DEFAULT_SAMPLE_RATE; 
  g_masterVolume = g_config.masterVolume;
  // Initialize active channels
  for (int i = 0; i < PICOSOUND_MAX_CHANNELS; i++) {
    // Reset ALL channels in physical array for safety
    g_channels[i].enabled = false;
    PicoSound_channelStatus[i].active = false;
    PicoSound_channelStatus[i].channelNumber = i;
    PicoSound_channelStatus[i].soundID = SND_NONE;
    PicoSound_channelStatus[i].waveType = WAVE_NONE;
    // Clean melody pointers for safety
    g_channels[i].melody = nullptr;
    g_channels[i].melody_numNotes = 0;
  }
  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("[PicoSound] Warning: LittleFS mount failed!");
  } else {
    Serial.println("[PicoSound] LittleFS mounted.");
  }
  if (g_config.mode == OUT_PWM) {
    pinMode(g_config.pinPWM, OUTPUT);
    // Set very high frequency to shift noise out of audible range
    analogWriteFreq(PICOSOUND_PWM_CARRIER); 
    analogWriteResolution(10); // 10 bit = 0-1023
    analogWrite(g_config.pinPWM, 512);        // anti "bump" for PWM output
    Serial.printf("[PicoSound] PWM Mode initialized on pin %d\n", g_config.pinPWM);
  } else {
    // I2S initialization
    g_i2s.setBCLK(g_config.pinBCK);
    g_i2s.setDATA(g_config.pinData);
    if (!g_i2s.begin(fixedSampleRate)) {
      Serial.println("[PicoSound] I2S init failed! -> program stop");
      while(1); 
    }
  }

  // Here we try to prevent corruption that WAVE_NOISE could make
  randomSeed(analogRead(A3) + micros());      
  uint32_t seed = rp2040.hwrand32();
  randomSeed(seed);

  Serial.println("[PicoSound] Init done with:");
  if (g_config.mode == OUT_PWM)
    Serial.println("    PWM on pin"+String(g_config.pinPWM));
  else{
    Serial.println("    I2S address:"+String((uint32_t)&g_i2s));  
    Serial.println("    I2S BCLK="+String(g_config.pinBCK)+" DIN="+String(g_config.pinData));
  }
  Serial.println("    N.channels="+String(g_config.numChannels)+" - Master volume="+String(g_config.masterVolume));
  g_initialized = true;
}

//=============================================================================
void PICOSOUND::loop() {
  if (!g_initialized) return;
  // sample = next sample from mixer
  int16_t sample = mixChannels();
  if (g_config.mode == OUT_PWM) {           // output on PWM pin
      // 10-bit output (0-1023)
      uint32_t pwmVal = ((int32_t)sample + 32768) >> 6;
      analogWrite(g_config.pinPWM, (int)pwmVal);
      // To syncronize the output we have to slow down
      // for a while between 2 samples 
      static uint32_t last_sample_time = 0;
      const uint32_t period = PICOSOUND_PWM_CARRIER*10 / PICOSOUND_DEFAULT_SAMPLE_RATE ; // 62us
      // Active wait to maintain constant sample rate
      while (micros() - last_sample_time < period) {
          tight_loop_contents(); 
      }
      last_sample_time = micros();
  } else {                  // output on I2S bus
      g_i2s.write((int32_t)sample, true);
  }
}

//=============================================================================
// PUBLIC API - PLAYBACK
//=============================================================================
bool PICOSOUND::playSound(uint8_t soundID, uint8_t volume) {
  if (soundID >= SND_MAX) return false;
  int ch = findFreeChannel();
  if (ch < 0){
    Serial.println("[PicoSound] Warning: No free channels for a new sound!");
    return false;
  } 
  return playSoundOnChannel(soundID, ch, volume);
}
//=============================================================================
bool PICOSOUND::playSoundOnChannel(uint8_t soundID, uint8_t channel, uint8_t volume) {
  if (soundID >= SND_MAX || channel >= g_activeNumChannels) return false;
  SoundDefinition snd = PICOSOUND_TABLE[soundID];
  uint8_t vol = (volume == 0) ? snd.default_volume : volume;
  if (snd.type == WAVE_WAV) {
    if (snd.useFS && snd.wavFilePath != nullptr) {
      // 1. Complete reset before opening file
      resetChannel(channel); 
      // 2. Specific configuration for filesystem
      g_channels[channel].enabled = true;
      g_channels[channel].waveType = WAVE_WAV;
      g_channels[channel].perc_vol = vol;
      if (!openWAVStream(&g_channels[channel], snd.wavFilePath, snd.loop)) {
        g_channels[channel].enabled = false; // Open failed
        return false;
      }
    } else {
      // Use simplified initChannelWAV (already calls resetChannel)
      initChannelWAV(channel, snd.wavData, snd.wavLength, vol, snd.loop);
    }
  } else {
    // Use simplified initChannel (already calls resetChannel)
    initChannel(channel, snd.type, snd.freq_start, snd.amplitude, vol);
    if (snd.duration_ms > 0) setupChannelDuration(channel, snd.duration_ms);
    if (snd.freq_end != 0) setupChannelSweep(channel, snd.freq_start, snd.freq_end, snd.duration_ms);
  }
  PicoSound_channelStatus[channel].soundID = soundID;
  updateSharedStatus(channel);
  return true;
}

//=============================================================================
void PICOSOUND::stopChannel(uint8_t channel) {
  if (channel < g_activeNumChannels) {
    stopChannelInternal(channel);
  }
}
//=============================================================================
void PICOSOUND::stopAll() {
  for (int i = 0; i < g_activeNumChannels; i++) {
    // LLLLLL Aggiunto il seguente if{ }
    // Se streaming da FS, chiudi file
    if (g_channels[i].streamFromFS && g_channels[i].wavFile) {
      g_channels[i].wavFile.close();  
    }
    stopChannelInternal(i);
  }
}
//=============================================================================
void PICOSOUND::setMasterVolume(uint8_t vol) {
  if (vol > 100) vol = 100;
  g_masterVolume = vol;
}
//=============================================================================
uint8_t PICOSOUND::getMasterVolume() {
  return g_masterVolume;
}

//=============================================================================
// PUBLIC API - CUSTOM SOUNDS
//=============================================================================
bool PICOSOUND::playCustomTone(WaveType type, float frequency, uint16_t duration_ms, uint8_t volume) {
  if (type == WAVE_NONE || type == WAVE_WAV) return false;
  int ch = findFreeChannel();
  if (ch < 0) return false;
  int16_t amp = 10000;
  initChannel(ch, type, frequency, amp, volume);
  if (duration_ms > 0) {
    setupChannelDuration(ch, duration_ms);
  }
  PicoSound_channelStatus[ch].soundID = SND_NONE;
  return true;
}
//=============================================================================
bool PICOSOUND::playCustomSweep(WaveType type, float freq_start, float freq_end, uint16_t duration_ms, uint8_t volume) {
  if (type == WAVE_NONE || type == WAVE_WAV || type == WAVE_NOISE) return false;
  int ch = findFreeChannel();
  if (ch < 0) return false;
  int16_t amp = 10000;
  initChannel(ch, type, freq_start, amp, volume);
  setupChannelSweep(ch, freq_start, freq_end, duration_ms);
  if (duration_ms > 0) {
    setupChannelDuration(ch, duration_ms);
  }
  PicoSound_channelStatus[ch].soundID = SND_NONE;
  return true;
}
//=============================================================================
bool PICOSOUND::playCustomNoise(uint16_t duration_ms, uint8_t volume) {
  int ch = findFreeChannel();
  if (ch < 0) return false;
  int16_t amp = 18000;
  initChannel(ch, WAVE_NOISE, 0, amp, volume);
  if (duration_ms > 0) {
    setupChannelDuration(ch, duration_ms);
  }
  PicoSound_channelStatus[ch].soundID = SND_NONE;
  return true;
}
//=============================================================================
bool PICOSOUND::playCustomExplosion(uint16_t duration_ms, uint8_t volume) {
  int ch = findFreeChannel();
  if (ch < 0) return false;
  int16_t amp = 20000;
  initChannel(ch, WAVE_EXPLOSION, 0, amp, volume);
  if (duration_ms > 0) {
    setupChannelDuration(ch, duration_ms);
  }
  PicoSound_channelStatus[ch].soundID = SND_NONE;
  return true;
}
//=============================================================================
bool PICOSOUND::playMelody(const Note* notes, uint8_t numNotes, WaveType waveType, uint8_t volume) {
  if (waveType == WAVE_NONE || waveType == WAVE_WAV || waveType == WAVE_NOISE) return false;
  if (notes == nullptr || numNotes == 0) return false;
  int ch = findFreeChannel();
  if (ch < 0) return false;
  Note firstNote = notes[0];
  int16_t amp = 10000;
  initChannel(ch, waveType, firstNote.frequency, amp, volume);
  g_channels[ch].melody = notes;
  g_channels[ch].melody_numNotes = numNotes;
  g_channels[ch].melody_currentNote = 0;
  g_channels[ch].melody_noteStartPos = 0;
  uint32_t total_duration = 0;
  for (uint8_t i = 0; i < numNotes; i++) {
    total_duration += notes[i].duration;
  }
  setupChannelDuration(ch, total_duration);
  PicoSound_channelStatus[ch].soundID = SND_NONE;
  return true;
}

//=============================================================================
// PUBLIC API - QUERY
//=============================================================================
uint8_t PICOSOUND::getFreeChannels() {
  uint8_t count = 0;
  for (int i = 0; i < g_activeNumChannels; i++) {
    if (!g_channels[i].enabled) count++;
  }
  return count;
}
//=============================================================================
bool PICOSOUND::isChannelActive(uint8_t channel) {
  if (channel >= g_activeNumChannels) return false;
  return g_channels[channel].enabled;
}
//=============================================================================
uint8_t PICOSOUND::findChannelPlayingSound(uint8_t soundID) {
  for (int i = 0; i < g_activeNumChannels; i++) {
    if (PicoSound_channelStatus[i].active && PicoSound_channelStatus[i].soundID == soundID) {
      return i;
    }
  }
  return 255;
}
//=============================================================================
bool PICOSOUND::isSoundPlaying(uint8_t soundID) {
  return (findChannelPlayingSound(soundID) != 255);
}

//=============================================================================
// PUBLIC API - COMMAND PROCESSING
//=============================================================================
void PICOSOUND::processCommand(const AudioCommand* cmd) {
bool success = false;  
  if (cmd == nullptr) return;
  switch(cmd->type) {
    case CMD_PLAY_SOUND:
      success = playSound(cmd->soundID, cmd->volume);
      break;
    case CMD_PLAY_MELODY_CUSTOM: // Custom melodies
      // Use pointers and parameters passed in AudioCommand struct
      success = playMelody(cmd->notes, cmd->numNotes, cmd->waveType, 
                                (cmd->volume == 0) ? 70 : cmd->volume);
      if (!success) {
        Serial.println("[PicoSound] Warning: No free channels for custom melody!");
      }
      break;
    case CMD_PLAY_CUSTOM:
      success = false;
      if (cmd->waveType == WAVE_NOISE) {
        success = playCustomNoise(cmd->duration, cmd->volume);
      } 
      else if (cmd->waveType == WAVE_EXPLOSION) {
        success = playCustomExplosion(cmd->duration, cmd->volume);
      }
      else if (cmd->freq_end != 0) {
        success = playCustomSweep(cmd->waveType, cmd->freq_start, 
                                            cmd->freq_end, cmd->duration, cmd->volume);
      } 
      else {
        success = playCustomTone(cmd->waveType, cmd->freq_start, 
                                          cmd->duration, cmd->volume);
      }
      if (!success) {
        Serial.println("[PicoSound] Warning: No free channels for custom sound!");
      }
      break;
    case CMD_STOP_CHANNEL:
      if (cmd->channel < g_activeNumChannels) {
        stopChannel(cmd->channel);
      }
      break;
    case CMD_STOP_ALL:
      stopAll();
      break;
    case CMD_SET_MASTER_VOLUME:
      setMasterVolume(cmd->volume);
      break;
    case CMD_NONE:
    default:
      break;
  }
}

// END OF FILE
