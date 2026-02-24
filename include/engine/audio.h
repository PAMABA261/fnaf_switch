#pragma once
#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

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

// Carga el sonido en memoria y devuelve el puntero
Mix_Chunk* audio_load_sfx(const char* path);

// Reproduce un sonido ya cargado en bucle
void audio_play_sfx_loop_chunk(Mix_Chunk* sfx);

// Libera la memoria cuando ya no lo necesitamos
void audio_free_sfx(Mix_Chunk* sfx);