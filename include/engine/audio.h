#pragma once
#include <stdbool.h>

// Enciende los altavoces de la consola
bool audio_init(void);

// Apaga el sistema de sonido 
void audio_cleanup(void);

// Reproduce música
void audio_play_music(const char* path);

// Reproduce sfx
void audio_play_sfx(const char* path);

// Detiene la musica
void audio_stop_music(void);

// Detiene los sfx
void audio_stop_all_sfx(void);