#include "game/animatronics.h"
#include "game/camera_system.h" // --- AÑADIDO: Necesario para saber qué cámara miras
#include "engine/audio.h"
#include <stdlib.h>

// --- ESTRUCTURA DEL ANIMATRÓNICO ---
typedef struct {
    int ai_level;
    int current_room;
    int prev_room;
    int move_timer;       
    int direction_timer;  
    int current_dir;      
    int current_pose;     
    int just_moved_timer; 
} Animatronic;

static Animatronic bonnie;
static Animatronic chica; 
static Animatronic freddy; 

static Mix_Chunk* sfx_deep_steps = NULL;

static void play_footsteps(int volume) {
    if (sfx_deep_steps) {
        audio_set_sfx_volume(sfx_deep_steps, volume);
        audio_play_sfx_chunk(sfx_deep_steps);
    }
}

void animatronics_init(int night_number) {
    sfx_deep_steps = audio_load_sfx("romfs:/sfx/deep_steps.wav");

    bonnie.ai_level = 0;
    chica.ai_level = 0;
    freddy.ai_level = 0; 
    
    switch (night_number) {
        case 1: bonnie.ai_level = 0; chica.ai_level = 0; freddy.ai_level = 0; break;
        case 2: bonnie.ai_level = 3; chica.ai_level = 1; freddy.ai_level = 0; break;
        case 3: bonnie.ai_level = 0; chica.ai_level = 5; freddy.ai_level = 1; break;
        case 4: bonnie.ai_level = 2; chica.ai_level = 4; freddy.ai_level = 2; break;
        case 5: bonnie.ai_level = 5; chica.ai_level = 7; freddy.ai_level = 3; break;
        case 6: bonnie.ai_level = 10; chica.ai_level = 12; freddy.ai_level = 4; break;
        case 7: bonnie.ai_level = 20; chica.ai_level = 20; freddy.ai_level = 20; break; 
    }

    bonnie.current_room = CAM_1A;
    bonnie.prev_room = CAM_1A;
    bonnie.move_timer = 0; bonnie.direction_timer = 0;
    bonnie.current_dir = 1; bonnie.current_pose = 0;
    bonnie.just_moved_timer = 0;

    chica.current_room = CAM_1A;
    chica.prev_room = CAM_1A;
    chica.move_timer = 0; chica.direction_timer = 0;
    chica.current_dir = 1; chica.current_pose = 0;
    chica.just_moved_timer = 0;

    // --- AÑADIDO: Inicialización de Freddy ---
    freddy.current_room = CAM_1A;
    freddy.prev_room = CAM_1A;
    freddy.move_timer = 0; 
    freddy.direction_timer = 0; // (No lo usa, pero por si acaso)
    freddy.current_dir = 1;     // (No lo usa)
    freddy.current_pose = 0;    // (No lo usa)
    freddy.just_moved_timer = 0;
}

