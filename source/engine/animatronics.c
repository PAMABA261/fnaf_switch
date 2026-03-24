#include "game/animatronics.h"
#include "engine/audio.h"
#include <stdlib.h>

// --- ESTRUCTURA DEL ANIMATRÓNICO ---
typedef struct {
    int ai_level;
    int current_room;
    int move_timer;       // Cronómetro de los 4.97s
    int direction_timer;  // Cronómetro de 1s para cambiar de idea
    int current_dir;      // Dirección 1 o 2
    int current_pose;     // Pose fija al entrar en una sala (0 o 1)
} Animatronic;

static Animatronic bonnie;

// --- AUDIO ---
static Mix_Chunk* sfx_deep_steps = NULL;

// En FNAF, los pasos varían de volumen según lo cerca que estén.
static void play_footsteps(int volume) {
    if (sfx_deep_steps) {
        audio_set_sfx_volume(sfx_deep_steps, volume);
        audio_play_sfx_chunk(sfx_deep_steps);
    }
}

void animatronics_init(int night_number) {
    sfx_deep_steps = audio_load_sfx("romfs:/sfx/deep_steps.wav");

    // 1. ASIGNAR NIVEL DE IA SEGÚN LA NOCHE (Basado en el código de Scott)
    bonnie.ai_level = 0;
    switch (night_number) {
        case 1: bonnie.ai_level = 0; break;
        case 2: bonnie.ai_level = 3; break;
        case 3: bonnie.ai_level = 0; break;
        case 4: bonnie.ai_level = 2; break;
        case 5: bonnie.ai_level = 5; break;
        case 6: bonnie.ai_level = 10; break;
        case 7: bonnie.ai_level = 20; break; // Asumimos 20 para la Custom Night de momento
    }

    // 2. ESTADO INICIAL
    bonnie.current_room = CAM_1A; // Show Stage
    bonnie.move_timer = 0;
    bonnie.direction_timer = 0;
    bonnie.current_dir = 1;
    bonnie.current_pose = 0;
}

void animatronics_update(bool left_door_closed, bool right_door_closed, bool camera_is_open) {
    
    // --- 1. ACTUALIZAR DIRECCIÓN (Cada 1 segundo / 60 frames) ---
    bonnie.direction_timer++;
    if (bonnie.direction_timer >= 60) {
        bonnie.direction_timer = 0;
        bonnie.current_dir = (rand() % 2) + 1; // Genera 1 o 2
    }

    // --- 2. INTENTO DE MOVIMIENTO (Cada 4.97 segundos / 298 frames aprox) ---
    bonnie.move_timer++;
    if (bonnie.move_timer >= 298) {
        bonnie.move_timer = 0;

        // TIRADA DE DADO: ¿Se mueve?
        if ((rand() % 20) + 1 <= bonnie.ai_level) {
            
            int prev_room = bonnie.current_room;
            int dir = bonnie.current_dir;

            // RUTAS DE BONNIE (Traducción exacta del Clickteam)
            switch (bonnie.current_room) {
                case CAM_1A: // Show Stage
                    bonnie.current_room = (dir == 1) ? CAM_5 : CAM_1B;
                    play_footsteps(10);
                    break;
                
                case CAM_5: // Backstage
                    bonnie.current_room = (dir == 1) ? CAM_1B : CAM_2A;
                    play_footsteps(10);
                    break;
                
                case CAM_1B: // Dining Area
                    bonnie.current_room = (dir == 1) ? CAM_5 : CAM_2A;
                    play_footsteps(20);
                    break;
                
                case CAM_2A: // West Hall
                    bonnie.current_room = (dir == 1) ? CAM_3 : CAM_2B;
                    play_footsteps(30);
                    break;
                
                case CAM_2B: // West Hall Corner
                    bonnie.current_room = (dir == 1) ? CAM_3 : ROOM_DOOR_LEFT;
                    play_footsteps(40);
                    break;
                
                case CAM_3: // Supply Closet
                   bonnie.current_room = (dir == 1) ? CAM_2B : CAM_2A;
                    play_footsteps(30);
                    break;
                
                case ROOM_DOOR_LEFT: // Ready to attack left (Pasillo frente a la puerta)
                    if (left_door_closed) {
                        // Si la puerta está cerrada, se rinde y vuelve al Dining Area
                        bonnie.current_room = CAM_1B;
                        play_footsteps(30);
                    } else {
                        // Si la puerta está abierta... ¡Entra en la oficina!
                        bonnie.current_room = ROOM_OFFICE;
                    }
                    break;
            }

            // --- LÓGICA DE POSE ESTÁTICA ---
            // Si Bonnie se acaba de mover a una sala nueva, elige una pose y se la queda
            if (prev_room != bonnie.current_room) {
                bonnie.current_pose = rand() % 2; // Genera 0 o 1
            }
        }
    }
}

void animatronics_on_hour_changed(int new_hour) {
    // A las 2 AM, solo sube Bonnie
    if (new_hour == 2) {
        bonnie.ai_level++;
    }
    // A las 3 AM y 4 AM, suben Bonnie, Chica y Foxy
    else if (new_hour == 3 || new_hour == 4) {
        bonnie.ai_level++;
        // chica.ai_level++; // Lo descomentaremos cuando programemos a Chica
        // foxy.ai_level++;  // Lo descomentaremos cuando programemos a Foxy
    }
}

void animatronics_cleanup(void) {
    if (sfx_deep_steps) {
        audio_free_sfx(sfx_deep_steps);
        sfx_deep_steps = NULL;
    }
}

int animatronics_get_bonnie_room(void) {
    return bonnie.current_room;
}

// Devuelve la pose fija que eligió al entrar en la sala
int animatronics_get_bonnie_pose(void) {
    return bonnie.current_pose;
}