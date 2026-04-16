#include "game/assets.h"
#include "game/state_manager.h"
#include "game/hud.h"
#include "game/power_system.h"
#include "game/camera_system.h"
#include "game/animatronics.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/audio.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// Entorno de la Oficina
static float camera_x = 160.0f;
static SDL_Texture* tex_office_normal = NULL;
static SDL_Texture* tex_office_light_L = NULL;
static SDL_Texture* tex_office_light_R = NULL;
static SDL_Texture* tex_fan[3] = {NULL};
static SDL_Texture* tex_office_blackout = NULL;
static SDL_Texture* tex_office_blackout_freddy = NULL;
static SDL_Texture* tex_office_bonnie = NULL; 
static SDL_Texture* tex_office_chica = NULL;
static int fan_frame = 0;
static int fan_timer = 0;
#define FAN_ANIM_SPEED 1

// Puertas y Botones
#define DOOR_FRAMES 15
static const float DOOR_ANIM_SPEED = 0.75f;
static float door_L_frame = 0.0f;
static float door_R_frame = 0.0f;
static bool left_door_on = false;
static bool right_door_on = false;
static bool left_light_on = false;
static bool right_light_on = false;
static SDL_Texture* tex_door_L_close[DOOR_FRAMES] = {NULL};
static SDL_Texture* tex_door_R_close[DOOR_FRAMES] = {NULL};
static SDL_Texture* tex_button_L[4] = {NULL};
static SDL_Texture* tex_button_R[4] = {NULL};

// Victoria
static bool is_winning = false;
static float win_fade = 0.0f;

// Alucinaciones
static SDL_Texture* tex_hallucinations[4] = {NULL};
static int hallucination_timer = 0;      
static int current_hallucination = -1;
static int random_sound_timer = 0; 
static int hallucination_hold_timer = 0;

// Audio de la oficina
static Mix_Chunk* sfx_fan = NULL;
static Mix_Chunk* sfx_light = NULL;
static Mix_Chunk* sfx_door = NULL;
static int channel_light_L = -1;
static int channel_light_R = -1;
static int channel_fan = -1; 
static int channel_breath = -1;
static int channel_circus = -1;

static Mix_Chunk* sfx_circus = NULL;
static Mix_Chunk* sfx_pounding = NULL;
static Mix_Chunk* sfx_hallucination[4] = {NULL};
static Mix_Chunk* sfx_window_scare = NULL; 
static Mix_Chunk* sfx_error = NULL;
static Mix_Chunk* sfx_steps = NULL;

static Mix_Chunk* sfx_breath[4] = {NULL}; 
static int breath_timer = 0; 
static bool bonnie_scare_played = false; 
static bool chica_scare_played = false; 

// Sonidos de la Cocina
static Mix_Chunk* sfx_kitchen[4] = {NULL};
static int kitchen_timer = 0;
static int channel_kitchen = -1; 

// --- LÓGICA DEL JUMPSCARE ---
#define JUMPSCARE_BONNIE_FRAMES 11
#define JUMPSCARE_CHICA_FRAMES 16 
#define JUMPSCARE_FREDDY_FRAMES 31

static SDL_Texture* tex_bonnie_jumpscare[JUMPSCARE_BONNIE_FRAMES] = {NULL};
static SDL_Texture* tex_chica_jumpscare[JUMPSCARE_CHICA_FRAMES] = {NULL}; 
static SDL_Texture* tex_freddy_jumpscare[JUMPSCARE_FREDDY_FRAMES] = {NULL}; 

static Mix_Chunk* sfx_jumpscare = NULL;

static bool is_bonnie_jumpscare = false;
static bool is_chica_jumpscare = false; 
static bool is_freddy_jumpscare = false; 

static float bonnie_jumpscare_frame = 0.0f;
static float chica_jumpscare_frame = 0.0f; 
static float freddy_jumpscare_frame = 0.0f; 

static const float JUMPSCARE_ANIM_SPEED = 0.75f; 
static int jumpscare_duration_timer = 0;

static int bonnie_force_down_timer = 0; 
static int chica_force_down_timer = 0; 

static Mix_Chunk* sfx_freddy_laugh[3] = {NULL};
static Mix_Chunk* sfx_running_fast = NULL;


