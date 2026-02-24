#pragma once
#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

// Enciende los altavoces de la consola
bool audio_init(void);

// Apaga el sistema de sonido 
void audio_cleanup(void);

// Reproduce música
void audio_play_music(const char* path);

// Detiene la musica
void audio_stop_music(void);

// Detiene TODOS los sfx a la vez
void audio_stop_all_sfx(void);

// Carga el sonido en memoria y devuelve el puntero
Mix_Chunk* audio_load_sfx(const char* path);

// Reproduce un sonido ya cargado UNA SOLA VEZ y devuelve el canal asignado
int audio_play_sfx_chunk(Mix_Chunk* sfx);

// Reproduce un sonido ya cargado EN BUCLE y devuelve el canal asignado
int audio_play_sfx_loop_chunk(Mix_Chunk* sfx);

// Detiene un canal de audio específico
void audio_stop_channel(int channel);

// Libera la memoria cuando ya no lo necesitamos
void audio_free_sfx(Mix_Chunk* sfx);

// Ajusta el volumen general de un canal (de 0 a 100)
void audio_set_channel_volume(int channel, int volume_percent);

// Ajusta el volumen general de la música (de 0 a 100)
void audio_set_music_volume(int volume_percent);

// Ajusta el volumen base de un efecto de sonido cargado (de 0 a 100)
void audio_set_sfx_volume(Mix_Chunk* sfx, int volume_percent);