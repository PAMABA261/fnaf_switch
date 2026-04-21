#include "game/assets.h"
#include "game/state_manager.h"
#include "game/save_system.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include <SDL2/SDL.h>

extern int current_night;

static SDL_Texture* tex_5 = NULL;
static SDL_Texture* tex_6 = NULL;
static SDL_Texture* tex_am = NULL;

static Mix_Chunk* sfx_chimes = NULL;
static Mix_Chunk* sfx_cheer = NULL;

static int timer = 0;
static float num_y_offset = 0.0f; 
static bool cheer_played = false;
static float fade_in = 255.0f; 
static float fade_out = 0.0f;

void state_6am_init(void) {
    // 1. CARGA DE GRÁFICOS Y AUDIO
    tex_5 = graphics_load_texture(IMG_6AM_5);
    tex_6 = graphics_load_texture(IMG_6AM_6);
    tex_am = graphics_load_texture(IMG_6AM_AM);
    
    sfx_chimes = audio_load_sfx("romfs:/sfx/chimes_2.wav");
    sfx_cheer = audio_load_sfx("romfs:/sfx/CROWD_SMALL_CHIL_EC049202.wav");

    // 2. LÓGICA DE INICIO (Guardado y Reseteo)
    current_night++;
    if (current_night > 5) {
        current_night = 5; 
    }
    save_system_save();

    // 3. INICIALIZACIÓN DE VARIABLES
    timer = 0;
    num_y_offset = 0.0f;
    cheer_played = false;
    fade_in = 255.0f; 
    fade_out = 0.0f;

    // 4. REPRODUCCIÓN DE AUDIO INICIAL
    if (sfx_chimes) {
        audio_play_sfx_chunk(sfx_chimes);
    }
}

void state_6am_update(void) {
    timer++;

    if (fade_in > 0.0f) {
        fade_in -= 20.0f;
        if (fade_in < 0.0f) fade_in = 0.0f;
    }

    if (timer > 60 && timer <= 360) {
        num_y_offset += 0.3666f; 
    }

    if (timer == 360) {
        num_y_offset = 110.0f; 
        if (sfx_cheer && !cheer_played) {
            audio_set_sfx_volume(sfx_cheer, 100);
            audio_play_sfx_chunk(sfx_cheer);
            cheer_played = true;
        }
    }

    if (timer > 540 && timer <= 600) {
        fade_out += 4.25f;
        if (fade_out > 255.0f) fade_out = 255.0f;
    }

    if (timer > 600) {
        if (current_night <= 5) {
            state_manager_change(STATE_WHAT_DAY); 
        } else {
            state_manager_change(STATE_TITLE); 
        }
    }
}

void state_6am_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (tex_am) {
        int pos_am_x = 645; 
        int pos_am_y = 296;
        SDL_Rect rect_am = {pos_am_x, pos_am_y, 113, 72}; 
        SDL_RenderCopy(renderer, tex_am, NULL, &rect_am);
    }

    if (tex_5) {
        int pos_5_x = 549; 
        int pos_5_y = 298;
        SDL_Rect rect_5 = {pos_5_x, pos_5_y - (int)num_y_offset, 53, 72};
        SDL_RenderCopy(renderer, tex_5, NULL, &rect_5);
    }

    if (tex_6) {
        int pos_6_x = 553; 
        int pos_6_y = 408; 
        SDL_Rect rect_6 = {pos_6_x, pos_6_y - (int)num_y_offset, 53, 72};
        SDL_RenderCopy(renderer, tex_6, NULL, &rect_6);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    
    SDL_Rect top_mask = {498, 169, 158, 118};
    SDL_RenderFillRect(renderer, &top_mask);

    SDL_Rect bottom_mask = {499, 385, 158, 118};
    SDL_RenderFillRect(renderer, &bottom_mask);

    if (fade_in > 0.0f) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)fade_in);
        SDL_Rect fullscreen = {0, 0, 1280, 720};
        SDL_RenderFillRect(renderer, &fullscreen);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    if (fade_out > 0.0f) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)fade_out);
        SDL_Rect fullscreen = {0, 0, 1280, 720};
        SDL_RenderFillRect(renderer, &fullscreen);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

void state_6am_cleanup(void) {
    if (tex_5) { SDL_DestroyTexture(tex_5); tex_5 = NULL; }
    if (tex_6) { SDL_DestroyTexture(tex_6); tex_6 = NULL; }
    if (tex_am) { SDL_DestroyTexture(tex_am); tex_am = NULL; }
    if (sfx_chimes) { audio_free_sfx(sfx_chimes); sfx_chimes = NULL; }
    if (sfx_cheer) { audio_free_sfx(sfx_cheer); sfx_cheer = NULL; }
}