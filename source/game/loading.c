#include "game/assets.h"
#include "game/state_manager.h"
#include "engine/graphics.h"
#include <SDL2/SDL.h>

static SDL_Texture* tex_clock = NULL;
static int timer = 0;

void loading_init(void) {
    // 1. CARGA DE GRÁFICOS
    tex_clock = graphics_load_texture(IMG_CLOCK);
    
    // 2. INICIALIZACIÓN DE VARIABLES
    timer = 0;
}

void loading_update(void) {
    timer++;
    if (timer >= 30) {
        state_manager_change(STATE_GAME);
    }
}

void loading_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (tex_clock) {
        SDL_Rect dest_clock = {1206, 657, 40, 40}; 
        SDL_RenderCopy(renderer, tex_clock, NULL, &dest_clock);
    }
}

void loading_cleanup(void) {
    if (tex_clock) {
        SDL_DestroyTexture(tex_clock);
        tex_clock = NULL;
    }
}