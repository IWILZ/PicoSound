/*
 * PicoSound_AudioCore.h
 * 
 * Setup e Loop per Core1 (Audio Engine)
 * Condiviso tra tutti i giochi e il Multigame
 * 
 * IMPORTANTE: Questo file è IDENTICO in tutti i progetti.
 * Non modificare direttamente, ma sincronizza modifiche.
 */

#ifndef PICOSOUND_AUDIOCORE_H
#define PICOSOUND_AUDIOCORE_H

#include <Arduino.h>
#include <PicoSem.h>
#include <PicoSound.h>
#include <LittleFS.h>

//=============================================================================
// VARIABILI GLOBALI AUDIO (condivise tra Core0 e Core1)
//=============================================================================
extern PicoSem AudioSem;
extern AudioCommand audioCmd;
extern PICOSOUND PicoSound;

//=============================================================================
// FUNZIONI CORE1 (implementate in PicoSound_AudioCore.cpp)
//=============================================================================
/**
 * @brief Inizializza il sistema audio su Core1
 * Chiama questa funzione da setup1()
 */
void PicoSound_AudioCore_Setup1();

/**
 * @brief Loop principale del sistema audio su Core1
 * Chiama questa funzione da loop1()
 */
void PicoSound_AudioCore_Loop1();

#endif // PICOSOUND_AUDIOCORE_H
