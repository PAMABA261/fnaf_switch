#include "game/assets.h"
#include "game/state_manager.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/audio.h"
#include <stdlib.h>
#include <SDL2/SDL.h>

// ==========================================
// VARIABLES GLOBALES
// ==========================================

// --- Entorno y Cámara ---
static float camera_x = 160.0f;
static SDL_Texture* tex_office_normal = NULL;
static SDL_Texture* tex_office_light_L = NULL;
static SDL_Texture* tex_office_light_R = NULL;
static SDL_Texture* tex_fan[3] = {NULL};
static int fan_frame = 0;
static int fan_timer = 0;
#define FAN_ANIM_SPEED 1
static bool is_power_out = false;
static SDL_Texture* tex_office_blackout = NULL;
static SDL_Texture* tex_office_blackout_freddy = NULL;
static int powerout_state = 0; 
static Uint64 powerout_step_time = 0;
static Uint64 powerout_total_time = 0;  
static Uint64 powerout_flicker_time = 0;
static bool show_freddy = false;

// --- Puertas y Botones ---
#define DOOR_FRAMES 15
static const float DOOR_ANIM_SPEED = 0.6f;
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

// --- Tableta (Cámaras) ---
#define CAM_FRAMES 11
static const float CAM_ANIM_SPEED = 0.6f;
static float cam_frame = 0.0f;
static bool cam_open = false;
static SDL_Texture* tex_cam[CAM_FRAMES] = {NULL};
static SDL_Texture* tex_button_cam = NULL; 

// --- Interfaz (HUD) y Tiempo ---
int current_night = 5;
static int current_hour = 0;
static Uint64 night_start_time = 0;
static SDL_Texture* tex_night_text = NULL;
static SDL_Texture* tex_night_num[5] = {NULL};
static SDL_Texture* tex_hour[5] = {NULL};
static SDL_Texture* tex_am = NULL;

// --- Sistema de Energía ---
static int power_left = 999; 
static int current_usage = 1;
static Uint64 last_drain_time = 0; 
static Uint64 last_passive_drain_time = 0;
static SDL_Texture* tex_usage_text = NULL;
static SDL_Texture* tex_battery[5] = {NULL};
static SDL_Texture* tex_power_text = NULL;
static SDL_Texture* tex_power_percent = NULL;
static SDL_Texture* tex_batt_num[10] = {NULL}; 

#define JUMPSCARE_FRAMES 21
static SDL_Texture* tex_jumpscare_freddy[JUMPSCARE_FRAMES] = {NULL};
static float jumpscare_frame = 0.0f;

// --- Audio ---
static Mix_Chunk* sfx_fan = NULL;
static Mix_Chunk* sfx_light = NULL;
static Mix_Chunk* sfx_door = NULL;
static Mix_Chunk* sfx_cam_up = NULL;
static Mix_Chunk* sfx_cam_down = NULL;
static Mix_Chunk* sfx_powerdown = NULL;
static Mix_Chunk* sfx_musicbox = NULL;
static Mix_Chunk* sfx_jumpscare = NULL;
static Mix_Chunk* sfx_ambience2 = NULL;
static int channel_light_L = -1;
static int channel_light_R = -1;


