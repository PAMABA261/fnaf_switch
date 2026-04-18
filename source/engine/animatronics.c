#include "game/animatronics.h"
#include "game/camera_system.h"
#include "engine/audio.h"
#include <stdlib.h>

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

// --- Variables de Foxy ---
static int foxy_ai_level = 0;
static int foxy_state = 0;        // 0 a 5 (Pirate Cove -> Pasillo -> Puerta -> Jumpscare)
static int foxy_move_timer = 0;
static int foxy_action_timer = 0;
static int foxy_bang_count = 0;
static bool foxy_just_banged = false;

static Mix_Chunk* sfx_deep_steps = NULL;

// Niveles de IA por noche: {bonnie, chica, freddy, foxy}
static const int AI_LEVELS[8][4] = {
    {0,  0,  0,  0},  // noche 0 (no usada)
    {0,  0,  0,  0},  // noche 1
    {3,  1,  0,  1},  // noche 2
    {0,  5,  1,  2},  // noche 3
    {2,  4,  2,  6},  // noche 4
    {5,  7,  3,  5},  // noche 5
    {10, 12, 4,  6},  // noche 6
    {20, 20, 20, 20}, // noche 7
};

static void play_footsteps(int volume) {
    if (!sfx_deep_steps) return;
    audio_set_sfx_volume(sfx_deep_steps, volume);
    audio_play_sfx_chunk(sfx_deep_steps);
}

static void init_animatronic(Animatronic* a, int ai) {
    a->ai_level = ai;
    a->current_room = a->prev_room = CAM_1A;
    a->move_timer = a->direction_timer = 0;
    a->current_dir = 1;
    a->current_pose = a->just_moved_timer = 0;
}

static void tick_direction(Animatronic* a) {
    if (++a->direction_timer >= 60) {
        a->direction_timer = 0;
        a->current_dir = (rand() % 2) + 1;
    }
}

static void on_moved(Animatronic* a, int prev_room) {
    if (a->current_room != prev_room) {
        a->current_pose = rand() % 2;
        a->just_moved_timer = 10;
        a->prev_room = prev_room;
    }
}

static void update_bonnie(bool left_door_closed) {
    tick_direction(&bonnie);
    if (++bonnie.move_timer < 298) return;
    bonnie.move_timer = 0;
    if ((rand() % 20) + 1 > bonnie.ai_level) return;

    int prev = bonnie.current_room;
    int d = bonnie.current_dir;
    switch (bonnie.current_room) {
        case CAM_1A: bonnie.current_room = (d==1) ? CAM_5   : CAM_1B; play_footsteps(10); break;
        case CAM_5:  bonnie.current_room = (d==1) ? CAM_1B  : CAM_2A; play_footsteps(10); break;
        case CAM_1B: bonnie.current_room = (d==1) ? CAM_5   : CAM_2A; play_footsteps(20); break;
        case CAM_2A: bonnie.current_room = (d==1) ? CAM_3   : CAM_2B; play_footsteps(30); break;
        case CAM_2B: bonnie.current_room = (d==1) ? CAM_3   : ROOM_DOOR_LEFT; play_footsteps(40); break;
        case CAM_3:  bonnie.current_room = (d==1) ? CAM_2B  : CAM_2A; play_footsteps(30); break;
        case ROOM_DOOR_LEFT:
            if (left_door_closed) { bonnie.current_room = CAM_1B; play_footsteps(30); }
            else                    bonnie.current_room = ROOM_OFFICE;
            break;
    }
    on_moved(&bonnie, prev);
}

static void update_chica(bool right_door_closed) {
    tick_direction(&chica);
    if (++chica.move_timer < 299) return;
    chica.move_timer = 0;
    if ((rand() % 20) + 1 > chica.ai_level) return;

    int prev = chica.current_room;
    int d = chica.current_dir;
    switch (chica.current_room) {
        case CAM_1A: chica.current_room = CAM_1B; play_footsteps(10); break;
        case CAM_1B: chica.current_room = (d==1) ? CAM_7  : CAM_6;  play_footsteps(10); break;
        case CAM_6:  chica.current_room = (d==1) ? CAM_7  : CAM_4A; play_footsteps((d==1)?10:20); break;
        case CAM_7:  chica.current_room = (d==1) ? CAM_6  : CAM_4A; play_footsteps(20); break;
        case CAM_4A: chica.current_room = (d==1) ? CAM_1B : CAM_4B; play_footsteps(30); break;
        case CAM_4B: chica.current_room = (d==1) ? CAM_4A : ROOM_DOOR_RIGHT; play_footsteps(40); break;
        case ROOM_DOOR_RIGHT:
            if (right_door_closed) { chica.current_room = CAM_4A; play_footsteps(40); }
            else                     chica.current_room = ROOM_OFFICE;
            break;
    }
    on_moved(&chica, prev);
}

static void update_freddy(bool right_door_closed, bool camera_is_open) {
    if (freddy.ai_level == 0) return;

    if (camera_is_open && camera_system_get_current_cam() == freddy.current_room) {
        freddy.move_timer = 0;
        return;
    }

    int limit = 1000 - (freddy.ai_level * 100);
    if (limit < 0) limit = 0;
    if (++freddy.move_timer < limit) return;
    freddy.move_timer = 0;

    int prev = freddy.current_room;
    if      (freddy.current_room == CAM_1A && bonnie.current_room != CAM_1A && chica.current_room != CAM_1A)
        freddy.current_room = CAM_1B;
    else if (freddy.current_room == CAM_1B) freddy.current_room = CAM_7;
    else if (freddy.current_room == CAM_7)  freddy.current_room = CAM_6;
    else if (freddy.current_room == CAM_6)  freddy.current_room = CAM_4A;
    else if (freddy.current_room == CAM_4A) freddy.current_room = CAM_4B;
    else if (freddy.current_room == CAM_4B) {
        if (right_door_closed)
            freddy.current_room = CAM_4A;
        else if (!camera_is_open || camera_system_get_current_cam() != CAM_4B)
            freddy.current_room = ROOM_OFFICE;
    }
    on_moved(&freddy, prev);
}

