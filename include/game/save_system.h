#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

// Carga la noche guardada. Si no hay partida, la pone a 1.
void save_system_load(void);

// Guarda la noche actual en el archivo.
void save_system_save(void);

#endif 