void game_init(void) {
    Mix_AllocateChannels(32); 

    hud_init(); 
    power_system_init(); 
    camera_system_init(); 
    animatronics_init(current_night);

    tex_office_normal = graphics_load_texture(IMG_OFFICE);
    tex_office_light_L = graphics_load_texture(IMG_OFFICE_LIGHT_L);
    tex_office_light_R = graphics_load_texture(IMG_OFFICE_LIGHT_R);
    tex_fan[0] = graphics_load_texture(IMG_FAN_1);
    tex_fan[1] = graphics_load_texture(IMG_FAN_2);
    tex_fan[2] = graphics_load_texture(IMG_FAN_3);
    tex_office_blackout = graphics_load_texture(IMG_OFFICE_BLACK_OUT);
    tex_office_blackout_freddy = graphics_load_texture(IMG_OFFICE_BLACK_OUT_FREDDY);
    tex_office_bonnie = graphics_load_texture(IMG_OFFICE_BONNIE); 
    tex_office_chica = graphics_load_texture(IMG_OFFICE_CHICA); 

    tex_button_L[0] = graphics_load_texture(IMG_BUTTON_L_1);
    tex_button_L[1] = graphics_load_texture(IMG_BUTTON_L_2);
    tex_button_L[2] = graphics_load_texture(IMG_BUTTON_L_3);
    tex_button_L[3] = graphics_load_texture(IMG_BUTTON_L_4);
    tex_button_R[0] = graphics_load_texture(IMG_BUTTON_R_1);
    tex_button_R[1] = graphics_load_texture(IMG_BUTTON_R_2);
    tex_button_R[2] = graphics_load_texture(IMG_BUTTON_R_3);
    tex_button_R[3] = graphics_load_texture(IMG_BUTTON_R_4);

    const char* paths_door_L[DOOR_FRAMES] = {
        IMG_DOOR_L_CLOSE_1, IMG_DOOR_L_CLOSE_2, IMG_DOOR_L_CLOSE_3, IMG_DOOR_L_CLOSE_4, 
        IMG_DOOR_L_CLOSE_5, IMG_DOOR_L_CLOSE_6, IMG_DOOR_L_CLOSE_7, IMG_DOOR_L_CLOSE_8, 
        IMG_DOOR_L_CLOSE_9, IMG_DOOR_L_CLOSE_10, IMG_DOOR_L_CLOSE_11, IMG_DOOR_L_CLOSE_12, 
        IMG_DOOR_L_CLOSE_13, IMG_DOOR_L_CLOSE_14, IMG_DOOR_L_CLOSE_15
    };
    const char* paths_door_R[DOOR_FRAMES] = {
        IMG_DOOR_R_CLOSE_1, IMG_DOOR_R_CLOSE_2, IMG_DOOR_R_CLOSE_3, IMG_DOOR_R_CLOSE_4, 
        IMG_DOOR_R_CLOSE_5, IMG_DOOR_R_CLOSE_6, IMG_DOOR_R_CLOSE_7, IMG_DOOR_R_CLOSE_8, 
        IMG_DOOR_R_CLOSE_9, IMG_DOOR_R_CLOSE_10, IMG_DOOR_R_CLOSE_11, IMG_DOOR_R_CLOSE_12, 
        IMG_DOOR_R_CLOSE_13, IMG_DOOR_R_CLOSE_14, IMG_DOOR_R_CLOSE_15
    };
    for (int i = 0; i < DOOR_FRAMES; i++) {
        tex_door_L_close[i] = graphics_load_texture(paths_door_L[i]);
        tex_door_R_close[i] = graphics_load_texture(paths_door_R[i]);
    }

    tex_hallucinations[0] = graphics_load_texture(IMG_FREDDY_HALLUTINATION);
    tex_hallucinations[1] = graphics_load_texture(IMG_ITS_ME_1);
    tex_hallucinations[2] = graphics_load_texture(IMG_BONNIE_HALLUTINATION);
    tex_hallucinations[3] = graphics_load_texture(IMG_ITS_ME_2);

    hallucination_timer = 0;
    current_hallucination = -1;

    for (int i = 0; i < 4; i++) {
        if (tex_hallucinations[i]) {
            SDL_SetTextureBlendMode(tex_hallucinations[i], SDL_BLENDMODE_BLEND);
        }
    }

    const char* paths_bonnie_js[JUMPSCARE_BONNIE_FRAMES] = {
        IMG_BONNIE_JUMPSCARE_1, IMG_BONNIE_JUMPSCARE_2, IMG_BONNIE_JUMPSCARE_3,
        IMG_BONNIE_JUMPSCARE_4, IMG_BONNIE_JUMPSCARE_5, IMG_BONNIE_JUMPSCARE_6,
        IMG_BONNIE_JUMPSCARE_7, IMG_BONNIE_JUMPSCARE_8, IMG_BONNIE_JUMPSCARE_9,
        IMG_BONNIE_JUMPSCARE_10, IMG_BONNIE_JUMPSCARE_11
    };
    for (int i = 0; i < JUMPSCARE_BONNIE_FRAMES; i++) {
        tex_bonnie_jumpscare[i] = graphics_load_texture(paths_bonnie_js[i]);
    }

    const char* paths_chica_js[JUMPSCARE_CHICA_FRAMES] = {
        IMG_CHICA_JUMPSCARE_1, IMG_CHICA_JUMPSCARE_2, IMG_CHICA_JUMPSCARE_3,
        IMG_CHICA_JUMPSCARE_4, IMG_CHICA_JUMPSCARE_5, IMG_CHICA_JUMPSCARE_6,
        IMG_CHICA_JUMPSCARE_7, IMG_CHICA_JUMPSCARE_8, IMG_CHICA_JUMPSCARE_9,
        IMG_CHICA_JUMPSCARE_10, IMG_CHICA_JUMPSCARE_11, IMG_CHICA_JUMPSCARE_12,
        IMG_CHICA_JUMPSCARE_13, IMG_CHICA_JUMPSCARE_14, IMG_CHICA_JUMPSCARE_15,
        IMG_CHICA_JUMPSCARE_16
    };
    for (int i = 0; i < JUMPSCARE_CHICA_FRAMES; i++) {
        tex_chica_jumpscare[i] = graphics_load_texture(paths_chica_js[i]);
    }

    const char* paths_freddy_js[JUMPSCARE_FREDDY_FRAMES] = {
        IMG_FREDDY_JUMPSCARE_1, IMG_FREDDY_JUMPSCARE_2, IMG_FREDDY_JUMPSCARE_3, IMG_FREDDY_JUMPSCARE_4,
        IMG_FREDDY_JUMPSCARE_5, IMG_FREDDY_JUMPSCARE_6, IMG_FREDDY_JUMPSCARE_7, IMG_FREDDY_JUMPSCARE_8,
        IMG_FREDDY_JUMPSCARE_9, IMG_FREDDY_JUMPSCARE_10, IMG_FREDDY_JUMPSCARE_11, IMG_FREDDY_JUMPSCARE_12,
        IMG_FREDDY_JUMPSCARE_13, IMG_FREDDY_JUMPSCARE_14, IMG_FREDDY_JUMPSCARE_15, IMG_FREDDY_JUMPSCARE_16,
        IMG_FREDDY_JUMPSCARE_17, IMG_FREDDY_JUMPSCARE_18, IMG_FREDDY_JUMPSCARE_19, IMG_FREDDY_JUMPSCARE_20,
        IMG_FREDDY_JUMPSCARE_21, IMG_FREDDY_JUMPSCARE_22, IMG_FREDDY_JUMPSCARE_23, IMG_FREDDY_JUMPSCARE_24,
        IMG_FREDDY_JUMPSCARE_25, IMG_FREDDY_JUMPSCARE_26, IMG_FREDDY_JUMPSCARE_27, IMG_FREDDY_JUMPSCARE_28,
        IMG_FREDDY_JUMPSCARE_29, IMG_FREDDY_JUMPSCARE_30, IMG_FREDDY_JUMPSCARE_31
    };
    for (int i = 0; i < JUMPSCARE_FREDDY_FRAMES; i++) {
        tex_freddy_jumpscare[i] = graphics_load_texture(paths_freddy_js[i]);
    }


    sfx_fan = audio_load_sfx("romfs:/sfx/Buzz_Fan_Florescent2.wav");
    sfx_light = audio_load_sfx("romfs:/sfx/BallastHumMedium2.wav");
    sfx_door = audio_load_sfx("romfs:/sfx/SFXBible_12478.wav");

    sfx_circus = audio_load_sfx("romfs:/sfx/circus.wav");
    sfx_pounding = audio_load_sfx("romfs:/sfx/DOOR_POUNDING_ME_D0291401.wav");
    sfx_window_scare = audio_load_sfx("romfs:/sfx/windowscare.wav"); 
    sfx_error = audio_load_sfx("romfs:/sfx/error.wav");
    sfx_steps = audio_load_sfx("romfs:/sfx/deep_steps.wav");

    sfx_breath[0] = audio_load_sfx("romfs:/sfx/Vocals_Breaths_S_35972006.wav");
    sfx_breath[1] = audio_load_sfx("romfs:/sfx/Vocals_Breaths_S_35972008.wav");
    sfx_breath[2] = audio_load_sfx("romfs:/sfx/Vocals_Breaths_S_35972012.wav");
    sfx_breath[3] = audio_load_sfx("romfs:/sfx/Vocals_Breaths_S_35972014.wav");

    sfx_hallucination[0] = audio_load_sfx("romfs:/sfx/COMPUTER_DIGITAL_L2076505.wav");
    sfx_hallucination[1] = audio_load_sfx("romfs:/sfx/garble1.wav");
    sfx_hallucination[2] = audio_load_sfx("romfs:/sfx/garble2.wav");
    sfx_hallucination[3] = audio_load_sfx("romfs:/sfx/garble3.wav");
    
    sfx_kitchen[0] = audio_load_sfx("romfs:/sfx/OVEN-DRA_1_GEN-HDF18119.wav");
    sfx_kitchen[1] = audio_load_sfx("romfs:/sfx/OVEN-DRA_2_GEN-HDF18120.wav");
    sfx_kitchen[2] = audio_load_sfx("romfs:/sfx/OVEN-DRA_7_GEN-HDF18121.wav");
    sfx_kitchen[3] = audio_load_sfx("romfs:/sfx/OVEN-DRAWE_GEN-HDF18122.wav");

    // --- AÑADIDO: Cargar Audios de Freddy ---
    sfx_freddy_laugh[0] = audio_load_sfx("romfs:/sfx/Laugh_Giggle_Girl_1d.wav");
    sfx_freddy_laugh[1] = audio_load_sfx("romfs:/sfx/Laugh_Giggle_Girl_2d.wav");
    sfx_freddy_laugh[2] = audio_load_sfx("romfs:/sfx/Laugh_Giggle_Girl_8d.wav");
    sfx_running_fast = audio_load_sfx("romfs:/sfx/running fast3.wav");

    sfx_jumpscare = audio_load_sfx("romfs:/sfx/XSCREAM.wav");

    camera_x = 160.0f;
    door_L_frame = 0.0f;
    door_R_frame = 0.0f;
    left_door_on = false;
    left_light_on = false;
    right_door_on = false;
    right_light_on = false;
    fan_timer = 0;
    fan_frame = 0;
    is_winning = false;
    win_fade = 0.0f;
    random_sound_timer = 0;
    bonnie_scare_played = false; 
    chica_scare_played = false; 
    breath_timer = 0; 
    channel_breath = -1;
    channel_circus = -1; 
    kitchen_timer = 0; 
    channel_kitchen = -1; 

    is_bonnie_jumpscare = false;
    is_chica_jumpscare = false; 
    is_freddy_jumpscare = false; // --- AÑADIDO
    bonnie_jumpscare_frame = 0.0f;
    chica_jumpscare_frame = 0.0f; 
    freddy_jumpscare_frame = 0.0f; // --- AÑADIDO
    bonnie_force_down_timer = 0;
    chica_force_down_timer = 0; 
    jumpscare_duration_timer = 0;

    audio_play_music("romfs:/sfx/ColdPresc_B.wav");
    audio_set_music_volume(50); 
    if (sfx_fan) {
        audio_set_sfx_volume(sfx_fan, 25); 
        channel_fan = audio_play_sfx_loop_chunk(sfx_fan); 
    }
}

