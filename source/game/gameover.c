#include "game/state_manager.h"
#include "game/assets.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

// --- Variables de Textura ---
static SDL_Texture* tex_static[8] = {NULL};
static SDL_Texture* tex_gameover_bg = NULL; 
static SDL_Texture* tex_gameover_txt = NULL; 
static SDL_Texture* tex_blip[9] = {NULL};

// --- Variables de Audio ---
static Mix_Chunk* sfx_static = NULL;
static Mix_Chunk* sfx_blip = NULL; // Sonido de las barras blancas
static int channel_static = -1;

// --- Lógica y Animación ---
static bool is_blipping = true;
static float blip_anim_frame = 0.0f;
static int timer = 0;
static float static_frame = 0.0f;
static float crossfade_alpha = 0.0f;
static int phase = 0; // 0 = Estática, 1 = Pantalla Game Over

void state_gameover_init(void) {
    // 1. Limpieza total de sonidos previos (oficina, ventilador, etc.)
    audio_stop_all_sfx();
    audio_stop_music();

    // 2. Cargar texturas de estática
    const char* paths_static[8] = { 
        IMG_STATIC_1, IMG_STATIC_2, IMG_STATIC_3, IMG_STATIC_4, 
        IMG_STATIC_5, IMG_STATIC_6, IMG_STATIC_7, IMG_STATIC_8 
    };
    for (int i = 0; i < 8; i++) {
        tex_static[i] = graphics_load_texture(paths_static[i]);
    }

    // 3. Cargar texturas del Blip (Barras blancas)
    const char* paths_blip[9] = { 
        IMG_BLIP_FLASH_3, IMG_BLIP_FLASH_4, IMG_BLIP_FLASH_5, 
        IMG_BLIP_FLASH_6, IMG_BLIP_FLASH_7, IMG_BLIP_FLASH_8, 
        IMG_BLIP_FLASH_9, IMG_BLIP_FLASH_10, IMG_BLIP_FLASH_11 
    };
    for (int i = 0; i < 9; i++) {
        tex_blip[i] = graphics_load_texture(paths_blip[i]);
    }

    // 4. Cargar pantalla de Game Over
    tex_gameover_bg = graphics_load_texture(IMG_GAME_OVER);
    tex_gameover_txt = graphics_load_texture(IMG_GAME_OVER_TEXT);

    // 5. Configurar Audio
    sfx_static = audio_load_sfx("romfs:/sfx/static2.wav"); 
    if (sfx_static) {
        audio_set_sfx_volume(sfx_static, 100);
        channel_static = audio_play_sfx_loop_chunk(sfx_static);
    }

    // --- AÑADIDO: Sonido del Blip al entrar ---
    sfx_blip = audio_load_sfx("romfs:/sfx/blip3.wav");
    if (sfx_blip) {
        audio_set_sfx_volume(sfx_blip, 100);
        audio_play_sfx_chunk(sfx_blip);
    }

    // 6. Reset de variables
    timer = 0;
    phase = 0; 
    static_frame = 0.0f;
    crossfade_alpha = 0.0f;
    is_blipping = true;
    blip_anim_frame = 0.0f;
}

void state_gameover_update(void) {
    timer++;

    if (phase == 0) {
        // --- FASE 0: ESTÁTICA ---
        static_frame += 1.0f;
        if (static_frame >= 8.0f) static_frame = 0.0f;

        if (is_blipping) {
            blip_anim_frame += 0.70f;
            if (blip_anim_frame >= 9.0f) is_blipping = false;
        }

        // A los 10 segundos pasamos a la pantalla negra con el texto
        if (timer >= 600) {
            phase = 1;      
            timer = 0;      
            if (channel_static != -1) audio_stop_channel(channel_static); 
        }
    } 
    else if (phase == 1) {
        // --- FASE 1: CROSSFADE ---
        if (crossfade_alpha < 255.0f) {
            crossfade_alpha += 3.0f;
            if (crossfade_alpha > 255.0f) crossfade_alpha = 255.0f;
        }

        if (timer >= 600) {
            state_manager_change(STATE_TITLE);
        }
    }
}

void state_gameover_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    // Dibujar Estática y Blip (mientras no sea opaco el Game Over)
    if (phase == 0 || (phase == 1 && crossfade_alpha < 255.0f)) {
        int s_idx = (int)static_frame;
        if (tex_static[s_idx]) {
            SDL_RenderCopy(renderer, tex_static[s_idx], NULL, NULL);
        }

        if (is_blipping) {
            int b_idx = (int)blip_anim_frame;
            if (b_idx >= 0 && b_idx < 9 && tex_blip[b_idx]) {
                SDL_RenderCopy(renderer, tex_blip[b_idx], NULL, NULL);
            }
        }
    } 

    // Dibujar pantalla Game Over con transparencia
    if (phase == 1) {
        Uint8 alpha = (Uint8)crossfade_alpha;

        if (tex_gameover_bg) {
            SDL_SetTextureBlendMode(tex_gameover_bg, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(tex_gameover_bg, alpha);
            SDL_RenderCopy(renderer, tex_gameover_bg, NULL, NULL);
        } else {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
            SDL_Rect full = {0, 0, 1280, 720};
            SDL_RenderFillRect(renderer, &full);
        }

        if (tex_gameover_txt) {
            SDL_SetTextureBlendMode(tex_gameover_txt, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(tex_gameover_txt, alpha);
            SDL_Rect dst_txt = {1046, 660, 206, 27}; 
            SDL_RenderCopy(renderer, tex_gameover_txt, NULL, &dst_txt);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

void state_gameover_cleanup(void) {
    for (int i = 0; i < 8; i++) {
        if (tex_static[i]) SDL_DestroyTexture(tex_static[i]);
        tex_static[i] = NULL;
    }
    for (int i = 0; i < 9; i++) {
        if (tex_blip[i]) SDL_DestroyTexture(tex_blip[i]);
        tex_blip[i] = NULL;
    }
    if (tex_gameover_bg) SDL_DestroyTexture(tex_gameover_bg);
    if (tex_gameover_txt) SDL_DestroyTexture(tex_gameover_txt);
    
    if (channel_static != -1) audio_stop_channel(channel_static);
    if (sfx_static) audio_free_sfx(sfx_static);
    if (sfx_blip) audio_free_sfx(sfx_blip); // Limpieza del sonido del blip
}