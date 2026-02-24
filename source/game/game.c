#include "game/assets.h"
#include "game/state_manager.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/audio.h"
#include <stdlib.h>
#include <SDL2/SDL.h>

static SDL_Texture* tex_office_normal = NULL;
static SDL_Texture* tex_office_light_L = NULL;
static SDL_Texture* tex_office_light_R = NULL;
static SDL_Texture* tex_fan[3] = {NULL, NULL, NULL};

static SDL_Texture* tex_button_L[4] = {NULL, NULL, NULL, NULL};
static SDL_Texture* tex_button_R[4] = {NULL, NULL, NULL, NULL};

#define DOOR_FRAMES 15
static SDL_Texture* tex_door_L_close[DOOR_FRAMES] = {NULL};
static SDL_Texture* tex_door_R_close[DOOR_FRAMES] = {NULL};

static bool left_door_on = false;
static bool left_light_on = false;
static bool right_door_on = false;
static bool right_light_on = false;

static float door_L_frame = 0.0f;
static float door_R_frame = 0.0f;

static const float DOOR_ANIM_SPEED = 0.6f; // Velocidad de la animación de las puertas

static int fan_frame = 0;
static int fan_timer = 0;
#define FAN_ANIM_SPEED 1 // Cambia de imagen cada frame

static Mix_Chunk* sfx_fan = NULL; //Guardamos el sonido del ventilador para el bucle
static Mix_Chunk* sfx_light = NULL; //Guardamos el sonido de la luz para el bucle
static Mix_Chunk* sfx_door = NULL; //Guardamos el sonido del impacto de la puerta

// Canales para controlar el zumbido de las luces individualmente
static int channel_light_L = -1;
static int channel_light_R = -1;

static float camera_x = 160.0f; 

void game_init(void) {
    tex_office_normal = graphics_load_texture(IMG_OFFICE);
    tex_office_light_L = graphics_load_texture(IMG_OFFICE_LIGHT_L);
    tex_office_light_R = graphics_load_texture(IMG_OFFICE_LIGHT_R);

    tex_fan[0] = graphics_load_texture(IMG_FAN_1);
    tex_fan[1] = graphics_load_texture(IMG_FAN_2);
    tex_fan[2] = graphics_load_texture(IMG_FAN_3);

    tex_button_L[0] = graphics_load_texture(IMG_BUTTON_L_1);
    tex_button_L[1] = graphics_load_texture(IMG_BUTTON_L_2);
    tex_button_L[2] = graphics_load_texture(IMG_BUTTON_L_3);
    tex_button_L[3] = graphics_load_texture(IMG_BUTTON_L_4);

    tex_button_R[0] = graphics_load_texture(IMG_BUTTON_R_1);
    tex_button_R[1] = graphics_load_texture(IMG_BUTTON_R_2);
    tex_button_R[2] = graphics_load_texture(IMG_BUTTON_R_3);
    tex_button_R[3] = graphics_load_texture(IMG_BUTTON_R_4);

    const char* paths_door_L[DOOR_FRAMES] = {
        IMG_DOOR_L_CLOSE_1, IMG_DOOR_L_CLOSE_2, IMG_DOOR_L_CLOSE_3, 
        IMG_DOOR_L_CLOSE_4, IMG_DOOR_L_CLOSE_5, IMG_DOOR_L_CLOSE_6, 
        IMG_DOOR_L_CLOSE_7, IMG_DOOR_L_CLOSE_8, IMG_DOOR_L_CLOSE_9, 
        IMG_DOOR_L_CLOSE_10, IMG_DOOR_L_CLOSE_11, IMG_DOOR_L_CLOSE_12, 
        IMG_DOOR_L_CLOSE_13, IMG_DOOR_L_CLOSE_14, IMG_DOOR_L_CLOSE_15
    };

    const char* paths_door_R[DOOR_FRAMES] = {
        IMG_DOOR_R_CLOSE_1, IMG_DOOR_R_CLOSE_2, IMG_DOOR_R_CLOSE_3, 
        IMG_DOOR_R_CLOSE_4, IMG_DOOR_R_CLOSE_5, IMG_DOOR_R_CLOSE_6, 
        IMG_DOOR_R_CLOSE_7, IMG_DOOR_R_CLOSE_8, IMG_DOOR_R_CLOSE_9, 
        IMG_DOOR_R_CLOSE_10, IMG_DOOR_R_CLOSE_11, IMG_DOOR_R_CLOSE_12, 
        IMG_DOOR_R_CLOSE_13, IMG_DOOR_R_CLOSE_14, IMG_DOOR_R_CLOSE_15
    };

    for (int i = 0; i < DOOR_FRAMES; i++) {
        tex_door_L_close[i] = graphics_load_texture(paths_door_L[i]);
        tex_door_R_close[i] = graphics_load_texture(paths_door_R[i]);
    }

    audio_play_music("romfs:/sfx/ColdPresc_B.wav");
    audio_set_music_volume(50); 
    
    sfx_fan = audio_load_sfx("romfs:/sfx/Buzz_Fan_Florescent2.wav");
    audio_set_sfx_volume(sfx_fan, 50); 
    audio_play_sfx_loop_chunk(sfx_fan); 

    sfx_light = audio_load_sfx("romfs:/sfx/BallastHumMedium2.wav");

    sfx_door = audio_load_sfx("romfs:/sfx/SFXBible_12478.wav");
}

