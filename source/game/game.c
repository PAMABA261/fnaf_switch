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

// Entorno de la Oficina
static float camera_x = 160.0f;
static SDL_Texture* tex_office_normal = NULL;
static SDL_Texture* tex_office_light_L = NULL;
static SDL_Texture* tex_office_light_R = NULL;
static SDL_Texture* tex_fan[3] = {NULL};
static SDL_Texture* tex_office_blackout = NULL;
static SDL_Texture* tex_office_blackout_freddy = NULL;
static SDL_Texture* tex_office_bonnie = NULL; 
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

// Audio de la oficina
static Mix_Chunk* sfx_fan = NULL;
static Mix_Chunk* sfx_light = NULL;
static Mix_Chunk* sfx_door = NULL;
static int channel_light_L = -1;
static int channel_light_R = -1;
static int channel_fan = -1; 

static Mix_Chunk* sfx_circus = NULL;
static Mix_Chunk* sfx_pounding = NULL;
static Mix_Chunk* sfx_hallucination[4] = {NULL};
static Mix_Chunk* sfx_window_scare = NULL; 
static Mix_Chunk* sfx_error = NULL;

static bool bonnie_scare_played = false; 

void game_init(void) {
    // 1. INICIALIZAR SUBSISTEMAS
    hud_init(); 
    power_system_init(); 
    camera_system_init(); 
    animatronics_init(current_night);

    // 2. CARGA DE GRÁFICOS DE LA OFICINA
    tex_office_normal = graphics_load_texture(IMG_OFFICE);
    tex_office_light_L = graphics_load_texture(IMG_OFFICE_LIGHT_L);
    tex_office_light_R = graphics_load_texture(IMG_OFFICE_LIGHT_R);
    tex_fan[0] = graphics_load_texture(IMG_FAN_1);
    tex_fan[1] = graphics_load_texture(IMG_FAN_2);
    tex_fan[2] = graphics_load_texture(IMG_FAN_3);
    tex_office_blackout = graphics_load_texture(IMG_OFFICE_BLACK_OUT);
    tex_office_blackout_freddy = graphics_load_texture(IMG_OFFICE_BLACK_OUT_FREDDY);
    tex_office_bonnie = graphics_load_texture(IMG_OFFICE_BONNIE); 

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

    // 3. CARGA DE AUDIO DE LA OFICINA
    sfx_fan = audio_load_sfx("romfs:/sfx/Buzz_Fan_Florescent2.wav");
    sfx_light = audio_load_sfx("romfs:/sfx/BallastHumMedium2.wav");
    sfx_door = audio_load_sfx("romfs:/sfx/SFXBible_12478.wav");

    sfx_circus = audio_load_sfx("romfs:/sfx/circus.wav");
    sfx_pounding = audio_load_sfx("romfs:/sfx/DOOR_POUNDING_ME_D0291401.wav");
    sfx_window_scare = audio_load_sfx("romfs:/sfx/windowscare.wav"); 
    sfx_error = audio_load_sfx("romfs:/sfx/error.wav");

    sfx_hallucination[0] = audio_load_sfx("romfs:/sfx/COMPUTER_DIGITAL_L2076505.wav");
    sfx_hallucination[1] = audio_load_sfx("romfs:/sfx/garble1.wav");
    sfx_hallucination[2] = audio_load_sfx("romfs:/sfx/garble2.wav");
    sfx_hallucination[3] = audio_load_sfx("romfs:/sfx/garble3.wav");

    // 4. INICIALIZACIÓN DE VARIABLES
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

    // 5. REPRODUCCIÓN DE AUDIO INICIAL
    audio_play_music("romfs:/sfx/ColdPresc_B.wav");
    audio_set_music_volume(50); 
    if (sfx_fan) {
        audio_set_sfx_volume(sfx_fan, 25); 
        channel_fan = audio_play_sfx_loop_chunk(sfx_fan); 
    }
}

