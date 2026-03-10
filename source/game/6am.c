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

void state_6am_init(void) {
    audio_stop_all_sfx();
    audio_stop_music();

    current_night++;
    
    // Si pasamos la noche 5, volvemos a ponerla a 5 en el guardado
    if (current_night > 5) {
        current_night = 5; 
    }
    save_system_save();

    tex_5 = graphics_load_texture(IMG_6AM_5);
    tex_6 = graphics_load_texture(IMG_6AM_6);
    tex_am = graphics_load_texture(IMG_6AM_AM);
    
    sfx_chimes = audio_load_sfx("romfs:/sfx/chimes_2.wav");
    sfx_cheer = audio_load_sfx("romfs:/sfx/CROWD_SMALL_CHIL_EC049202.wav");

    if (sfx_chimes) {
        audio_set_sfx_volume(sfx_chimes, 100);
        audio_play_sfx_chunk(sfx_chimes);
    }

    timer = 0;
    num_y_offset = 0.0f;
    cheer_played = false;
}

void state_6am_update(void) {
    timer++;

    if (timer > 120 && timer <= 180) {
        num_y_offset += 1.5f; 
    }

    if (timer == 180) {
        if (sfx_cheer && !cheer_played) {
            audio_set_sfx_volume(sfx_cheer, 100);
            audio_play_sfx_chunk(sfx_cheer);
            cheer_played = true;
        }
    }

    if (timer > 380) {
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

    int pos_5_x = 549; 
    int pos_5_y = 298;
    
    int pos_am_x = 645; 
    int pos_am_y = 296;

    if (tex_am) {
        SDL_Rect rect_am = {pos_am_x, pos_am_y, 113, 72}; 
        SDL_RenderCopy(renderer, tex_am, NULL, &rect_am);
    }

    if (tex_5 && num_y_offset < 72.0f) {
        SDL_Rect rect_5 = {pos_5_x, pos_5_y - (int)num_y_offset, 53, 72};
        SDL_RenderCopy(renderer, tex_5, NULL, &rect_5);
    }

    if (tex_6 && num_y_offset > 0.0f) {
        int y_pos = (pos_5_y + 72) - (int)num_y_offset;
        if (y_pos < pos_5_y) y_pos = pos_5_y; 
        
        SDL_Rect rect_6 = {pos_5_x, y_pos, 53, 72};
        SDL_RenderCopy(renderer, tex_6, NULL, &rect_6);
    }
}

void state_6am_cleanup(void) {
    if (tex_5) { SDL_DestroyTexture(tex_5); tex_5 = NULL; }
    if (tex_6) { SDL_DestroyTexture(tex_6); tex_6 = NULL; }
    if (tex_am) { SDL_DestroyTexture(tex_am); tex_am = NULL; }
    if (sfx_chimes) { audio_free_sfx(sfx_chimes); sfx_chimes = NULL; }
    if (sfx_cheer) { audio_free_sfx(sfx_cheer); sfx_cheer = NULL; }
}