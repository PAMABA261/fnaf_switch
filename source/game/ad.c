#include "game/assets.h"
#include "game/state_manager.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/audio.h"
#include <SDL2/SDL.h>

static SDL_Texture* tex_newspaper = NULL;
static int ad_timer = 0; 
static bool is_transitioning_out = false; 
static int fade_timer = 0;

void ad_init(void) {
    tex_newspaper = graphics_load_texture(IMG_NEWSPAPER);
    ad_timer = 0;
    is_transitioning_out = false;
    fade_timer = 0;
}

void ad_update(void) {
    if (!is_transitioning_out) {
        ad_timer++;
        if (input_get_button_down(HidNpadButton_A) || ad_timer > 300) {
        is_transitioning_out = true;
        }  
    } else {
        fade_timer++;
        if (fade_timer >= 120) {
            audio_stop_music();
            state_manager_change(STATE_WHAT_DAY);
        }
    }
}

void ad_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    if (tex_newspaper) {
        SDL_Rect dst_rect = {0, 0, 1280, 720};
        SDL_RenderCopy(renderer, tex_newspaper, NULL, &dst_rect);
    }

    if (is_transitioning_out) {
        int alpha = (fade_timer * 255) / 120; // Calculamos la transparencia en función del tiempo de fade
        if (alpha > 255) alpha = 255; // Aseguramos que no supere el máximo

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
        
        SDL_Rect black_rect = {0, 0, 1280, 720};
        SDL_RenderFillRect(renderer, &black_rect);
    }
}

void ad_cleanup(void) {
    if (tex_newspaper) {
        SDL_DestroyTexture(tex_newspaper);
        tex_newspaper = NULL;
    }
}