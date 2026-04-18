#include "game/power_system.h"
#include "game/hud.h" 
#include "game/assets.h"
#include "game/state_manager.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include <stdlib.h>
#include <SDL2/SDL.h>

int power_left = 999;
int current_usage = 1;
bool is_power_out = false;
int powerout_state = 0;
bool show_freddy = false;
float jumpscare_frame = 0.0f;
static const float JUMPSCARE_ANIM_SPEED = 0.60f;

static Uint64 last_drain_time = 0;
static Uint64 last_passive_drain_time = 0;
static Uint64 powerout_step_time = 0;
static Uint64 powerout_total_time = 0;
static Uint64 powerout_flicker_time = 0;

static SDL_Texture* tex_usage_text = NULL;
static SDL_Texture* tex_battery[5] = {NULL};
static SDL_Texture* tex_power_text = NULL;
static SDL_Texture* tex_power_percent = NULL;
static SDL_Texture* tex_batt_num[10] = {NULL};
static SDL_Texture* tex_jumpscare_freddy[JUMPSCARE_FRAMES] = {NULL};

static Mix_Chunk* sfx_powerdown = NULL;
static Mix_Chunk* sfx_musicbox = NULL;
static Mix_Chunk* sfx_jumpscare = NULL;
static Mix_Chunk* sfx_ambience2 = NULL;
static Mix_Chunk* sfx_fan_blackout = NULL;

// --- CAMBIO: Canales para el apagado quirúrgico ---
static int ch_ambience2 = -1;
static int ch_fan_blackout = -1;
static int ch_musicbox = -1;

#define CAM_FRAMES 11

void power_system_init(void) {
    // 1. Carga de Gráficos
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

    // 2. Carga de Audio del apagón
    sfx_powerdown = audio_load_sfx("romfs:/sfx/powerdown.wav");
    sfx_musicbox = audio_load_sfx("romfs:/sfx/music_box.wav");
    sfx_jumpscare = audio_load_sfx("romfs:/sfx/XSCREAM.wav");
    sfx_ambience2 = audio_load_sfx("romfs:/sfx/ambience2.wav");
    sfx_fan_blackout = audio_load_sfx("romfs:/sfx/Buzz_Fan_Florescent2.wav");

    // 3. Inicialización
    power_left = 999;
    current_usage = 1;
    is_power_out = false;
    powerout_state = 0;
    show_freddy = false;
    jumpscare_frame = 0.0f;
    last_drain_time = SDL_GetTicks64();
    last_passive_drain_time = SDL_GetTicks64();
    
    // Resetear canales
    ch_ambience2 = -1;
    ch_fan_blackout = -1;
    ch_musicbox = -1;
}

