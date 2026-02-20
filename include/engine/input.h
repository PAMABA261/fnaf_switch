#pragma once
#include <stdbool.h>
#include <switch.h>

// 1. Una función para configurar los mandos al abrir el juego
void input_init(void);

// 2. Una función que se ejecute 60 veces por segundo para "leer" el mando
void input_update(void);

// 3. Una función que nos diga si queremos salir del juego (si pulsamos el botón '+')
bool input_should_quit(void);

// 4. Función para preguntar si se acaba de pulsar un botón concreto
bool input_get_button_down(u64 button);