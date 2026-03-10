#include "game/assets.h"
#include "game/state_manager.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include <SDL2/SDL.h>

static SDL_Texture* tex_warning = NULL;
static int timer = 0;
static SDL_Rect dest_rect; 

void warning_init(void) {
    tex_warning = graphics_load_texture(IMG_WARNING);

    timer = 0;

    dest_rect.w = 465;
    dest_rect.h = 124;
    dest_rect.x = (1280 / 2) - (dest_rect.w / 2); 
    dest_rect.y = (720 / 2) - (dest_rect.h / 2);  
}

void warning_update(void) {
    timer++;

    if (timer > 300) {
        state_manager_change(STATE_TITLE);
    }
}

void warning_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (tex_warning) {
        SDL_RenderCopy(renderer, tex_warning, NULL, &dest_rect);
    }
}

void warning_cleanup(void) {
    if (tex_warning) {
        SDL_DestroyTexture(tex_warning);
        tex_warning = NULL;
    }
}