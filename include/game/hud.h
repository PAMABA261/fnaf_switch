#pragma once
#include <stdbool.h>

// Variables globales que game.c necesita leer para saber si hemos ganado
extern int current_night;
extern int current_hour;

// Funciones principales del sistema HUD
void hud_init(void);
void hud_update(void);
void hud_draw(bool cam_open, float cam_frame); 
void hud_cleanup(void);