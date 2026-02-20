#pragma once
#include <stdbool.h>
#include <SDL2/SDL.h>

// Inicializa la ventana y el motor gráfico
bool graphics_init(void);

// Limpia la memoria al cerrar el juego
void graphics_cleanup(void);

// Pinta la pantalla de negro 
void graphics_clear(void);

// Muestra en pantalla lo que hemos dibujado
void graphics_present(void);

// Carga una imagen y la devuelve como una textura de SDL
SDL_Texture* graphics_load_texture(const char* path);

// Devuelve el renderer para que otros módulos puedan dibujar cosas
SDL_Renderer* graphics_get_renderer(void);