void game_update(void) {

    // Leemos el Stick Izquierdo (eje X)
    s16 stick_x = input_get_stick_x(0);

    // Aplicamos una "zona muerta"
   if (stick_x > 7000 || stick_x < -7000) {
        float speed = (stick_x / 32767.0f) * 5.0f;
        camera_x += speed;
    }

    if (camera_x < 0)   camera_x = 0;
    if (camera_x > 320) camera_x = 320;

    if (input_get_button_down(HidNpadButton_Plus)) {
        state_manager_change(STATE_TITLE);
    }

    // Animación del ventilador
    fan_timer++;
    if (fan_timer >= FAN_ANIM_SPEED) {
        fan_timer = 0;
        fan_frame++;

        if (fan_frame > 2) {
            fan_frame = 0;
        }
    }

    // --- PUERTA IZQUIERDA ---
    if (input_get_button_down(HidNpadButton_L)) {
        if (door_L_frame <= 0.0f || door_L_frame >= DOOR_FRAMES -1) { 
            left_door_on = !left_door_on; 
            audio_play_sfx_chunk(sfx_door); 
        }
    }

    // --- LUZ IZQUIERDA ---
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

    // --- PUERTA DERECHA ---
    if (input_get_button_down(HidNpadButton_R)) {
        if (door_R_frame <= 0.0f || door_R_frame >= DOOR_FRAMES -1) { 
            right_door_on = !right_door_on;
            audio_play_sfx_chunk(sfx_door); 
        }
    }

    // --- LUZ DERECHA ---
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

    // --- LÓGICA DE ANIMACIÓN DE PUERTAS ---
    if (left_door_on) {
        if (door_L_frame < DOOR_FRAMES -1) {
            door_L_frame += DOOR_ANIM_SPEED;
            if (door_L_frame > DOOR_FRAMES -1) {
                door_L_frame = DOOR_FRAMES -1;
            }
        }
    } else {
        if (door_L_frame > 0.0f) {
            door_L_frame -= DOOR_ANIM_SPEED;
            if (door_L_frame < 0) {
                door_L_frame = 0;
            }
        }
    }

    if (right_door_on) {
        if (door_R_frame < DOOR_FRAMES -1) {
            door_R_frame += DOOR_ANIM_SPEED;
            if (door_R_frame > DOOR_FRAMES -1) {
                door_R_frame = DOOR_FRAMES -1;
            }
        }
    } else {
        if (door_R_frame > 0.0f) {
            door_R_frame -= DOOR_ANIM_SPEED;
            if (door_R_frame < 0) {
                door_R_frame = 0;
            }
        }
    }
}