void game_update(void) {
    if (is_bonnie_jumpscare || is_chica_jumpscare || is_freddy_jumpscare) {
        if (is_bonnie_jumpscare) {
            bonnie_jumpscare_frame += JUMPSCARE_ANIM_SPEED;
            if (bonnie_jumpscare_frame >= JUMPSCARE_BONNIE_FRAMES) {
                bonnie_jumpscare_frame = 0.0f; // Loop
            }
        }
        else if (is_chica_jumpscare) {
            chica_jumpscare_frame += JUMPSCARE_ANIM_SPEED;
            if (chica_jumpscare_frame >= JUMPSCARE_CHICA_FRAMES) {
                chica_jumpscare_frame = 0.0f; 
            }
        }
        else if (is_freddy_jumpscare) { 
            if (freddy_jumpscare_frame < JUMPSCARE_FREDDY_FRAMES - 1) {
                freddy_jumpscare_frame += 0.5f;
            }
        }

        jumpscare_duration_timer++;
        if (jumpscare_duration_timer >= 85) {
            state_manager_change(STATE_TITLE); 
        }
        return; 
    }

    hud_update();
    camera_system_update();

    int items_on = 0;
    if (left_door_on) items_on++;
    if (right_door_on) items_on++;
    if (left_light_on) items_on++;
    if (right_light_on) items_on++;
    if (camera_system_is_open()) items_on++;

    bool just_blacked_out = power_system_update(items_on);

    if (just_blacked_out) {
        if (channel_light_L != -1) audio_stop_channel(channel_light_L);
        if (channel_light_R != -1) audio_stop_channel(channel_light_R);
        if (channel_fan != -1) audio_stop_channel(channel_fan);
        if (channel_kitchen != -1) audio_stop_channel(channel_kitchen); 

        channel_light_L = -1;
        channel_light_R = -1;
        channel_fan = -1;
        channel_kitchen = -1;

        left_light_on = false;
        right_light_on = false;
        if (left_door_on) { left_door_on = false; audio_play_sfx_chunk(sfx_door); }
        if (right_door_on) { right_door_on = false; audio_play_sfx_chunk(sfx_door); }
        camera_system_force_close();
    }

    if (current_hour >= 6 && !is_winning) {
        is_winning = true;
        audio_stop_all_sfx();
        audio_stop_music();
    }

    if (is_winning) {
        win_fade += 8.0f; 
        if (win_fade >= 255.0f) {
            state_manager_change(STATE_6AM); 
        }
        return; 
    }

    random_sound_timer++;

    if (random_sound_timer % 300 == 0) {
        if ((rand() % 30) == 0) {
            if (channel_circus == -1 || !Mix_Playing(channel_circus)) {
                channel_circus = audio_play_sfx_chunk(sfx_circus);
            }
        }
    }

    if (random_sound_timer % 600 == 0) {
        if ((rand() % 50) == 0) {
            int random_vol = 10 + (rand() % 40); 
            audio_set_sfx_volume(sfx_pounding, random_vol);
            audio_play_sfx_chunk(sfx_pounding);
        }
    }
    
    if (!is_power_out) {
        animatronics_update(left_door_on, right_door_on, camera_system_is_open());
        
        // --- SONIDO DE PASOS AL MOVERSE ---
        if (animatronics_get_bonnie_moved_timer() == 10) {
            if (sfx_steps) {
                audio_set_sfx_volume(sfx_steps, 50); 
                audio_play_sfx_chunk(sfx_steps);
            }
        }
        
        if (animatronics_get_chica_moved_timer() == 10) {
            // Silenciamos los pasos SOLO si se mueve DENTRO de la cocina
            if (animatronics_get_chica_room() != CAM_6 && animatronics_get_chica_prev_room() != CAM_6) {
                if (sfx_steps) {
                    audio_set_sfx_volume(sfx_steps, 50); 
                    audio_play_sfx_chunk(sfx_steps);
                }
            }
        }

        // --- AÑADIDO: SONIDO DE RISA Y CORRER DE FREDDY ---
        if (animatronics_get_freddy_moved_timer() == 10) {
            int r_laugh = rand() % 3;
            if (sfx_freddy_laugh[r_laugh]) {
                audio_set_sfx_volume(sfx_freddy_laugh[r_laugh], 100);
                audio_play_sfx_chunk(sfx_freddy_laugh[r_laugh]);
            }
            if (sfx_running_fast) {
                audio_set_sfx_volume(sfx_running_fast, 70); 
                audio_play_sfx_chunk(sfx_running_fast);
            }
        }
        
        if (animatronics_get_bonnie_room() != ROOM_DOOR_LEFT) bonnie_scare_played = false;
        if (animatronics_get_chica_room() != ROOM_DOOR_RIGHT) chica_scare_played = false;

        if (animatronics_get_bonnie_room() == ROOM_OFFICE && left_light_on) {
            left_light_on = false;
            audio_stop_channel(channel_light_L);
            channel_light_L = -1;
        }
        if (animatronics_get_chica_room() == ROOM_OFFICE && right_light_on) {
            right_light_on = false;
            audio_stop_channel(channel_light_R);
            channel_light_R = -1;
        }

        if (random_sound_timer % 60 == 0) { 
            if ((rand() % 1000) == 0) {     
                hallucination_timer = 100;  
                int random_snd = rand() % 4;
                if (sfx_hallucination[random_snd]) {
                    audio_set_sfx_volume(sfx_hallucination[random_snd], 100); 
                    audio_play_sfx_chunk(sfx_hallucination[random_snd]);
                }
            }
        }

        if (hallucination_timer > 0) {
            hallucination_timer--;

            // Si el temporizador de retención ha llegado a 0, cambiamos de imagen
            if (hallucination_hold_timer <= 0) {
                hallucination_hold_timer = 2; 

                // 30% de probabilidad de mostrar una cara/texto, 70% de no mostrar nada (parpadeo negro)
                if ((rand() % 10) < 3) { 
                    current_hallucination = rand() % 4; 
                } else {
                    current_hallucination = -1; 
                }
            } else {
                hallucination_hold_timer--; 
            }
        } else {
            current_hallucination = -1; 
            hallucination_hold_timer = 0;
        }

        if (animatronics_get_chica_room() == CAM_6) {
            kitchen_timer++;
            if (kitchen_timer >= 240) {
                kitchen_timer = 0;
                if (rand() % 2 == 0) {
                    int r_kitchen = rand() % 4;
                    if (sfx_kitchen[r_kitchen]) {
                        audio_set_sfx_volume(sfx_kitchen[r_kitchen], 128); 
                        channel_kitchen = audio_play_sfx_chunk(sfx_kitchen[r_kitchen]);
                    }
                }
            }
        } else {
            kitchen_timer = 0;
        }

        if (channel_kitchen != -1) {
            if (Mix_Playing(channel_kitchen)) {
                int target_vol = 30; 
                if (camera_system_is_open()) {
                    if (camera_system_get_current_cam() == CAM_6) target_vol = 100; 
                    else target_vol = 50; 
                }
                audio_set_channel_volume(channel_kitchen, target_vol);
            } else {
                channel_kitchen = -1; 
            }
        }

        // --- SISTEMA DE RESPIRACIÓN Y MUERTE ---
        bool anyone_in_office = false;
        
        if (animatronics_get_bonnie_room() == ROOM_OFFICE || animatronics_get_chica_room() == ROOM_OFFICE) {
            anyone_in_office = true;

            if (camera_system_is_open()) {
                breath_timer++;
                if (breath_timer >= 300) {
                    breath_timer = 0;
                    if (channel_breath == -1 || !Mix_Playing(channel_breath)) {
                        if (rand() % 3 == 0) {
                            int r_breath = rand() % 4; 
                            if (sfx_breath[r_breath]) {
                                audio_set_sfx_volume(sfx_breath[r_breath], 100); 
                                channel_breath = audio_play_sfx_chunk(sfx_breath[r_breath]);
                            }
                        }
                    }
                }
                
                if (animatronics_get_bonnie_room() == ROOM_OFFICE) bonnie_force_down_timer++;
                if (animatronics_get_chica_room() == ROOM_OFFICE) chica_force_down_timer++;
            }

            bool trigger_bonnie_death = false;
            bool trigger_chica_death = false;

            if (bonnie_force_down_timer >= 1800) trigger_bonnie_death = true;
            if (chica_force_down_timer >= 1800) trigger_chica_death = true;

            if (input_get_button_down(HidNpadButton_A) && camera_system_is_open()) {
                if (animatronics_get_bonnie_room() == ROOM_OFFICE) trigger_bonnie_death = true;
                if (animatronics_get_chica_room() == ROOM_OFFICE) trigger_chica_death = true;
            }

            if (trigger_bonnie_death && !is_bonnie_jumpscare && !is_chica_jumpscare && !is_freddy_jumpscare) {
                is_bonnie_jumpscare = true;
                camera_system_force_close();
                audio_stop_all_sfx();
                audio_stop_music();
                if (sfx_jumpscare) {
                    audio_set_sfx_volume(sfx_jumpscare, 100);
                    audio_play_sfx_chunk(sfx_jumpscare);
                }
            } 
            else if (trigger_chica_death && !is_chica_jumpscare && !is_bonnie_jumpscare && !is_freddy_jumpscare) {
                is_chica_jumpscare = true;
                camera_system_force_close();
                audio_stop_all_sfx();
                audio_stop_music();
                if (sfx_jumpscare) {
                    audio_set_sfx_volume(sfx_jumpscare, 100);
                    audio_play_sfx_chunk(sfx_jumpscare);
                }
            }
        } 
        
        if (!anyone_in_office) {
            breath_timer = 0;
            bonnie_force_down_timer = 0; 
            chica_force_down_timer = 0;
        }
        
    } else {
        hallucination_timer = 0; 
        current_hallucination = -1;
        breath_timer = 0;
        bonnie_force_down_timer = 0;
        chica_force_down_timer = 0;
        kitchen_timer = 0;
    }
    
    if (!camera_system_is_open() && camera_system_get_frame() <= 0.0f) {
        s16 stick_x = input_get_stick_x(0);
        if (stick_x > 7000 || stick_x < -7000) {
            float speed = (stick_x / 32767.0f) * 5.0f;
            camera_x += speed;
        }
        if (camera_x < 0)   camera_x = 0;
        if (camera_x > 320) camera_x = 320;
    }

    if (input_get_button_down(HidNpadButton_Plus)) state_manager_change(STATE_TITLE);

    if (!is_power_out) { 
        if (!camera_system_is_open()) {
            if (input_get_button_down(HidNpadButton_L)) {
                if (animatronics_get_bonnie_room() == ROOM_OFFICE) {
                    audio_play_sfx_chunk(sfx_error); 
                } else if (door_L_frame <= 0.0f || door_L_frame >= DOOR_FRAMES -1) { 
                    left_door_on = !left_door_on; 
                    audio_play_sfx_chunk(sfx_door); 
                }
            }
            if (input_get_button_down(HidNpadButton_ZL)) {
                if (animatronics_get_bonnie_room() == ROOM_OFFICE) {
                    audio_play_sfx_chunk(sfx_error); 
                } else {
                    left_light_on = !left_light_on;
                    if (left_light_on) {
                        right_light_on = false;  
                        audio_stop_channel(channel_light_R); 
                        channel_light_L = audio_play_sfx_loop_chunk(sfx_light); 
                    } else {
                        audio_stop_channel(channel_light_L); 
                        channel_light_L = -1;
                    }
                }
            }
            if (input_get_button_down(HidNpadButton_R)) {
                if (animatronics_get_chica_room() == ROOM_OFFICE) { 
                    audio_play_sfx_chunk(sfx_error); 
                } else if (door_R_frame <= 0.0f || door_R_frame >= DOOR_FRAMES -1) { 
                    right_door_on = !right_door_on;
                    audio_play_sfx_chunk(sfx_door); 
                }
            }
            if (input_get_button_down(HidNpadButton_ZR)) {
                if (animatronics_get_chica_room() == ROOM_OFFICE) { 
                    audio_play_sfx_chunk(sfx_error); 
                } else {
                    right_light_on = !right_light_on;
                    if (right_light_on) {
                        left_light_on = false;
                        audio_stop_channel(channel_light_L); 
                        channel_light_R = audio_play_sfx_loop_chunk(sfx_light); 
                    } else {
                        audio_stop_channel(channel_light_R);
                        channel_light_R = -1;
                    }
                }
            }
        } 
        
        if (input_get_button_down(HidNpadButton_A)) {
            camera_system_toggle();
            if (camera_system_is_open()) {
                left_light_on = false;
                right_light_on = false;
                audio_stop_channel(channel_light_L);
                audio_stop_channel(channel_light_R);
                channel_light_L = -1;
                channel_light_R = -1;
                audio_set_sfx_volume(sfx_fan, 10); 
            } else {
                audio_set_sfx_volume(sfx_fan, 25); 

                if (animatronics_get_bonnie_room() == ROOM_OFFICE && !is_chica_jumpscare && !is_freddy_jumpscare) {
                    is_bonnie_jumpscare = true;
                    audio_stop_all_sfx();
                    audio_stop_music();
                    if (sfx_jumpscare) {
                        audio_set_sfx_volume(sfx_jumpscare, 100);
                        audio_play_sfx_chunk(sfx_jumpscare);
                    }
                }
                else if (animatronics_get_chica_room() == ROOM_OFFICE && !is_bonnie_jumpscare && !is_freddy_jumpscare) {
                    is_chica_jumpscare = true;
                    audio_stop_all_sfx();
                    audio_stop_music();
                    if (sfx_jumpscare) {
                        audio_set_sfx_volume(sfx_jumpscare, 100);
                        audio_play_sfx_chunk(sfx_jumpscare);
                    }
                }
                // --- AÑADIDO: Jumpscare Freddy ---
                else if (animatronics_get_freddy_room() == ROOM_OFFICE && !is_bonnie_jumpscare && !is_chica_jumpscare) {
                    is_freddy_jumpscare = true;
                    audio_stop_all_sfx();
                    audio_stop_music();
                    if (sfx_jumpscare) {
                        audio_set_sfx_volume(sfx_jumpscare, 100);
                        audio_play_sfx_chunk(sfx_jumpscare);
                    }
                }
            }
        }
    }

    fan_timer++;
    if (!is_power_out && fan_timer >= FAN_ANIM_SPEED) {
        fan_timer = 0;
        fan_frame++;
        if (fan_frame > 2) fan_frame = 0;
    }

    if (left_door_on) { if (door_L_frame < DOOR_FRAMES -1) door_L_frame += DOOR_ANIM_SPEED; } 
    else { if (door_L_frame > 0.0f) door_L_frame -= DOOR_ANIM_SPEED; }

    if (right_door_on) { if (door_R_frame < DOOR_FRAMES -1) door_R_frame += DOOR_ANIM_SPEED; } 
    else { if (door_R_frame > 0.0f) door_R_frame -= DOOR_ANIM_SPEED; }
}