void game_update(void) {
    // 1. ACTUALIZAR SUBSISTEMAS
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
        
        channel_light_L = -1;
        channel_light_R = -1;
        channel_fan = -1;

        left_light_on = false;
        right_light_on = false;
        if (left_door_on) { left_door_on = false; audio_play_sfx_chunk(sfx_door); }
        if (right_door_on) { right_door_on = false; audio_play_sfx_chunk(sfx_door); }
        camera_system_force_close();
    }

    // 2. COMPROBAR VICTORIA
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

    // 3. LOGICA PRINCIPAL DE LA OFICINA
    random_sound_timer++;

    if (random_sound_timer % 300 == 0) {
        if ((rand() % 30) == 0) audio_play_sfx_chunk(sfx_circus);
    }
    if (random_sound_timer % 600 == 0) {
        if ((rand() % 50) == 0) {
            int random_vol = 10 + (rand() % 40); 
            audio_set_sfx_volume(sfx_pounding, random_vol);
            audio_play_sfx_chunk(sfx_pounding);
        }
    }
    
    // Alucinaciones...
    if (!is_power_out) {
        animatronics_update(left_door_on, right_door_on, camera_system_is_open());
        
        if (animatronics_get_bonnie_room() != ROOM_DOOR_LEFT) {
            bonnie_scare_played = false;
        }

        if (animatronics_get_bonnie_room() == ROOM_OFFICE && left_light_on) {
            left_light_on = false;
            audio_stop_channel(channel_light_L);
            channel_light_L = -1;
        }

        if (random_sound_timer % 300 == 0) {
            if ((rand() % 100) == 0) { 
                hallucination_timer = 60; 
                int random_snd = rand() % 4;
                if (sfx_hallucination[random_snd]) {
                    audio_set_sfx_volume(sfx_hallucination[random_snd], 100); 
                    audio_play_sfx_chunk(sfx_hallucination[random_snd]);
                }
            }
        }
        if (hallucination_timer > 0) {
            hallucination_timer--;
            if ((rand() % 2) == 0) current_hallucination = rand() % 4; 
            else current_hallucination = -1; 
        } else current_hallucination = -1; 
    } else {
        hallucination_timer = 0; 
        current_hallucination = -1;
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
            }
        }
    }

    fan_timer++;
    // Solo animamos el ventilador si hay energía
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

    // DIBUJAR FONDO Y LUCES/APAGÓN
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
            current_background = tex_office_light_R;
        }
    }

    if (current_background) {
        SDL_Rect src_rect = {(int)camera_x, 0, 1280, 720};
        SDL_RenderCopy(renderer, current_background, &src_rect, NULL);
    }

    // DIBUJAR PUERTAS
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

    // DIBUJAR CÁMARA DE SEGURIDAD (Capa de fondo)
    camera_system_draw_room();
    camera_system_draw_ui();
    camera_system_draw_animation();

    // DIBUJAR UI Y OBJETOS SI HAY LUZ
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
    camera_system_draw_animation();

    if (hallucination_timer > 0 && !is_power_out && current_hallucination >= 0) {
        if (tex_hallucinations[current_hallucination]) {
            SDL_Rect dst_rect = {0, 0, 1280, 720};
            SDL_RenderCopy(renderer, tex_hallucinations[current_hallucination], NULL, &dst_rect);
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

    audio_stop_music();
    audio_stop_all_sfx();
    if (sfx_fan) { audio_free_sfx(sfx_fan); sfx_fan = NULL; }
    if (sfx_light) { audio_free_sfx(sfx_light); sfx_light = NULL; }
    if (sfx_door) { audio_free_sfx(sfx_door); sfx_door = NULL; }
    if (sfx_circus) audio_free_sfx(sfx_circus);
    if (sfx_pounding) audio_free_sfx(sfx_pounding);
    if (sfx_window_scare) { audio_free_sfx(sfx_window_scare); sfx_window_scare = NULL; } 
    if (sfx_error) { audio_free_sfx(sfx_error); sfx_error = NULL; } 
    for (int i = 0; i < 4; i++) {
        if (sfx_hallucination[i]) { 
            audio_free_sfx(sfx_hallucination[i]); 
            sfx_hallucination[i] = NULL; 
        }
    }
}