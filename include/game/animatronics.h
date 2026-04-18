#ifndef ANIMATRONICS_H
#define ANIMATRONICS_H

#include <stdbool.h>
#include "game/camera_system.h" 

// Definimos salas especiales que no son cámaras
#define ROOM_DOOR_LEFT  99  // "ready to attack left" (Bonnie)
#define ROOM_OFFICE     100 // "got you left" / "got you right" (Oficina)
#define ROOM_DOOR_RIGHT 101 // "ready to attack right" (Chica)

void animatronics_init(int night_number);
void animatronics_update(bool left_door_closed, bool right_door_closed, bool camera_is_open);
void animatronics_on_hour_changed(int new_hour);
void animatronics_cleanup(void);

// Getters para saber dónde están
int animatronics_get_bonnie_room(void);
int animatronics_get_bonnie_pose(void);
int animatronics_get_bonnie_moved_timer(void);
int animatronics_get_bonnie_prev_room(void);

int animatronics_get_chica_room(void);
int animatronics_get_chica_pose(void);
int animatronics_get_chica_moved_timer(void);
int animatronics_get_chica_prev_room(void);

int animatronics_get_freddy_room(void);
int animatronics_get_freddy_prev_room(void);
int animatronics_get_freddy_moved_timer(void);

int animatronics_get_foxy_state(void);
bool animatronics_get_foxy_just_banged(void);
int animatronics_get_foxy_bang_count(void);
void animatronics_trigger_foxy_run(void);
#endif