void game_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    SDL_Texture* current_background = tex_office_normal;
    
    if (is_power_out) {
        if (powerout_state == 0) current_background = tex_office_blackout;
        else if (powerout_state == 1) current_background = show_freddy ? tex_office_blackout_freddy : tex_office_blackout;
        else if (powerout_state == 2) current_background = (rand() % 2 == 0) ? tex_office_blackout : NULL;
        else current_background = NULL;
    } else {
        int flicker_chance = rand() % 10;
        
        if (left_light_on && flicker_chance > 1) {
            if (animatronics_get_bonnie_room() == ROOM_DOOR_LEFT) {
                current_background = tex_office_bonnie;
                if (!bonnie_scare_played) {
                    audio_play_sfx_chunk(sfx_window_scare);
                    bonnie_scare_played = true;
                }
            } else {
                current_background = tex_office_light_L;
            }
        } 
        else if (right_light_on && flicker_chance > 1) {
            if (animatronics_get_chica_room() == ROOM_DOOR_RIGHT) {
                current_background = tex_office_chica;
                if (!chica_scare_played) {
                    audio_play_sfx_chunk(sfx_window_scare);
                    chica_scare_played = true;
                }
            } else {
                current_background = tex_office_light_R;
            }
        }
    }

    if (current_background) {
        SDL_Rect src_rect = {(int)camera_x, 0, 1280, 720};
        SDL_RenderCopy(renderer, current_background, &src_rect, NULL);
    }

    int current_L = (int)door_L_frame;
    if (current_L >= 0 && tex_door_L_close[current_L]) {
        SDL_Rect dst_L = {72 - (int)camera_x, -1, 223, 720};
        SDL_RenderCopy(renderer, tex_door_L_close[current_L], NULL, &dst_L);
    }
    int current_R = (int)door_R_frame;
    if (current_R >= 0 && tex_door_R_close[current_R]) {
        SDL_Rect dst_R = {1270 - (int)camera_x, -2, 248, 720};
        SDL_RenderCopy(renderer, tex_door_R_close[current_R], NULL, &dst_R);
    }

    // 1. Dibujamos el interior de la cámara y la Interfaz (UI)
    camera_system_draw_room();
    camera_system_draw_ui();

    // 2. Dibujamos la Oficina (ventilador, botones)
    if (!is_power_out) {
        bool is_cam_fully_open = (camera_system_is_open() && camera_system_get_frame() >= (CAM_FRAMES - 1));
        if (!is_cam_fully_open) {
            int state_L = 0;
            if (left_door_on && !left_light_on) state_L = 1;      
            else if (!left_door_on && left_light_on) state_L = 2; 
            else if (left_door_on && left_light_on) state_L = 3;  
            if (tex_button_L[state_L]) {
                SDL_Rect dst_L = {6 - (int)camera_x, 263, 92, 247};
                SDL_RenderCopy(renderer, tex_button_L[state_L], NULL, &dst_L);
            }

            int state_R = 0;
            if (right_door_on && !right_light_on) state_R = 1;      
            else if (!right_door_on && right_light_on) state_R = 2; 
            else if (right_door_on && right_light_on) state_R = 3;  
            if (tex_button_R[state_R]) {
                SDL_Rect dst_R = {1497 - (int)camera_x, 273, 92, 247};
                SDL_RenderCopy(renderer, tex_button_R[state_R], NULL, &dst_R);
            }

            if (tex_fan[fan_frame]) {
                int w, h;
                SDL_QueryTexture(tex_fan[fan_frame], NULL, NULL, &w, &h);
                SDL_Rect dst_rect = {780 - (int)camera_x, 303, w, h};
                SDL_RenderCopy(renderer, tex_fan[fan_frame], NULL, &dst_rect);
            }
        }
        hud_draw(camera_system_is_open(), camera_system_get_frame());
        power_system_draw_hud(camera_system_is_open(), camera_system_get_frame());
        camera_system_draw_button();
    } 

    power_system_draw_jumpscare();

    // 3. ¡LA CLAVE ESTÁ AQUÍ! La animación de la tablet se dibuja AHORA, tapando la oficina y el ventilador.
    camera_system_draw_animation();

    // 4. Jumpscares y Alucinaciones (tapan absolutamente todo)
    if (hallucination_timer > 0 && !is_power_out && current_hallucination >= 0) {
        if (tex_hallucinations[current_hallucination]) {
            SDL_Rect dst_rect = {0, 0, 1280, 720};
            SDL_RenderCopy(renderer, tex_hallucinations[current_hallucination], NULL, &dst_rect);
        }
    }

    if (is_bonnie_jumpscare) {
        int frame = (int)bonnie_jumpscare_frame;
        if (frame >= JUMPSCARE_BONNIE_FRAMES) frame = JUMPSCARE_BONNIE_FRAMES - 1; 

        if (tex_bonnie_jumpscare[frame]) {
            SDL_Rect src_jumpscare = {160, 0, 1280, 720}; 
            SDL_Rect dst_jumpscare = {0, 0, 1280, 720}; 
            SDL_RenderCopy(renderer, tex_bonnie_jumpscare[frame], &src_jumpscare, &dst_jumpscare);
        }
    }
    else if (is_chica_jumpscare) {
        int frame = (int)chica_jumpscare_frame;
        if (frame >= JUMPSCARE_CHICA_FRAMES) frame = JUMPSCARE_CHICA_FRAMES - 1;

        if (tex_chica_jumpscare[frame]) {
            SDL_Rect src_jumpscare = {160, 0, 1280, 720}; 
            SDL_Rect dst_jumpscare = {0, 0, 1280, 720}; 
            SDL_RenderCopy(renderer, tex_chica_jumpscare[frame], &src_jumpscare, &dst_jumpscare);
        }
    }
    // --- AÑADIDO: Dibujo de Jumpscare Freddy ---
    else if (is_freddy_jumpscare) {
        int frame = (int)freddy_jumpscare_frame;
        if (frame >= JUMPSCARE_FREDDY_FRAMES) frame = JUMPSCARE_FREDDY_FRAMES - 1;

        if (tex_freddy_jumpscare[frame]) {
            SDL_Rect src_jumpscare = {160, 0, 1280, 720}; 
            SDL_Rect dst_jumpscare = {0, 0, 1280, 720}; 
            SDL_RenderCopy(renderer, tex_freddy_jumpscare[frame], &src_jumpscare, &dst_jumpscare);
        }
    }

    if (is_winning) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)win_fade);
        SDL_Rect fullscreen = {0, 0, 1280, 720};
        SDL_RenderFillRect(renderer, &fullscreen);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

