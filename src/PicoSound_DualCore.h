/*
 * PicoSound_DualCore.h
 * 
 * Funzioni helper per uso dual-core di PicoSound
 * Wrapper high-level per comunicazione Core0 → Core1 via PicoSem
 * 
 * DIPENDENZE:
 * - PicoSound.h
 * - PicoSem.h
 * 
 * SETUP RICHIESTO:
 * Devi definire nel tuo progetto (main.cpp o AudioCore.cpp):
 *   extern PicoSem AudioSem;
 *   extern AudioCommand audioCmd;
 */

#ifndef PICOSOUND_DUALCORE_H
#define PICOSOUND_DUALCORE_H

#include <PicoSound.h>
#include <PicoSem.h>

//=============================================================================
// VARIABILI ESTERNE (da definire nel progetto principale)
//=============================================================================
extern PicoSem AudioSem;
extern AudioCommand audioCmd;
extern PICOSOUND PicoSound;  

//=============================================================================
// FUNZIONI WRAPPER DUAL-CORE
//=============================================================================
/**
 * @brief Invia comando audio a Core1
 * @param cmdType Tipo di comando (CMD_PLAY_SOUND, CMD_PLAY_CUSTOM, ecc.)
 * @param soundID ID del suono (dalla tabella)
 * @param volume Volume 0-100
 * @param wave Tipo di onda (WAVE_SQUARE, WAVE_SINE, ecc.)
 * @param freq_start Frequenza iniziale (Hz)
 * @param freq_end Frequenza finale (Hz) per sweep
 * @param duration Durata (ms)
 * @param notes Array di note (per melodie)
 * @param numNotes Numero di note
 */
void SendAudioCommand(AudioCommandType cmdType, uint8_t soundID = 0, 
                      uint8_t volume = 80, WaveType wave = WAVE_SQUARE,
                      float freq_start = 0, float freq_end = 0, uint16_t duration = 0,
                      const Note* notes = nullptr, uint8_t numNotes = 0,
                      uint8_t channel = 255);

/**
 * @brief Suona una melodia su Core1
 * @param notes Array di note
 * @param numNotes Numero di note
 * @param wave Tipo di onda (default: WAVE_SQUARE)
 * @param volume Volume 0-100 (default: 80)
 */
void PlayMelody(const Note* notes, uint8_t numNotes, 
                WaveType wave = WAVE_SQUARE, uint8_t volume = 80);

/**
 * @brief Suona un tono semplice (compatibilità con tone())
 * @param frequency Frequenza in Hz
 * @param duration Durata in ms
 * @param volume Volume 0-100 (default: 80)
 */
void PlayTone(int frequency, int duration, uint8_t volume = 80);

/**
 * @brief Ferma tutti i suoni
 */
void StopAllSounds();

/**
 * @brief Ferma un suono specifico
 * @param soundID ID del suono da fermare
 */
void StopSound(uint8_t soundID);

/**
 * @brief Imposta volume master
 * @param volume Volume 0-100
 */
void SetMasterVolume(uint8_t volume);

#endif // PICOSOUND_DUALCORE_H
