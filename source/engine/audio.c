#include "engine/audio.h"
#include <SDL2/SDL_mixer.h>

bool audio_init(void) {
    // Iniciamos el mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        return false; 
    }
    // Reservamos 16 canales para efectos de sonido. 
    Mix_AllocateChannels(16);
    
    return true;
}

void audio_cleanup(void) {
    // Apagamos el sistema de sonido
    Mix_CloseAudio();
}

void audio_play_music(const char* path) {
    // Cargamos la música y la reproducimos
    Mix_Music* music = Mix_LoadMUS(path);
    if (music != NULL) {
        Mix_PlayMusic(music, -1); // El "-1" significa que se repetirá en bucle infinito
    }
}

void audio_play_sfx(const char* path) {
    // Cargamos el efecto de sonido
    Mix_Chunk* sfx = Mix_LoadWAV(path);
    if (sfx != NULL) {
        Mix_PlayChannel(-1, sfx, 0); // El "-1" busca el primer canal libre. El "0" significa que no hace bucle
    }
}

void audio_stop_music(void) {
    Mix_HaltMusic();
}

void audio_stop_all_sfx(void) {
    Mix_HaltChannel(-1); // Detiene todos los canales de sonido
}

Mix_Chunk* audio_load_sfx(const char* path) {
    return Mix_LoadWAV(path);
}

void audio_play_sfx_loop_chunk(Mix_Chunk* sfx) {
    if (sfx != NULL) {
        Mix_PlayChannel(-1, sfx, -1); // El "-1" busca el primer canal libre. El "-1" para que haga bucle infinito
    }
}

void audio_free_sfx(Mix_Chunk* sfx) {
    Mix_FreeChunk(sfx);
}