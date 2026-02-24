#include "engine/graphics.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Variables internas 
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

bool graphics_init(void) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // Iniciamos el núcleo de vídeo de SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    
    // Iniciamos explícitamente el soporte para imágenes PNG
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) return false;
    
    // Creamos la ventana y el motor de renderizado
    window = SDL_CreateWindow("Five Nights at Freddy's", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    if (!window) return false;
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    // Ajustamos la escala para mantener la relación de aspecto original
    SDL_RenderSetLogicalSize(renderer, 1280, 720);
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    
    return true;
}

void graphics_cleanup(void) {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    
    IMG_Quit();
    SDL_Quit();
}

void graphics_clear(void) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void graphics_present(void) {
    SDL_RenderPresent(renderer);
}

SDL_Texture* graphics_load_texture(const char* path) {
    // Cargamos la imagen en una "superficie" 
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (!loadedSurface) {
        return NULL; 
    }

    // La convertimos en "textura" 
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    
    // Liberamos la superficie de la RAM porque ya no la necesitamos
    SDL_FreeSurface(loadedSurface);

    if (texture) {
        SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest); // Evitamos el suavizado para que se vea pixelado
    }

    return texture;
}

SDL_Renderer* graphics_get_renderer(void) {
    return renderer;
}