#include "engine/input.h"
#include <switch.h>
#include <SDL2/SDL.h>

// Variable interna para guardar el estado del mando de la Switch
static PadState pad;
static bool quit_requested = false;

void input_init(void) {
    // Le decimos a la Switch que vamos a usar los mandos estándar (Joy-Cons o Pro Controller)
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
}

void input_update(void) {
    // LEER LOS MANDOS DE SWITCH
    padUpdate(&pad); 
    u64 kDown = padGetButtonsDown(&pad); 
    
    // Si se pulsa el botón '+', marcamos la bandera de salir como verdadera
    if (kDown & HidNpadButton_Plus) {
        quit_requested = true;
    }

    // LEER LA VENTANA DE SDL (Eventos de sistema)
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit_requested = true;
        }
    }
}

bool input_should_quit(void) {
    return quit_requested;
}

bool input_get_button_down(u64 button) {
    u64 kDown = padGetButtonsDown(&pad);
    return (kDown & button) != 0;
}

s16 input_get_stick_x(int side) { 
    // Si 'side' es 0, leerá el izquierdo. Si es 1, el derecho.
    return (s16)padGetStickPos(&pad, side).x;
}