void game_cleanup(void) {
    hud_cleanup();
    power_system_cleanup();
    camera_system_cleanup();
    animatronics_cleanup();

    if (tex_office_normal) { SDL_DestroyTexture(tex_office_normal); tex_office_normal = NULL; }
    if (tex_office_light_L) { SDL_DestroyTexture(tex_office_light_L); tex_office_light_L = NULL; }
    if (tex_office_light_R) { SDL_DestroyTexture(tex_office_light_R); tex_office_light_R = NULL; }
    if (tex_office_blackout) { SDL_DestroyTexture(tex_office_blackout); tex_office_blackout = NULL; }
    if (tex_office_blackout_freddy) { SDL_DestroyTexture(tex_office_blackout_freddy); tex_office_blackout_freddy = NULL; }
    if (tex_office_bonnie) { SDL_DestroyTexture(tex_office_bonnie); tex_office_bonnie = NULL; } 
    if (tex_office_chica) { SDL_DestroyTexture(tex_office_chica); tex_office_chica = NULL; } 

    for (int i = 0; i < DOOR_FRAMES; i++) {
        if (tex_door_L_close[i]) { SDL_DestroyTexture(tex_door_L_close[i]); tex_door_L_close[i] = NULL; }
        if (tex_door_R_close[i]) { SDL_DestroyTexture(tex_door_R_close[i]); tex_door_R_close[i] = NULL; }
    }
    for (int i = 0; i < 4; i++) {
        if (tex_button_L[i]) { SDL_DestroyTexture(tex_button_L[i]); tex_button_L[i] = NULL; }
        if (tex_button_R[i]) { SDL_DestroyTexture(tex_button_R[i]); tex_button_R[i] = NULL; }
    }
    for (int i = 0; i < 3; i++) {
        if (tex_fan[i]) { SDL_DestroyTexture(tex_fan[i]); tex_fan[i] = NULL; }
    }
    for (int i = 0; i < 4; i++) {
        if (tex_hallucinations[i]) {
            SDL_DestroyTexture(tex_hallucinations[i]);
            tex_hallucinations[i] = NULL;
        }
    }

    for (int i = 0; i < JUMPSCARE_BONNIE_FRAMES; i++) {
        if (tex_bonnie_jumpscare[i]) { 
            SDL_DestroyTexture(tex_bonnie_jumpscare[i]); 
            tex_bonnie_jumpscare[i] = NULL; 
        }
    }
    
    for (int i = 0; i < JUMPSCARE_CHICA_FRAMES; i++) {
        if (tex_chica_jumpscare[i]) { 
            SDL_DestroyTexture(tex_chica_jumpscare[i]); 
            tex_chica_jumpscare[i] = NULL; 
        }
    }

    // --- AÑADIDO: Limpieza de texturas de Freddy ---
    for (int i = 0; i < JUMPSCARE_FREDDY_FRAMES; i++) {
        if (tex_freddy_jumpscare[i]) { 
            SDL_DestroyTexture(tex_freddy_jumpscare[i]); 
            tex_freddy_jumpscare[i] = NULL; 
        }
    }

    audio_stop_music();
    audio_stop_all_sfx();
    
    if (sfx_fan) { audio_free_sfx(sfx_fan); sfx_fan = NULL; }
    if (sfx_light) { audio_free_sfx(sfx_light); sfx_light = NULL; }
    if (sfx_door) { audio_free_sfx(sfx_door); sfx_door = NULL; }
    if (sfx_circus) audio_free_sfx(sfx_circus);
    if (sfx_pounding) audio_free_sfx(sfx_pounding);
    if (sfx_window_scare) { audio_free_sfx(sfx_window_scare); sfx_window_scare = NULL; } 
    if (sfx_error) { audio_free_sfx(sfx_error); sfx_error = NULL; } 
    if (sfx_steps) { audio_free_sfx(sfx_steps); sfx_steps = NULL; }

    for (int i = 0; i < 4; i++) {
        if (sfx_breath[i]) { 
            audio_free_sfx(sfx_breath[i]); 
            sfx_breath[i] = NULL; 
        }
    }
    for (int i = 0; i < 4; i++) {
        if (sfx_hallucination[i]) { 
            audio_free_sfx(sfx_hallucination[i]); 
            sfx_hallucination[i] = NULL; 
        }
    }
    
    for (int i = 0; i < 4; i++) {
        if (sfx_kitchen[i]) { 
            audio_free_sfx(sfx_kitchen[i]); 
            sfx_kitchen[i] = NULL; 
        }
    }

    // --- AÑADIDO: Limpieza de audios de Freddy ---
    for (int i = 0; i < 3; i++) {
        if (sfx_freddy_laugh[i]) {
            audio_free_sfx(sfx_freddy_laugh[i]);
            sfx_freddy_laugh[i] = NULL;
        }
    }
    if (sfx_running_fast) { audio_free_sfx(sfx_running_fast); sfx_running_fast = NULL; }


    if (sfx_jumpscare) { audio_free_sfx(sfx_jumpscare); sfx_jumpscare = NULL; }
}