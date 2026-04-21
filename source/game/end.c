#include "game/state_manager.h"
#include "game/assets.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include <SDL2/SDL.h>

extern int night_beaten; // Traemos la variable desde state_6am.c
static SDL_Texture* tex_paycheck = NULL;
static Mix_Chunk* sfx_music_box = NULL;

static int end_timer = 0;
static float fade_alpha = 0.0f;

void state_the_end_init(void) {
    audio_stop_all_sfx();
    audio_stop_music();

    // --- LÓGICA DE SELECCIÓN DE IMAGEN ---
    if (night_beaten == 5) {
        tex_paycheck = graphics_load_texture(IMG_END_1); // $120.00
    } else if (night_beaten == 6) {
        tex_paycheck = graphics_load_texture(IMG_END_2); // $120.50 (Overtime)
    } else if (night_beaten >= 7) {
        tex_paycheck = graphics_load_texture(IMG_END_3); // Despido (Pink Slip)
    }

    sfx_music_box = audio_load_sfx("romfs:/sfx/music_box.wav");
    if (sfx_music_box) {
        audio_set_sfx_volume(sfx_music_box, 100);
        Mix_PlayChannel(1, sfx_music_box, 0); 
    }

    end_timer = 0;
    fade_alpha = 0.0f; 
}

void state_the_end_update(void) {
    end_timer++;

    // Fade In inicial (0 a 1 segundo)
    if (end_timer < 60) {
        fade_alpha += 4.25f; 
        if (fade_alpha > 255.0f) fade_alpha = 255.0f;
    } 
    // --- CAMBIO: Fade Out retrasado (Empieza en el frame 810 / segundo 13.5) ---
    else if (end_timer > 810) {
        fade_alpha -= 3.0f; 
        if (fade_alpha < 0.0f) fade_alpha = 0.0f;
    }

    // Al llegar a los 15 segundos (900 frames) volvemos al menú.
    if (end_timer >= 900) {
        state_manager_change(STATE_TITLE);
    }
}

void state_the_end_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    // Fondo negro
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (tex_paycheck) {
        SDL_SetTextureBlendMode(tex_paycheck, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(tex_paycheck, (Uint8)fade_alpha);
        SDL_RenderCopy(renderer, tex_paycheck, NULL, NULL);
    }
}

void state_the_end_cleanup(void) {
    if (tex_paycheck) {
        SDL_DestroyTexture(tex_paycheck);
        tex_paycheck = NULL;
    }
    
    if (sfx_music_box) {
        audio_free_sfx(sfx_music_box);
        sfx_music_box = NULL;
    }
}