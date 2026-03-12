#pragma once
#include <stdbool.h>

#define CAM_FRAMES 11

typedef enum {
    CAM_1A = 0, CAM_1B, CAM_1C,
    CAM_2A, CAM_2B, CAM_3,
    CAM_4A, CAM_4B, CAM_5,
    CAM_6, CAM_7
} CameraID;

void camera_system_init(void);
void camera_system_update(void);

// Funciones de dibujado (separadas en capas para que game.c las intercale)
void camera_system_draw_room(void);      // Se dibuja AL FONDO (cuando la cámara tapa la oficina)
void camera_system_draw_button(void);    // Se dibuja el botón gris de la UI
void camera_system_draw_animation(void); // Se dibuja POR ENCIMA (animación de subir/bajar)

void camera_system_cleanup(void);

// Interacciones desde game.c
void camera_system_toggle(void);         // Llamada cuando pulsamos 'A'
void camera_system_force_close(void);    // Llamada cuando se va la luz
bool camera_system_is_open(void);        // Para saber si gasta batería
float camera_system_get_frame(void);     // Para tapar/destapar los relojes del HUD