#include <switch.h>
#include "engine/input.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include "game/state_manager.h" 

int main(int argc, char* argv[]) {

    consoleDebugInit(debugDevice_SVC);

    // 1. SISTEMA DE ARCHIVOS (Lo primero siempre, para poder leer datos)
    romfsInit(); 

    // 2. HARDWARE (Mandos, Pantalla, Chip de Sonido)
    input_init();
    
    // Si el hardware falla, cerramos todo y salimos
    if (!graphics_init() || !audio_init()) {
        graphics_cleanup();
        audio_cleanup();
        romfsExit();
        return -1;
    }

    // 3. EL JUEGO (Cargar el primer estado)
    // Ahora que el hardware está listo, llamamos al Warning
    state_manager_init(STATE_WARNING);

    // 4. EL BUCLE INFINITO (Update -> Draw)
    while (appletMainLoop()) {
        
        input_update();
        
        if (input_should_quit()) {
            break; 
        }

        // Primero calculamos lógica
        state_manager_update();

        // Luego dibujamos (Borrar -> Dibujar -> Mostrar)
        graphics_clear();
        state_manager_draw();
        graphics_present();
    }

    // 5. APAGADO ORDENADO (En orden inverso al encendido)
    state_manager_cleanup();
    audio_cleanup();
    graphics_cleanup();
    romfsExit();   
    
    return 0;
}