void game_init(void) {
    // --- RESETEO DE VARIABLES DE ESTADO ---
    camera_x = 160.0f;
    door_L_frame = 0.0f;
    door_R_frame = 0.0f;
    left_door_on = false;
    left_light_on = false;
    right_door_on = false;
    right_light_on = false;
    cam_open = false;
    cam_frame = 0.0f;
    fan_timer = 0;
    fan_frame = 0;
    night_start_time = SDL_GetTicks64();
    last_drain_time = SDL_GetTicks64();
    last_passive_drain_time = SDL_GetTicks64();
    power_left = 999;
    current_hour = 0;
    is_power_out = false;
    powerout_state = 0;
    powerout_step_time = 0;
    powerout_total_time = 0;
    powerout_flicker_time = 0;
    show_freddy = false;
    jumpscare_frame = 0.0f;

    // Entorno
    tex_office_normal = graphics_load_texture(IMG_OFFICE);
    tex_office_light_L = graphics_load_texture(IMG_OFFICE_LIGHT_L);
    tex_office_light_R = graphics_load_texture(IMG_OFFICE_LIGHT_R);
    tex_fan[0] = graphics_load_texture(IMG_FAN_1);
    tex_fan[1] = graphics_load_texture(IMG_FAN_2);
    tex_fan[2] = graphics_load_texture(IMG_FAN_3);
    tex_office_blackout = graphics_load_texture(IMG_OFFICE_BLACK_OUT);
    tex_office_blackout_freddy = graphics_load_texture(IMG_OFFICE_BLACK_OUT_FREDDY);

    // Botones
    tex_button_L[0] = graphics_load_texture(IMG_BUTTON_L_1);
    tex_button_L[1] = graphics_load_texture(IMG_BUTTON_L_2);
    tex_button_L[2] = graphics_load_texture(IMG_BUTTON_L_3);
    tex_button_L[3] = graphics_load_texture(IMG_BUTTON_L_4);
    tex_button_R[0] = graphics_load_texture(IMG_BUTTON_R_1);
    tex_button_R[1] = graphics_load_texture(IMG_BUTTON_R_2);
    tex_button_R[2] = graphics_load_texture(IMG_BUTTON_R_3);
    tex_button_R[3] = graphics_load_texture(IMG_BUTTON_R_4);

    // Puertas
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

    // Tableta
    tex_button_cam = graphics_load_texture(IMG_BUTTON_CAM);
    const char* paths_cam[CAM_FRAMES] = {
        IMG_CAM_OPEN_1, IMG_CAM_OPEN_2, IMG_CAM_OPEN_3, IMG_CAM_OPEN_4, IMG_CAM_OPEN_5,
        IMG_CAM_OPEN_6, IMG_CAM_OPEN_7, IMG_CAM_OPEN_8, IMG_CAM_OPEN_9, IMG_CAM_OPEN_10, 
        IMG_CAM_OPEN_11
    };
    for (int i = 0; i < CAM_FRAMES; i++) {
        tex_cam[i] = graphics_load_texture(paths_cam[i]);
    }

    // HUD: Tiempo y Noche
    tex_hour[0] = graphics_load_texture(IMG_HOUR_1);
    tex_hour[1] = graphics_load_texture(IMG_HOUR_2);
    tex_hour[2] = graphics_load_texture(IMG_HOUR_3);
    tex_hour[3] = graphics_load_texture(IMG_HOUR_4);
    tex_hour[4] = graphics_load_texture(IMG_HOUR_5);
    tex_am = graphics_load_texture(IMG_HOUR_AM);
    
    tex_night_text = graphics_load_texture(IMG_NIGHT_GAME);
    tex_night_num[0] = graphics_load_texture(IMG_NIGHT_NUM_1);
    tex_night_num[1] = graphics_load_texture(IMG_NIGHT_NUM_2);
    tex_night_num[2] = graphics_load_texture(IMG_NIGHT_NUM_3);
    tex_night_num[3] = graphics_load_texture(IMG_NIGHT_NUM_4);
    tex_night_num[4] = graphics_load_texture(IMG_NIGHT_NUM_5);

    // HUD: Energía
    tex_usage_text = graphics_load_texture(IMG_USAGE);
    tex_battery[0] = graphics_load_texture(IMG_BATTERY_1);
    tex_battery[1] = graphics_load_texture(IMG_BATTERY_2);
    tex_battery[2] = graphics_load_texture(IMG_BATTERY_3);
    tex_battery[3] = graphics_load_texture(IMG_BATTERY_4);
    tex_battery[4] = graphics_load_texture(IMG_BATTERY_5);
    
    tex_power_text = graphics_load_texture(IMG_POWER_LEFT);
    tex_power_percent = graphics_load_texture(IMG_BATTERY_PERCENT);
    tex_batt_num[0] = graphics_load_texture(IMG_BATTERY_NUMBERS_1); 
    tex_batt_num[1] = graphics_load_texture(IMG_BATTERY_NUMBERS_2); 
    tex_batt_num[2] = graphics_load_texture(IMG_BATTERY_NUMBERS_3); 
    tex_batt_num[3] = graphics_load_texture(IMG_BATTERY_NUMBERS_4); 
    tex_batt_num[4] = graphics_load_texture(IMG_BATTERY_NUMBERS_5); 
    tex_batt_num[5] = graphics_load_texture(IMG_BATTERY_NUMBERS_6); 
    tex_batt_num[6] = graphics_load_texture(IMG_BATTERY_NUMBERS_7); 
    tex_batt_num[7] = graphics_load_texture(IMG_BATTERY_NUMBERS_8); 
    tex_batt_num[8] = graphics_load_texture(IMG_BATTERY_NUMBERS_9); 
    tex_batt_num[9] = graphics_load_texture(IMG_BATTERY_NUMBERS_10);

    const char* paths_jumpscare[JUMPSCARE_FRAMES] = {
        IMG_FREDDY_JUMPSCARE_BLACKOUT_1, IMG_FREDDY_JUMPSCARE_BLACKOUT_2,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_3, IMG_FREDDY_JUMPSCARE_BLACKOUT_4,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_5, IMG_FREDDY_JUMPSCARE_BLACKOUT_6,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_7, IMG_FREDDY_JUMPSCARE_BLACKOUT_8,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_9, IMG_FREDDY_JUMPSCARE_BLACKOUT_10,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_11, IMG_FREDDY_JUMPSCARE_BLACKOUT_12,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_13, IMG_FREDDY_JUMPSCARE_BLACKOUT_14,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_15, IMG_FREDDY_JUMPSCARE_BLACKOUT_16,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_17, IMG_FREDDY_JUMPSCARE_BLACKOUT_18,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_19, IMG_FREDDY_JUMPSCARE_BLACKOUT_20,
        IMG_FREDDY_JUMPSCARE_BLACKOUT_21
    };

    for (int i = 0; i < JUMPSCARE_FRAMES; i++) {
        tex_jumpscare_freddy[i] = graphics_load_texture(paths_jumpscare[i]);
    }

    // Audio
    audio_play_music("romfs:/sfx/ColdPresc_B.wav");
    audio_set_music_volume(50); 
    sfx_fan = audio_load_sfx("romfs:/sfx/Buzz_Fan_Florescent2.wav");
    audio_set_sfx_volume(sfx_fan, 50); 
    audio_play_sfx_loop_chunk(sfx_fan); 
    sfx_light = audio_load_sfx("romfs:/sfx/BallastHumMedium2.wav");
    sfx_door = audio_load_sfx("romfs:/sfx/SFXBible_12478.wav");
    sfx_cam_up = audio_load_sfx("romfs:/sfx/CAMERA_VIDEO_LOA_60105303.wav");
    sfx_cam_down = audio_load_sfx("romfs:/sfx/put_down.wav");
    sfx_powerdown = audio_load_sfx("romfs:/sfx/powerdown.wav");
    sfx_musicbox = audio_load_sfx("romfs:/sfx/music_box.wav");
    sfx_jumpscare = audio_load_sfx("romfs:/sfx/XSCREAM.wav");
    sfx_ambience2 = audio_load_sfx("romfs:/sfx/ambience2.wav");
}


