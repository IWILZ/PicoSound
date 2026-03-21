/*
 * PicoSound - Melodies Template
 * 
 * Optional: Separate melody definitions for cleaner code organization.
 * Include this from picosound_user_cfg.h if using melodies.
 */

#ifndef PICOSOUND_MELODIES_H
#define PICOSOUND_MELODIES_H

#include <PicoSound.h>

//=============================================================================
// MELODY DEFINITIONS
//=============================================================================

// Power-up jingle
#define LEN_POWER_UP (sizeof(MELODY_POWER_UP)/sizeof(Note))
inline const Note MELODY_POWER_UP[] = {
  {262, 100},  // C4
  {330, 100},  // E4
  {392, 100},  // G4
  {523, 200},  // C5
};

// Game over
#define LEN_GAME_OVER (sizeof(MELODY_GAME_OVER)/sizeof(Note))
inline const Note MELODY_GAME_OVER[] = {
  {523, 150},  // C5
  {392, 150},  // G4
  {330, 150},  // E4
  {262, 400},  // C4
};

// Coin pickup
#define LEN_COIN (sizeof(MELODY_COIN)/sizeof(Note))
inline const Note MELODY_COIN[] = {
  {1047, 60},   // C6
  {1319, 120},  // E6
};

#endif
