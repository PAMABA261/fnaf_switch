#pragma once
#include <stdbool.h>
#include <switch.h>

// Una función para configurar los mandos al abrir el juego
void input_init(void);

// Una función que se ejecute 60 veces por segundo para "leer" el mando
void input_update(void);

// Una función que nos diga si queremos salir del juego (si pulsamos el botón '+')
bool input_should_quit(void);

// Función para preguntar si se acaba de pulsar un botón concreto
bool input_get_button_down(u64 button);

// Función para leer el valor del stick izquierdo (eje X)
s16 input_get_stick_x(int side);