void game_update(void) {
    Uint64 current_time = SDL_GetTicks64();
    
    // Condición de victoria
    current_hour = (current_time - night_start_time) / 86000;
    if (current_hour >= 6) {
        audio_stop_all_sfx();
        audio_stop_music();
        state_manager_change(STATE_TITLE);
        return;
    }

    // Paneo de cámara
    s16 stick_x = input_get_stick_x(0);
    if (stick_x > 7000 || stick_x < -7000) {
        float speed = (stick_x / 32767.0f) * 5.0f;
        camera_x += speed;
    }
    if (camera_x < 0)   camera_x = 0;
    if (camera_x > 320) camera_x = 320;

    // Salida rápida
    if (input_get_button_down(HidNpadButton_Plus)) {
        state_manager_change(STATE_TITLE);
    }

    if (!is_power_out) {
        // Input: Puertas y Luces
        if (input_get_button_down(HidNpadButton_L)) {
            if (door_L_frame <= 0.0f || door_L_frame >= DOOR_FRAMES -1) { 
                left_door_on = !left_door_on; 
                audio_play_sfx_chunk(sfx_door); 
            }
        }

        if (input_get_button_down(HidNpadButton_ZL)) {
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

        if (input_get_button_down(HidNpadButton_R)) {
            if (door_R_frame <= 0.0f || door_R_frame >= DOOR_FRAMES -1) { 
                right_door_on = !right_door_on;
                audio_play_sfx_chunk(sfx_door); 
            }
        }

        if (input_get_button_down(HidNpadButton_ZR)) {
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

        // Input: Tableta
        if (input_get_button_down(HidNpadButton_A)) {
            if (cam_frame <= 0.0f || cam_frame >= (CAM_FRAMES - 1)) {
                cam_open = !cam_open;
                if (cam_open) {
                    audio_play_sfx_chunk(sfx_cam_up);
                } else {
                    audio_play_sfx_chunk(sfx_cam_down);
                }
            }
        }
    }

    // Animaciones
    fan_timer++;
    if (fan_timer >= FAN_ANIM_SPEED) {
        fan_timer = 0;
        fan_frame++;
        if (fan_frame > 2) fan_frame = 0;
    }

    if (left_door_on) {
        if (door_L_frame < DOOR_FRAMES -1) door_L_frame += DOOR_ANIM_SPEED;
    } else {
        if (door_L_frame > 0.0f) door_L_frame -= DOOR_ANIM_SPEED;
    }

    if (right_door_on) {
        if (door_R_frame < DOOR_FRAMES -1) door_R_frame += DOOR_ANIM_SPEED;
    } else {
        if (door_R_frame > 0.0f) door_R_frame -= DOOR_ANIM_SPEED;
    }

    if (cam_open) {
        if (cam_frame < CAM_FRAMES - 1) cam_frame += CAM_ANIM_SPEED;
    } else {
        if (cam_frame > 0.0f) cam_frame -= CAM_ANIM_SPEED;
    }

    // Consumo de Energía
    current_usage = 1; 
    if (left_door_on) current_usage++;
    if (right_door_on) current_usage++;
    if (left_light_on) current_usage++;
    if (right_light_on) current_usage++;
    if (cam_open) current_usage++; 
    if (current_usage > 5) current_usage = 5;

    // Drenaje de Energía
    if (power_left > 0) {
        if (current_time - last_drain_time >= 1000) { 
            last_drain_time = current_time;
            power_left -= current_usage;
        }

        if (current_night >= 2) {
            int passive_interval = 0;
            if (current_night == 2) passive_interval = 6000;
            else if (current_night == 3) passive_interval = 5000;
            else if (current_night == 4) passive_interval = 4000;
            else if (current_night >= 5) passive_interval = 3000;

            if (passive_interval > 0 && (current_time - last_passive_drain_time >= passive_interval)) {
                last_passive_drain_time = current_time;
                power_left -= 1;
            }
        }
    }
    
    if (power_left < 0) power_left = 0;

    // --- EVENTO: BLACKOUT (APAGÓN) ---
    if (power_left <= 0 && !is_power_out) {
        is_power_out = true;
        powerout_state = 0;

        powerout_step_time = current_time;
        powerout_total_time = current_time;
        
        audio_stop_all_sfx(); 
        audio_stop_music();
        if (sfx_powerdown) audio_play_sfx_chunk(sfx_powerdown);

        if (sfx_ambience2) {
            audio_set_sfx_volume(sfx_ambience2, 50);
            audio_play_sfx_loop_chunk(sfx_ambience2);
        }
        
        left_light_on = false;
        right_light_on = false;
        
        if (left_door_on) {
            left_door_on = false;
            audio_play_sfx_chunk(sfx_door);
        }
        if (right_door_on) {
            right_door_on = false;
            audio_play_sfx_chunk(sfx_door);
        }
        
        if (cam_open) cam_open = false;
    }

    if (is_power_out) {
        if (powerout_state == 0) {
            // Fase 0: Esperando a Freddy (20% cada 5s, o a los 20s forzado)
            if (current_time - powerout_step_time >= 5000) {
                powerout_step_time = current_time;
                if (rand() % 5 == 0) powerout_state = 1; 
            }
            if (current_time - powerout_total_time >= 20000) {
                powerout_state = 1; 
            }

            if (powerout_state == 1) {
                powerout_step_time = current_time;
                powerout_total_time = current_time;
                powerout_flicker_time = current_time;
                if (sfx_musicbox) audio_play_sfx_chunk(sfx_musicbox);
            }

        } else if (powerout_state == 1) {
            // Fase 1: Música sonando y Freddy parpadeando (cada 50ms, 25% de verse)
            if (current_time - powerout_flicker_time >= 50) {
                powerout_flicker_time = current_time;
                show_freddy = (rand() % 4 == 0); 
            }

            // 20% cada 5s de que se corte, o a los 20s forzado
            if (current_time - powerout_step_time >= 5000) {
                powerout_step_time = current_time;
                if (rand() % 5 == 0) powerout_state = 2;
            }
            if (current_time - powerout_total_time >= 20000) {
                powerout_state = 2;
            }

            if (powerout_state == 2) {
                powerout_step_time = current_time;
                powerout_total_time = current_time;
                show_freddy = false; 
                audio_stop_all_sfx(); 
                if (sfx_fan) audio_play_sfx_loop_chunk(sfx_fan); // Zumbido para el cortocircuito
            }

        } else if (powerout_state == 2) {
            // Fase 2: Cortocircuito visual y sonoro (~400ms)
            if (sfx_fan) {
                if (rand() % 2 == 0) audio_set_sfx_volume(sfx_fan, 50);
                else audio_set_sfx_volume(sfx_fan, 0);
            }

            if (current_time - powerout_step_time > 400) {
                powerout_state = 3; 
                powerout_step_time = current_time;
                powerout_total_time = current_time;
                audio_stop_all_sfx(); // Silencio absoluto
                audio_stop_music();
            }

        } else if (powerout_state == 3) {
            // Fase 3: Oscuridad total. 20% cada 2s de atacar, o a los 20s forzado
            if (current_time - powerout_step_time >= 2000) {
                powerout_step_time = current_time;
                if (rand() % 5 == 0) {
                    powerout_state = 4; // ¡Ataque!
                    if (sfx_jumpscare) audio_play_sfx_chunk(sfx_jumpscare);
                }
            }
            if (current_time - powerout_total_time >= 20000 && powerout_state == 3) {
                powerout_state = 4; // Ataque forzado
                if (sfx_jumpscare) audio_play_sfx_chunk(sfx_jumpscare);
            }

        } else if (powerout_state == 4) {
            // Fase 4: JUMPSCARE EN CURSO
            jumpscare_frame += 0.8f; // Velocidad de la animación (casi a 60fps)
            
            // Cuando la animación termina, Game Over (volvemos al menú)
            if (jumpscare_frame >= JUMPSCARE_FRAMES) {
                state_manager_change(STATE_TITLE); 
            }
        }
    }
}


// ==========================================
// RENDERIZADO / DRAW
// ==========================================
void game_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    // 1. Fondo de la Oficina (SOLO FASES 0, 1, 2 Y 3 DEL APAGÓN)
    SDL_Texture* current_background = tex_office_normal;
    
    if (is_power_out) {
        // En Fase 4 (Jumpscare) no dibujamos fondo aquí, lo haremos al final
        if (powerout_state == 0) {
            current_background = tex_office_blackout;
        } else if (powerout_state == 1) {
            if (show_freddy) current_background = tex_office_blackout_freddy;
            else current_background = tex_office_blackout;
        } else if (powerout_state == 2) {
            // Fase 2: Parpadeo del cortocircuito visual
            if (rand() % 2 == 0) current_background = tex_office_blackout;
            else current_background = NULL; // Negro puro
        } else if (powerout_state == 3) {
            // Fase 3: Oscuridad total
            current_background = NULL; 
        } else if (powerout_state == 4) {
            // Fase 4: Jumpscare inminente, fondo negro para preparar
            current_background = NULL; 
        }
    } else {
        // Lógica normal de luces de la oficina (cuando hay batería)
        int flicker_chance = rand() % 10;
        if (left_light_on && flicker_chance > 1) current_background = tex_office_light_L;
        else if (right_light_on && flicker_chance > 1) current_background = tex_office_light_R;
    }

    // Dibujamos el fondo (si lo hay)
    if (current_background) {
        SDL_Rect src_rect = {(int)camera_x, 0, 1280, 720};
        SDL_RenderCopy(renderer, current_background, &src_rect, NULL);
    }

    // --- LAS PUERTAS SE DIBUJAN SIEMPRE ENCAPSULADAS ---
    // Así suben durante el apagón y Freddy se dibujará sobre ellas en Fase 4
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

    // Solo dibujamos los props y la interfaz si TODAVÍA HAY LUZ (No estamos en apagón)
    if (!is_power_out) {
        
        // Botones L
        int state_L = 0;
        if (left_door_on && !left_light_on) state_L = 1;      
        else if (!left_door_on && left_light_on) state_L = 2; 
        else if (left_door_on && left_light_on) state_L = 3;  
        if (tex_button_L[state_L]) {
            SDL_Rect dst_L = {6 - (int)camera_x, 263, 92, 247};
            SDL_RenderCopy(renderer, tex_button_L[state_L], NULL, &dst_L);
        }

        // Botones R
        int state_R = 0;
        if (right_door_on && !right_light_on) state_R = 1;      
        else if (!right_door_on && right_light_on) state_R = 2; 
        else if (right_door_on && right_light_on) state_R = 3;  
        if (tex_button_R[state_R]) {
            SDL_Rect dst_R = {1497 - (int)camera_x, 273, 92, 247};
            SDL_RenderCopy(renderer, tex_button_R[state_R], NULL, &dst_R);
        }

        // Ventilador
        if (tex_fan[fan_frame]) {
            int w, h;
            SDL_QueryTexture(tex_fan[fan_frame], NULL, NULL, &w, &h);
            SDL_Rect dst_rect = {780 - (int)camera_x, 303, w, h};
            SDL_RenderCopy(renderer, tex_fan[fan_frame], NULL, &dst_rect);
        }

        // 3. Interfaz (HUD)
        // Reloj
        int am_x = 1200, am_y = 31, num_x = 1135, num_y = 29; 

        if (current_hour == 0) { 
            if (tex_hour[0] && tex_hour[1]) {
                SDL_Rect dst_1 = {num_x, num_y, 24, 30};
                SDL_RenderCopy(renderer, tex_hour[0], NULL, &dst_1);
                SDL_Rect dst_2 = {num_x + 24, num_y, 24, 30};
                SDL_RenderCopy(renderer, tex_hour[1], NULL, &dst_2);
            }
        } else if (current_hour >= 1 && current_hour <= 5) {
            int h_idx = current_hour - 1;
            if (tex_hour[h_idx]) {
                SDL_Rect dst_h = {num_x + 24, num_y, 24, 30}; 
                SDL_RenderCopy(renderer, tex_hour[h_idx], NULL, &dst_h);
            }
        }
        if (tex_am) {
            SDL_Rect dst_am = {am_x, am_y, 42, 26}; 
            SDL_RenderCopy(renderer, tex_am, NULL, &dst_am);
        }

        // Noche actual
        if (tex_night_text) {
            int night_x = 1148, night_y = 74;
            SDL_Rect dst_night = {night_x, night_y, 63, 14};
            SDL_RenderCopy(renderer, tex_night_text, NULL, &dst_night);
            
            if (current_night >= 1 && current_night <= 5) {
                int n_idx = current_night - 1;
                if (tex_night_num[n_idx]) {
                    int num_x_pos = night_x + 63 + 10; 
                    int num_y_pos = 72;                 
                    SDL_Rect dst_num = {num_x_pos, num_y_pos, 14, 17};
                    SDL_RenderCopy(renderer, tex_night_num[n_idx], NULL, &dst_num);
                }
            }
        }
            
        // Consumo (Usage)
        if (tex_usage_text) {
            SDL_Rect dst_usage = {38, 667, 72, 14};
            SDL_RenderCopy(renderer, tex_usage_text, NULL, &dst_usage);
        }
        if (current_usage >= 1 && current_usage <= 5) {
            int b_idx = current_usage - 1; 
            if (tex_battery[b_idx]) {
                SDL_Rect dst_batt = {120, 657, 103, 32};
                SDL_RenderCopy(renderer, tex_battery[b_idx], NULL, &dst_batt);
            }
        }

        // Energía restante (Power Left)
        if (tex_power_text) {
            SDL_Rect dst_pow = {38, 631, 137, 14};
            SDL_RenderCopy(renderer, tex_power_text, NULL, &dst_pow);
        }

        int display_power = power_left / 10;
        if (display_power > 99) display_power = 99; 
        if (display_power < 0) display_power = 0;

        int tens = display_power / 10, units = display_power % 10;
        int tens_x = 185, units_x = 203, perc_x = 224, batt_y = 624;   

        if (display_power >= 10 && tex_batt_num[tens]) {
            SDL_Rect dst_tens = {tens_x, batt_y, 18, 22};
            SDL_RenderCopy(renderer, tex_batt_num[tens], NULL, &dst_tens);
        }
        if (tex_batt_num[units]) {
            SDL_Rect dst_units = {units_x, batt_y, 18, 22};
            SDL_RenderCopy(renderer, tex_batt_num[units], NULL, &dst_units);
        }
        if (tex_power_percent) {
            SDL_Rect dst_perc = {perc_x, 632, 11, 14};
            SDL_RenderCopy(renderer, tex_power_percent, NULL, &dst_perc);
        }

        // Botón de Tableta
        if (cam_frame <= 0.0f || cam_frame >= (CAM_FRAMES - 1)) {
            if (tex_button_cam) {
                SDL_Rect dst_cam = {340, 630, 600, 60};
                SDL_RenderCopy(renderer, tex_button_cam, NULL, &dst_cam);
            }
        }
    } 

    if (is_power_out && powerout_state == 4) {
        int frame = (int)jumpscare_frame;
        if (frame >= JUMPSCARE_FRAMES) frame = JUMPSCARE_FRAMES - 1; 

        if (tex_jumpscare_freddy[frame]) {
            SDL_Rect src_jumpscare = {0, 0, 1280, 720}; 
            SDL_RenderCopy(renderer, tex_jumpscare_freddy[frame], &src_jumpscare, NULL);
        }
    }

    // 5. Overlays (Cámara por encima de todo)
    if (cam_frame > 0.0f) {
        int current_cam = (int)cam_frame;
        if (current_cam >= CAM_FRAMES) current_cam = CAM_FRAMES - 1;
        if (tex_cam[current_cam]) {
            SDL_Rect dst_cam_anim = {0, 0, 1280, 720}; 
            SDL_RenderCopy(renderer, tex_cam[current_cam], NULL, &dst_cam_anim);
        }
    }
}


// ==========================================
// LIMPIEZA / CLEANUP
// ==========================================
void game_cleanup(void) {
    // Entorno
    if (tex_office_normal) { SDL_DestroyTexture(tex_office_normal); tex_office_normal = NULL; }
    if (tex_office_light_L) { SDL_DestroyTexture(tex_office_light_L); tex_office_light_L = NULL; }
    if (tex_office_light_R) { SDL_DestroyTexture(tex_office_light_R); tex_office_light_R = NULL; }

    // Puertas y Botones
    for (int i = 0; i < DOOR_FRAMES; i++) {
        if (tex_door_L_close[i]) { SDL_DestroyTexture(tex_door_L_close[i]); tex_door_L_close[i] = NULL; }
        if (tex_door_R_close[i]) { SDL_DestroyTexture(tex_door_R_close[i]); tex_door_R_close[i] = NULL; }
    }
    for (int i = 0; i < 4; i++) {
        if (tex_button_L[i]) { SDL_DestroyTexture(tex_button_L[i]); tex_button_L[i] = NULL; }
        if (tex_button_R[i]) { SDL_DestroyTexture(tex_button_R[i]); tex_button_R[i] = NULL; }
    }

    // Tableta
    if (tex_button_cam) { SDL_DestroyTexture(tex_button_cam); tex_button_cam = NULL; }
    for (int i = 0; i < CAM_FRAMES; i++) {
        if (tex_cam[i]) { SDL_DestroyTexture(tex_cam[i]); tex_cam[i] = NULL; }
    }

    // HUD
    if (tex_am) { SDL_DestroyTexture(tex_am); tex_am = NULL; }
    if (tex_night_text) { SDL_DestroyTexture(tex_night_text); tex_night_text = NULL; }
    if (tex_usage_text) { SDL_DestroyTexture(tex_usage_text); tex_usage_text = NULL; }
    if (tex_power_text) { SDL_DestroyTexture(tex_power_text); tex_power_text = NULL; }
    if (tex_power_percent) { SDL_DestroyTexture(tex_power_percent); tex_power_percent = NULL; }
    
    for (int i = 0; i < 5; i++) {
        if (tex_hour[i]) { SDL_DestroyTexture(tex_hour[i]); tex_hour[i] = NULL; }
        if (tex_night_num[i]) { SDL_DestroyTexture(tex_night_num[i]); tex_night_num[i] = NULL; }
        if (tex_battery[i]) { SDL_DestroyTexture(tex_battery[i]); tex_battery[i] = NULL; }
    }
    for (int i = 0; i < 10; i++) {
        if (tex_batt_num[i]) { SDL_DestroyTexture(tex_batt_num[i]); tex_batt_num[i] = NULL; }
    }

    if (tex_office_blackout) { SDL_DestroyTexture(tex_office_blackout); tex_office_blackout = NULL; }
    if (tex_office_blackout_freddy) { SDL_DestroyTexture(tex_office_blackout_freddy); tex_office_blackout_freddy = NULL; }
    
    for (int i = 0; i < JUMPSCARE_FRAMES; i++) {
        if (tex_jumpscare_freddy[i]) {
            SDL_DestroyTexture(tex_jumpscare_freddy[i]);
            tex_jumpscare_freddy[i] = NULL;
        }
    }

    // Audio
    audio_stop_music();
    audio_stop_all_sfx();
    if (sfx_fan) { audio_free_sfx(sfx_fan); sfx_fan = NULL; }
    if (sfx_light) { audio_free_sfx(sfx_light); sfx_light = NULL; }
    if (sfx_door) { audio_free_sfx(sfx_door); sfx_door = NULL; }
    if (sfx_cam_up) { audio_free_sfx(sfx_cam_up); sfx_cam_up = NULL; }
    if (sfx_cam_down) { audio_free_sfx(sfx_cam_down); sfx_cam_down = NULL; }
    if (sfx_powerdown) { audio_free_sfx(sfx_powerdown); sfx_powerdown = NULL; }
    if (sfx_musicbox) { audio_free_sfx(sfx_musicbox); sfx_musicbox = NULL;}
    if (sfx_jumpscare) { audio_free_sfx(sfx_jumpscare); sfx_jumpscare = NULL; }
    if (sfx_ambience2) { audio_free_sfx(sfx_ambience2); sfx_ambience2 = NULL;}
}