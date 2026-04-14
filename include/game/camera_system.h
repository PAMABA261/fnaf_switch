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

// Funciones de dibujado por capas
void camera_system_draw_room(void);      // Capa de fondo (Habitación y paneo)
void camera_system_draw_ui(void);        // Capa del minimapa, botones y REC
void camera_system_draw_button(void);    // Botón de subir/bajar tableta
void camera_system_draw_animation(void); // Animación de subir/bajar tableta

void camera_system_cleanup(void);

void camera_system_toggle(void);
void camera_system_force_close(void);
bool camera_system_is_open(void);
float camera_system_get_frame(void);
int camera_system_get_current_cam(void);