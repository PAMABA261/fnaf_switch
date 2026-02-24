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
        Mix_PlayMusic(music, -1); // El "-1" significa que se repetirá en bucle
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

void audio_free_sfx(Mix_Chunk* sfx) {
    if (sfx != NULL) {
        Mix_FreeChunk(sfx);
    }
}

int audio_play_sfx_chunk(Mix_Chunk* sfx) {
    // Reproduce un sonido precargado 1 vez y devuelve el canal asignado
    if (sfx != NULL) {
        return Mix_PlayChannel(-1, sfx, 0); 
    }
    return -1; // -1 significa error o que no hay canales libres
}

int audio_play_sfx_loop_chunk(Mix_Chunk* sfx) {
    // Reproduce en bucle infinito y devuelve el canal asignado
    if (sfx != NULL) {
        return Mix_PlayChannel(-1, sfx, -1); 
    }
    return -1;
}

void audio_stop_channel(int channel) {
    // Detiene un canal específico 
    if (channel != -1) {
        Mix_HaltChannel(channel);
    }
}

void audio_set_channel_volume(int channel, int volume_percent) {
    // Protegemos contra valores fuera de rango
    if (volume_percent < 0) volume_percent = 0;
    if (volume_percent > 100) volume_percent = 100;

    int sdl_volume = (volume_percent * MIX_MAX_VOLUME) / 100;
    Mix_Volume(channel, sdl_volume);
}

void audio_set_music_volume(int volume_percent) {
    if (volume_percent < 0) volume_percent = 0;
    if (volume_percent > 100) volume_percent = 100;

    int sdl_volume = (volume_percent * MIX_MAX_VOLUME) / 100;
    Mix_VolumeMusic(sdl_volume);
}

void audio_set_sfx_volume(Mix_Chunk* sfx, int volume_percent) {
    if (sfx != NULL) {
        if (volume_percent < 0) volume_percent = 0;
        if (volume_percent > 100) volume_percent = 100;

        int sdl_volume = (volume_percent * MIX_MAX_VOLUME) / 100;
        Mix_VolumeChunk(sfx, sdl_volume);
    }
}