void animatronics_update(bool left_door_closed, bool right_door_closed, bool camera_is_open) {
    
    if (bonnie.just_moved_timer > 0) bonnie.just_moved_timer--;
    if (chica.just_moved_timer > 0) chica.just_moved_timer--;
    if (freddy.just_moved_timer > 0) freddy.just_moved_timer--; // --- AÑADIDO

    // ========================================================================
    // LÓGICA DE BONNIE
    // ========================================================================
    bonnie.direction_timer++;
    if (bonnie.direction_timer >= 60) {
        bonnie.direction_timer = 0;
        bonnie.current_dir = (rand() % 2) + 1; 
    }

    bonnie.move_timer++;
    if (bonnie.move_timer >= 298) {
        bonnie.move_timer = 0;

        if ((rand() % 20) + 1 <= bonnie.ai_level) {
            int prev_room = bonnie.current_room;
            int dir = bonnie.current_dir;

            switch (bonnie.current_room) {
                case CAM_1A: bonnie.current_room = (dir == 1) ? CAM_5 : CAM_1B; play_footsteps(10); break;
                case CAM_5:  bonnie.current_room = (dir == 1) ? CAM_1B : CAM_2A; play_footsteps(10); break;
                case CAM_1B: bonnie.current_room = (dir == 1) ? CAM_5 : CAM_2A; play_footsteps(20); break;
                case CAM_2A: bonnie.current_room = (dir == 1) ? CAM_3 : CAM_2B; play_footsteps(30); break;
                case CAM_2B: bonnie.current_room = (dir == 1) ? CAM_3 : ROOM_DOOR_LEFT; play_footsteps(40); break;
                case CAM_3:  bonnie.current_room = (dir == 1) ? CAM_2B : CAM_2A; play_footsteps(30); break;
                case ROOM_DOOR_LEFT:
                    if (left_door_closed) { 
                        bonnie.current_room = CAM_1B; 
                        play_footsteps(30); 
                    } else { 
                        bonnie.current_room = ROOM_OFFICE; 
                    }
                    break;
            }

            if (prev_room != bonnie.current_room) {
                bonnie.current_pose = rand() % 2; 
                bonnie.just_moved_timer = 10; 
                bonnie.prev_room = prev_room; 
            }
        }
    }

    // ========================================================================
    // LÓGICA DE CHICA
    // ========================================================================
    chica.direction_timer++;
    if (chica.direction_timer >= 60) {
        chica.direction_timer = 0;
        chica.current_dir = (rand() % 2) + 1; 
    }

    chica.move_timer++;
    if (chica.move_timer >= 299) {
        chica.move_timer = 0;

        if ((rand() % 20) + 1 <= chica.ai_level) {
            int prev_room = chica.current_room;
            int dir = chica.current_dir;

            switch (chica.current_room) {
                case CAM_1A: chica.current_room = CAM_1B; play_footsteps(10); break;
                case CAM_1B: chica.current_room = (dir == 1) ? CAM_7 : CAM_6; play_footsteps(10); break;
                case CAM_6:  chica.current_room = (dir == 1) ? CAM_7 : CAM_4A; play_footsteps((dir == 1) ? 10 : 20); break;
                case CAM_7:  chica.current_room = (dir == 1) ? CAM_6 : CAM_4A; play_footsteps((dir == 1) ? 20 : 20); break;
                case CAM_4A: chica.current_room = (dir == 1) ? CAM_1B : CAM_4B; play_footsteps(30); break;
                case CAM_4B: chica.current_room = (dir == 1) ? CAM_4A : ROOM_DOOR_RIGHT; play_footsteps(40); break;
                case ROOM_DOOR_RIGHT:
                    if (right_door_closed) { 
                        chica.current_room = CAM_4A; 
                        play_footsteps(40); 
                    } else { 
                        chica.current_room = ROOM_OFFICE; 
                    }
                    break;
            }

            if (prev_room != chica.current_room) {
                chica.current_pose = rand() % 2; 
                chica.just_moved_timer = 10; 
                chica.prev_room = prev_room;
            }
        }
    }

    // ========================================================================
    // LÓGICA DE FREDDY
    // ========================================================================
    if (freddy.ai_level > 0) {
        // Camera Stalling: Si lo miras, se congela.
        if (camera_is_open && camera_system_get_current_cam() == freddy.current_room) {
            freddy.move_timer = 0; 
        } else {
            freddy.move_timer++;
            
            // Fórmula exacta del código original: 1000 - (Freddy AI * 100)
            int move_limit = 1000 - (freddy.ai_level * 100);
            if (move_limit < 0) move_limit = 0; // Por si subimos el nivel a 20

            if (freddy.move_timer >= move_limit) {
                freddy.move_timer = 0;
                int prev_room = freddy.current_room;

                // Ruta Fija de Freddy
                if (freddy.current_room == CAM_1A && bonnie.current_room != CAM_1A && chica.current_room != CAM_1A) {
                    freddy.current_room = CAM_1B;
                } else if (freddy.current_room == CAM_1B) {
                    freddy.current_room = CAM_7;
                } else if (freddy.current_room == CAM_7) {
                    freddy.current_room = CAM_6;
                } else if (freddy.current_room == CAM_6) {
                    freddy.current_room = CAM_4A;
                } else if (freddy.current_room == CAM_4A) {
                    freddy.current_room = CAM_4B;
                } else if (freddy.current_room == CAM_4B) {
                    // Intenta entrar. Si la puerta está cerrada, huye a la 4A.
                    if (right_door_closed) {
                        freddy.current_room = CAM_4A; 
                    } else {
                        // NO puede entrar si estás mirando la cámara 4B
                        if (!camera_is_open || camera_system_get_current_cam() != CAM_4B) {
                            freddy.current_room = ROOM_OFFICE;
                        }
                    }
                }

                if (freddy.current_room != prev_room) {
                    freddy.prev_room = prev_room;
                    freddy.just_moved_timer = 10; // Lo usamos en el update para lanzar su risa
                }
            }
        }
    }
}

void animatronics_on_hour_changed(int new_hour) {
    if (new_hour == 2) { bonnie.ai_level++; }
    else if (new_hour == 3 || new_hour == 4) { 
        bonnie.ai_level++; 
        chica.ai_level++; 
        freddy.ai_level++; // --- AÑADIDO: Freddy también sube a las 3 y a las 4 AM
    }
}

void animatronics_cleanup(void) {
    if (sfx_deep_steps) { audio_free_sfx(sfx_deep_steps); sfx_deep_steps = NULL; }
}

int animatronics_get_bonnie_room(void) { return bonnie.current_room; }
int animatronics_get_bonnie_pose(void) { return bonnie.current_pose; }
int animatronics_get_bonnie_moved_timer(void) { return bonnie.just_moved_timer; }
int animatronics_get_bonnie_prev_room(void) { return bonnie.prev_room; }

int animatronics_get_chica_room(void) { return chica.current_room; }
int animatronics_get_chica_pose(void) { return chica.current_pose; }
int animatronics_get_chica_moved_timer(void) { return chica.just_moved_timer; }
int animatronics_get_chica_prev_room(void) { return chica.prev_room; }

// --- AÑADIDOS LOS GETTERS DE FREDDY ---
int animatronics_get_freddy_room(void) { return freddy.current_room; }
int animatronics_get_freddy_prev_room(void) { return freddy.prev_room; }
int animatronics_get_freddy_moved_timer(void) { return freddy.just_moved_timer; }