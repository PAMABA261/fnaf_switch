#include <switch.h>
#include "engine/input.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include "game/state_manager.h" 

int main(int argc, char* argv[]) {

    // ABRIR EL DISCO DURO DE LA SWITCH (Sino crashea al intentar cargar cualquier archivo)
    romfsInit(); 

    // Inicialización de la Switch
    input_init();
    
    if (!graphics_init() || !audio_init()) {
        graphics_cleanup();
        audio_cleanup();
        romfsExit();
        return -1;
    }

    // Inicialización del gestor de estados, empezando en el título
    state_manager_init(STATE_TITLE);

    // El Bucle Principal
    while (appletMainLoop()) {
        
        input_update();
        
        if (input_should_quit()) {
            break; 
        }

        // El main le pregunta al gestor que lógica ejecutar
        state_manager_update();

        // El main le pregunta al gestor que limpie la pantalla 
        graphics_clear();

        // El main le pregunta al gestor que dibuje lo que quiera en pantalla
        state_manager_draw();
        
        // Una vez el gestor ha dibujado lo que quería, mostramos el resultado
        graphics_present();
    }

    // Limpieza final al salir
    state_manager_cleanup();
    audio_cleanup();
    graphics_cleanup();

    // CERRAR EL DISCO DURO AL SALIR
    romfsExit();   
    
    return 0;
}