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

void what_day_init(void) { 
    timer = 0;
    audio_stop_music();
    audio_stop_all_sfx();

    audio_play_sfx("romfs:/sfx/blip3.wav");

    const char* night_paths[7] = {IMG_NIGHT_1, IMG_NIGHT_2, IMG_NIGHT_3, IMG_NIGHT_4, IMG_NIGHT_5, IMG_NIGHT_6, IMG_NIGHT_7};

    int widths[7] = {227, 231, 231, 231, 231, 233, 240};
    int heights[7] = {97, 97, 97, 97, 97, 97, 100};

    int index = current_night - 1;
    if (index < 0) index = 0;
    if (index > 6) index = 6;

    tex_night = graphics_load_texture(night_paths[index]);

    // Calculamos las coordenadas para centrar la imagen en la pantalla
    dest_rect.w = widths[index];
    dest_rect.h = heights[index];
    dest_rect.x = 646 - (dest_rect.w / 2);
    dest_rect.y = 318 - (dest_rect.h / 2);

    const char* blip_flash_paths[11] = {IMG_BLIP_FLASH_1, IMG_BLIP_FLASH_2, IMG_BLIP_FLASH_3, IMG_BLIP_FLASH_4, IMG_BLIP_FLASH_5, IMG_BLIP_FLASH_6, IMG_BLIP_FLASH_7, IMG_BLIP_FLASH_8, IMG_BLIP_FLASH_9, IMG_BLIP_FLASH_10, IMG_BLIP_FLASH_11};
    for (int i = 0; i < 11; i++) {
        tex_blip_flash[i] = graphics_load_texture(blip_flash_paths[i]);
    }
}

void what_day_update(void) {
    timer++;
    
    if (timer > 130) {
        state_manager_change(STATE_GAME);
    }
}

void what_day_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    if (tex_night) {
        SDL_RenderCopy(renderer, tex_night, NULL, &dest_rect);
    }

    if (timer >= 1 && timer <= 11) {
        if (tex_blip_flash[timer - 1]) {
            SDL_RenderCopy(renderer, tex_blip_flash[timer - 1], NULL, NULL);
        }
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
}