void game_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    SDL_Texture* current_background = tex_office_normal;

    int flicker_chance = rand() % 10;

    if (left_light_on && flicker_chance > 1) {
        current_background = tex_office_light_L;
    } else if (right_light_on && flicker_chance > 1) {
        current_background = tex_office_light_R;
    }

    if (current_background) {
        SDL_Rect src_rect = {(int)camera_x, 0, 1280, 720};
        SDL_RenderCopy(renderer, current_background, &src_rect, NULL);
    }

    int current_L = (int)door_L_frame;
    if (current_L >= 0 && tex_door_L_close[current_L]) {
        int door_L_x = 72 - (int)camera_x; 
        int door_L_y = -1;
        SDL_Rect dst_L = {door_L_x, door_L_y, 223, 720};
        SDL_RenderCopy(renderer, tex_door_L_close[current_L], NULL, &dst_L);
    }

    int current_R = (int)door_R_frame;
    if (current_R >= 0 && tex_door_R_close[current_R]) {
        int door_R_x = 1270 - (int)camera_x; 
        int door_R_y = -2;
        SDL_Rect dst_R = {door_R_x, door_R_y, 248, 720};
        SDL_RenderCopy(renderer, tex_door_R_close[current_R], NULL, &dst_R);
    }

    int state_L = 0; // Apagado
    if (left_door_on && !left_light_on) state_L = 1;      // Solo Puerta
    else if (!left_door_on && left_light_on) state_L = 2; // Solo Luz
    else if (left_door_on && left_light_on) state_L = 3;  // Ambos

    if (tex_button_L[state_L]) {
        int btn_L_x = 6 - (int)camera_x; 
        int btn_L_y = 263;
        SDL_Rect dst_L = {btn_L_x, btn_L_y, 92, 247};
        SDL_RenderCopy(renderer, tex_button_L[state_L], NULL, &dst_L);
    }

    int state_R = 0; // Apagado
    if (right_door_on && !right_light_on) state_R = 1;      // Solo Puerta
    else if (!right_door_on && right_light_on) state_R = 2; // Solo Luz
    else if (right_door_on && right_light_on) state_R = 3;  // Ambos

    if (tex_button_R[state_R]) {
        int btn_R_x = 1497 - (int)camera_x;
        int btn_R_y = 273;
        SDL_Rect dst_R = {btn_R_x, btn_R_y, 92, 247};
        SDL_RenderCopy(renderer, tex_button_R[state_R], NULL, &dst_R);
    }

    if (tex_fan[fan_frame]) {
        int w, h;
        SDL_QueryTexture(tex_fan[fan_frame], NULL, NULL, &w, &h);
        int base_x = 780; 
        int base_y = 303;
        int fan_screen_x = base_x - (int)camera_x; // Ajustamos la posición del ventilador según la cámara
        SDL_Rect dst_rect = {fan_screen_x, base_y, w, h};
        SDL_RenderCopy(renderer, tex_fan[fan_frame], NULL, &dst_rect);
    }
}

void game_cleanup(void) {
    if (tex_office_normal) {
        SDL_DestroyTexture(tex_office_normal);
        tex_office_normal = NULL;
    }
    if (tex_office_light_L) {
        SDL_DestroyTexture(tex_office_light_L);
        tex_office_light_L = NULL;
    }
    if (tex_office_light_R) {
        SDL_DestroyTexture(tex_office_light_R);
        tex_office_light_R = NULL;
    }

    audio_stop_music();
    audio_stop_all_sfx();

    if (sfx_fan) {
        audio_free_sfx(sfx_fan);
        sfx_fan = NULL;
    }
    
    if (sfx_light) {
        audio_free_sfx(sfx_light);
        sfx_light = NULL;
    }
    if (sfx_door) {
        audio_free_sfx(sfx_door);
        sfx_door = NULL;
    }

    for (int i = 0; i < 4; i++) {
        if (tex_button_L[i]) {
            SDL_DestroyTexture(tex_button_L[i]);
            tex_button_L[i] = NULL;
        }
        if (tex_button_R[i]) {
            SDL_DestroyTexture(tex_button_R[i]);
            tex_button_R[i] = NULL;
        }
    }

    for (int i = 0; i < DOOR_FRAMES; i++) {
        if (tex_door_L_close[i]) {
            SDL_DestroyTexture(tex_door_L_close[i]);
            tex_door_L_close[i] = NULL;
        }
        if (tex_door_R_close[i]) {
            SDL_DestroyTexture(tex_door_R_close[i]);
            tex_door_R_close[i] = NULL;
        }
    }
}