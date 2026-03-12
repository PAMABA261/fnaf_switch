#include "game/hud.h"
#include "game/assets.h"
#include "engine/graphics.h"
#include <SDL2/SDL.h>

// Variables (las declaramos aquí, el 'extern' del .h hace que los demás puedan verlas)
int current_night = 1;
int current_hour = 0;
static Uint64 night_start_time = 0;

static SDL_Texture* tex_night_text = NULL;
static SDL_Texture* tex_night_num[5] = {NULL};
static SDL_Texture* tex_hour[6] = {NULL};
static SDL_Texture* tex_am = NULL;

#define CAM_FRAMES 11 // Necesitamos saber esto para ocultar el HUD si la cámara está subida

void hud_init(void) {
    // Carga de gráficos
    tex_hour[0] = graphics_load_texture(IMG_HOUR_1);
    tex_hour[1] = graphics_load_texture(IMG_HOUR_2);
    tex_hour[2] = graphics_load_texture(IMG_HOUR_3);
    tex_hour[3] = graphics_load_texture(IMG_HOUR_4);
    tex_hour[4] = graphics_load_texture(IMG_HOUR_5);
    tex_hour[5] = graphics_load_texture(IMG_HOUR_6);
    tex_am = graphics_load_texture(IMG_HOUR_AM);
    
    tex_night_text = graphics_load_texture(IMG_NIGHT_GAME);
    tex_night_num[0] = graphics_load_texture(IMG_NIGHT_NUM_1);
    tex_night_num[1] = graphics_load_texture(IMG_NIGHT_NUM_2);
    tex_night_num[2] = graphics_load_texture(IMG_NIGHT_NUM_3);
    tex_night_num[3] = graphics_load_texture(IMG_NIGHT_NUM_4);
    tex_night_num[4] = graphics_load_texture(IMG_NIGHT_NUM_5);

    // Inicialización de tiempo
    current_hour = 0;
    night_start_time = SDL_GetTicks64();
}

void hud_update(void) {
    Uint64 current_time = SDL_GetTicks64();
    // Calculamos la hora (basado en los 86000 ticks que le pusiste)
    current_hour = (current_time - night_start_time) / 86000;
}

void hud_draw(bool cam_open, float cam_frame) {
    SDL_Renderer* renderer = graphics_get_renderer();
    
    int am_x = 1200, am_y = 31, num_x = 1135, num_y = 29; 

    // Solo dibujamos el reloj si la tableta NO está arriba
    if (!cam_open || cam_frame < CAM_FRAMES - 1) { 
        if (current_hour == 0) { 
            if (tex_hour[0] && tex_hour[1]) {
                SDL_Rect dst_1 = {num_x, num_y, 24, 30};
                SDL_RenderCopy(renderer, tex_hour[0], NULL, &dst_1);
                SDL_Rect dst_2 = {num_x + 24, num_y, 24, 30};
                SDL_RenderCopy(renderer, tex_hour[1], NULL, &dst_2);
            }
        } else if (current_hour >= 1 && current_hour <= 6) {
            int h_idx = current_hour - 1;
            if (tex_hour[h_idx]) {
                SDL_Rect dst_h = {num_x + 24, num_y, 24, 30}; 
                SDL_RenderCopy(renderer, tex_hour[h_idx], NULL, &dst_h);
            }
        }
        
        if (tex_am) {
            SDL_Rect dst_am = {am_x, am_y, 42, 26}; 
            SDL_RenderCopy(renderer, tex_am, NULL, &dst_am);
        }

        if (tex_night_text) {
            int night_x = 1148, night_y = 74;
            SDL_Rect dst_night = {night_x, night_y, 63, 14};
            SDL_RenderCopy(renderer, tex_night_text, NULL, &dst_night);
            
            if (current_night >= 1 && current_night <= 5) {
                int n_idx = current_night - 1;
                if (tex_night_num[n_idx]) {
                    int num_x_pos = night_x + 63 + 10; 
                    int num_y_pos = 72;                 
                    SDL_Rect dst_num = {num_x_pos, num_y_pos, 14, 17};
                    SDL_RenderCopy(renderer, tex_night_num[n_idx], NULL, &dst_num);
                }
            }
        }
    }
}

void hud_cleanup(void) {
    if (tex_am) { SDL_DestroyTexture(tex_am); tex_am = NULL; }
    if (tex_night_text) { SDL_DestroyTexture(tex_night_text); tex_night_text = NULL; }
    
    for (int i = 0; i < 6; i++) {
        if (tex_hour[i]) { SDL_DestroyTexture(tex_hour[i]); tex_hour[i] = NULL; }
    }
    for (int i = 0; i < 5; i++) {
        if (tex_night_num[i]) { SDL_DestroyTexture(tex_night_num[i]); tex_night_num[i] = NULL; }
    }
}