#ifndef ANIMATRONICS_H
#define ANIMATRONICS_H

#include <stdbool.h>
#include "game/camera_system.h" 

// Definimos salas especiales que no son cámaras
#define ROOM_DOOR_LEFT  99  // "ready to attack left"
#define ROOM_OFFICE     100 // "got you left"

void animatronics_init(int night_number);
void animatronics_update(bool left_door_closed, bool right_door_closed, bool camera_is_open);
void animatronics_on_hour_changed(int new_hour);
void animatronics_cleanup(void);

// Getters para saber dónde están
int animatronics_get_bonnie_room(void);

int animatronics_get_bonnie_pose(void);

#endif