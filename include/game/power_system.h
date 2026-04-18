#pragma once
#include <stdbool.h>

#define JUMPSCARE_FRAMES 21

// Variables globales que game.c necesita leer para dibujar o cambiar estados
extern int power_left;
extern bool is_power_out;
extern int powerout_state;
extern bool show_freddy;

void power_system_init(void);

// Le pasamos cuántas cosas están encendidas (puertas, luces, cámaras) 
// Devuelve 'true' en el frame exacto en el que la energía llega a 0
bool power_system_update(int items_on); 

void power_system_draw_hud(bool cam_open, float cam_frame);
void power_system_draw_jumpscare(void);
void power_system_cleanup(void);
void power_system_subtract_power(int amount);