bool power_system_update(int items_on) {
    Uint64 current_time = SDL_GetTicks64();
    bool just_triggered_blackout = false;

    if (!is_power_out) {
        current_usage = 1 + items_on;
        if (current_usage > 5) current_usage = 5;

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
        
        if (power_left <= 0) power_left = 0;

        // Lógica del Blackout (Apagón) - Justo en el instante que se va la luz
        if (power_left <= 0) {
            is_power_out = true;
            powerout_state = 0;
            powerout_step_time = current_time;
            powerout_total_time = current_time;
            just_triggered_blackout = true; 
            
            // --- ELIMINADA la bomba nuclear. Solo detenemos la música de ambiente
            audio_stop_music();
            if (sfx_powerdown) audio_play_sfx_chunk(sfx_powerdown);

            if (sfx_ambience2) {
                audio_set_sfx_volume(sfx_ambience2, 50);
                ch_ambience2 = audio_play_sfx_loop_chunk(sfx_ambience2); // Guardamos canal
            }
        }
    } else {
        // Máquina de estados del apagón
        if (powerout_state == 0) {
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
                if (sfx_musicbox) ch_musicbox = audio_play_sfx_chunk(sfx_musicbox); // Guardamos canal
            }

        } else if (powerout_state == 1) {
            if (current_time - powerout_flicker_time >= 50) {
                powerout_flicker_time = current_time;
                show_freddy = (rand() % 4 == 0); 
            }

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
                
                // --- APAGADO QUIRÚRGICO DE LA MÚSICA DE FREDDY Y EL AMBIENTE ---
                if (ch_musicbox != -1) audio_stop_channel(ch_musicbox);
                if (ch_ambience2 != -1) audio_stop_channel(ch_ambience2);
                
                if (sfx_fan_blackout) ch_fan_blackout = audio_play_sfx_loop_chunk(sfx_fan_blackout); // Guardamos canal
            }

        } else if (powerout_state == 2) {
            if (sfx_fan_blackout) {
                if (rand() % 2 == 0) audio_set_sfx_volume(sfx_fan_blackout, 50);
                else audio_set_sfx_volume(sfx_fan_blackout, 0);
            }

            if (current_time - powerout_step_time > 400) {
                powerout_state = 3; 
                powerout_step_time = current_time;
                powerout_total_time = current_time;
                
                // --- APAGADO QUIRÚRGICO DEL VENTILADOR ---
                if (ch_fan_blackout != -1) audio_stop_channel(ch_fan_blackout);
            }

        } else if (powerout_state == 3) {
            if (current_time - powerout_step_time >= 2000) {
                powerout_step_time = current_time;
                if (rand() % 5 == 0) {
                    powerout_state = 4; 
                    if (sfx_jumpscare) audio_play_sfx_chunk(sfx_jumpscare);
                }
            }
            if (current_time - powerout_total_time >= 20000 && powerout_state == 3) {
                powerout_state = 4; 
                if (sfx_jumpscare) audio_play_sfx_chunk(sfx_jumpscare);
            }

        } else if (powerout_state == 4) {
            jumpscare_frame += JUMPSCARE_ANIM_SPEED;  
            if (jumpscare_frame >= JUMPSCARE_FRAMES) {
                state_manager_change(STATE_TITLE); 
            }
        }
    }

    return just_triggered_blackout;
}

void power_system_draw_hud(bool cam_open, float cam_frame) {
    if (is_power_out) return; // Si no hay luz, no hay UI de batería

    SDL_Renderer* renderer = graphics_get_renderer();

    // Dibujamos la batería 
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
}

void power_system_draw_jumpscare(void) {
    if (is_power_out && powerout_state == 4) {
        int frame = (int)jumpscare_frame;
        if (frame >= JUMPSCARE_FRAMES) frame = JUMPSCARE_FRAMES - 1; 

        if (tex_jumpscare_freddy[frame]) {
            SDL_Rect src_jumpscare = {0, 0, 1280, 720}; 
            SDL_RenderCopy(graphics_get_renderer(), tex_jumpscare_freddy[frame], &src_jumpscare, NULL);
        }
    }
}

void power_system_cleanup(void) {
    if (tex_usage_text) { SDL_DestroyTexture(tex_usage_text); tex_usage_text = NULL; }
    if (tex_power_text) { SDL_DestroyTexture(tex_power_text); tex_power_text = NULL; }
    if (tex_power_percent) { SDL_DestroyTexture(tex_power_percent); tex_power_percent = NULL; }
    
    for (int i = 0; i < 5; i++) {
        if (tex_battery[i]) { SDL_DestroyTexture(tex_battery[i]); tex_battery[i] = NULL; }
    }
    for (int i = 0; i < 10; i++) {
        if (tex_batt_num[i]) { SDL_DestroyTexture(tex_batt_num[i]); tex_batt_num[i] = NULL; }
    }
    for (int i = 0; i < JUMPSCARE_FRAMES; i++) {
        if (tex_jumpscare_freddy[i]) { SDL_DestroyTexture(tex_jumpscare_freddy[i]); tex_jumpscare_freddy[i] = NULL; }
    }

    if (sfx_powerdown) { audio_free_sfx(sfx_powerdown); sfx_powerdown = NULL; }
    if (sfx_musicbox) { audio_free_sfx(sfx_musicbox); sfx_musicbox = NULL;}
    if (sfx_jumpscare) { audio_free_sfx(sfx_jumpscare); sfx_jumpscare = NULL; }
    if (sfx_ambience2) { audio_free_sfx(sfx_ambience2); sfx_ambience2 = NULL;}
    if (sfx_fan_blackout) { audio_free_sfx(sfx_fan_blackout); sfx_fan_blackout = NULL;}
}

void power_system_subtract_power(int amount) {
    if (is_power_out) return;
    power_left -= amount;
    if (power_left < 0) power_left = 0;
}