static void update_foxy(bool left_door_closed, bool camera_is_open) {
    foxy_just_banged = false; // Se resetea cada tick

    if (foxy_state < 3) {
        foxy_move_timer++;
        // 300 frames = 5 segundos
        if (foxy_move_timer >= 300) {
            foxy_move_timer = 0;
            
            // Foxy solo tira los dados si NO estás mirando la CAM 1C
            bool looking_at_cove = (camera_is_open && camera_system_get_current_cam() == CAM_1C);
            
            if (!looking_at_cove) {
                if ((rand() % 20) + 1 <= foxy_ai_level) {
                    foxy_state++;
                }
            }
        }
    } 
    else if (foxy_state == 3) {
        // Foxy ya no está en la cortina. Cuenta atrás oculta.
        foxy_action_timer++;
        
        // Si pasan 1500 frames (25 segundos) sin mirar el pasillo 2A, ataca
        if (foxy_action_timer >= 1500) {
            foxy_state = 5; 
            foxy_action_timer = 0;
        }
    }
    else if (foxy_state == 4) {
        // Foxy llegó a la puerta tras la animación de correr
        foxy_action_timer++;
        
        // Espera 100 frames (~1.6 segundos) en la puerta izquierda
        if (foxy_action_timer >= 100) {
            if (left_door_closed) {
                // Bloqueado: Golpea la puerta
                foxy_state = rand() % 2; // Vuelve al estado 0 o 1
                foxy_just_banged = true; // Flag para reproducir sonido y restar luz en game.c
                foxy_bang_count++;
            } else {
                // Puerta abierta: Jumpscare
                foxy_state = 5;
            }
            foxy_action_timer = 0;
        }
    }
}

// ── PUBLIC API ────────────────────────────────────────────────────────────────

void animatronics_init(int night_number) {
    sfx_deep_steps = audio_load_sfx("romfs:/sfx/deep_steps.wav");
    int n = (night_number >= 1 && night_number <= 7) ? night_number : 0;
    
    init_animatronic(&bonnie,  AI_LEVELS[n][0]);
    init_animatronic(&chica,   AI_LEVELS[n][1]);
    init_animatronic(&freddy,  AI_LEVELS[n][2]);
    
    foxy_ai_level = AI_LEVELS[n][3];
    foxy_state = 0;
    foxy_move_timer = 0;
    foxy_action_timer = 0;
    foxy_bang_count = 0;
    foxy_just_banged = false;
}

void animatronics_update(bool left_door_closed, bool right_door_closed, bool camera_is_open) {
    if (bonnie.just_moved_timer > 0) bonnie.just_moved_timer--;
    if (chica.just_moved_timer  > 0) chica.just_moved_timer--;
    if (freddy.just_moved_timer > 0) freddy.just_moved_timer--;

    update_bonnie(left_door_closed);
    update_chica(right_door_closed);
    update_freddy(right_door_closed, camera_is_open);
    update_foxy(left_door_closed, camera_is_open);
}

void animatronics_on_hour_changed(int new_hour) {
    if (new_hour == 2) { bonnie.ai_level++; }
    else if (new_hour == 3 || new_hour == 4) {
        bonnie.ai_level++;
        chica.ai_level++;
        freddy.ai_level++;
        foxy_ai_level++; 
    }
}

void animatronics_cleanup(void) {
    if (sfx_deep_steps) { audio_free_sfx(sfx_deep_steps); sfx_deep_steps = NULL; }
}

int animatronics_get_bonnie_room(void)         { return bonnie.current_room; }
int animatronics_get_bonnie_pose(void)         { return bonnie.current_pose; }
int animatronics_get_bonnie_moved_timer(void)  { return bonnie.just_moved_timer; }
int animatronics_get_bonnie_prev_room(void)    { return bonnie.prev_room; }

int animatronics_get_chica_room(void)          { return chica.current_room; }
int animatronics_get_chica_pose(void)          { return chica.current_pose; }
int animatronics_get_chica_moved_timer(void)   { return chica.just_moved_timer; }
int animatronics_get_chica_prev_room(void)     { return chica.prev_room; }

int animatronics_get_freddy_room(void)         { return freddy.current_room; }
int animatronics_get_freddy_prev_room(void)    { return freddy.prev_room; }
int animatronics_get_freddy_moved_timer(void)  { return freddy.just_moved_timer; }

int animatronics_get_foxy_state(void)          { return foxy_state; }
bool animatronics_get_foxy_just_banged(void)   { return foxy_just_banged; }
int animatronics_get_foxy_bang_count(void)     { return foxy_bang_count; }

void animatronics_trigger_foxy_run(void) {
    // Si la cámara 2A lo pilla escapado (estado 3), se lanza a correr hacia la puerta (estado 4)
    if (foxy_state == 3) {
        foxy_state = 4;
        foxy_action_timer = 0; // Reseteamos para que tarde exactamente 100 frames en golpear
    }
}