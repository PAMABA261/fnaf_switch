#include "game/assets.h"
#include "game/state_manager.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/audio.h"
#include <SDL2/SDL.h>

extern int current_night;

static SDL_Texture* tex_night = NULL;
static SDL_Rect dest_rect; 
static int timer = 0;

static SDL_Texture* tex_blip_flash[11] = {NULL};

static Mix_Chunk* sfx_blip = NULL;

static float fade_out = 0.0f;

void what_day_init(void) { 
    // 1. CARGA DE GRÁFICOS Y AUDIO
    sfx_blip = audio_load_sfx("romfs:/sfx/blip3.wav");

    int index = current_night - 1;
    if (index < 0) index = 0;
    if (index > 6) index = 6;

    const char* night_paths[7] = {IMG_NIGHT_1, IMG_NIGHT_2, IMG_NIGHT_3, IMG_NIGHT_4, IMG_NIGHT_5, IMG_NIGHT_6, IMG_NIGHT_7};
    tex_night = graphics_load_texture(night_paths[index]);

    const char* blip_flash_paths[11] = {IMG_BLIP_FLASH_1, IMG_BLIP_FLASH_2, IMG_BLIP_FLASH_3, IMG_BLIP_FLASH_4, IMG_BLIP_FLASH_5, IMG_BLIP_FLASH_6, IMG_BLIP_FLASH_7, IMG_BLIP_FLASH_8, IMG_BLIP_FLASH_9, IMG_BLIP_FLASH_10, IMG_BLIP_FLASH_11};
    for (int i = 0; i < 11; i++) {
        tex_blip_flash[i] = graphics_load_texture(blip_flash_paths[i]);
    }

    // 2. INICIALIZACIÓN DE VARIABLES
    audio_stop_music();
    timer = 0;
    fade_out = 0.0f;

    int widths[7] = {227, 231, 231, 231, 231, 233, 240};
    int heights[7] = {97, 97, 97, 97, 97, 97, 100};

    dest_rect.w = widths[index];
    dest_rect.h = heights[index];
    dest_rect.x = 646 - (dest_rect.w / 2);
    dest_rect.y = 318 - (dest_rect.h / 2);

    // 3. REPRODUCCIÓN DE AUDIO INICIAL
    if (sfx_blip) {
        audio_play_sfx_chunk(sfx_blip);
    }
}

void what_day_update(void) {
    timer++;
    
    // Inicia el fundido a negro en el último segundo (del frame 70 al 130)
    if (timer > 70) {
        fade_out += 4.25f; // Sube de 0 a 255 en 60 fotogramas
        if (fade_out > 255.0f) fade_out = 255.0f;
    }

    if (timer > 130) {
        state_manager_change(STATE_LOADING);
    }
}

void what_day_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    // Limpiamos la pantalla de negro por debajo
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Dibujamos el texto de la noche
    if (tex_night) {
        SDL_RenderCopy(renderer, tex_night, NULL, &dest_rect);
    }

    // Dibujamos la animación del blip (el parpadeo inicial)
    if (timer >= 1 && timer <= 11) {
        if (tex_blip_flash[timer - 1]) {
            SDL_RenderCopy(renderer, tex_blip_flash[timer - 1], NULL, NULL);
        }
    }

    // Dibujamos la capa negra del fundido por encima de todo
    if (fade_out > 0.0f) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)fade_out);
        SDL_Rect fullscreen = {0, 0, 1280, 720};
        SDL_RenderFillRect(renderer, &fullscreen);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

void what_day_cleanup(void) {
    if (tex_night) {
        SDL_DestroyTexture(tex_night);
        tex_night = NULL; 
    }

    for (int i = 0; i < 11; i++) {
        if(tex_blip_flash[i]) {
            SDL_DestroyTexture(tex_blip_flash[i]);
            tex_blip_flash[i] = NULL;
        }
    }

    if (sfx_blip) {
        audio_free_sfx(sfx_blip);
        sfx_blip = NULL;
    }
}