#include "game/assets.h"
#include "game/state_manager.h"
#include "engine/graphics.h"
#include "engine/input.h"
#include "engine/audio.h"
#include <SDL2/SDL.h>

static SDL_Texture* tex_office = NULL;
static SDL_Texture* tex_fan[3] = {NULL, NULL, NULL};

static SDL_Texture* tex_button_L[4] = {NULL, NULL, NULL, NULL};
static SDL_Texture* tex_button_R[4] = {NULL, NULL, NULL, NULL};

static bool left_door_on = false;
static bool left_light_on = false;
static bool right_door_on = false;
static bool right_light_on = false;

static int fan_frame = 0;
static int fan_timer = 0;
#define FAN_ANIM_SPEED 1 // Cambia de imagen cada frame

static Mix_Chunk* sfx_fan = NULL; //Guardamos el sonido del ventilador para el bucle

static float camera_x = 160.0f; 

void game_init(void) {
    tex_office = graphics_load_texture(IMG_OFFICE);

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

    audio_play_music("romfs:/sfx/ColdPresc_B.wav");
    sfx_fan = audio_load_sfx("romfs:/sfx/Buzz_Fan_Florescent2.wav");
    audio_play_sfx_loop_chunk(sfx_fan);
}

void game_update(void) {

    // Leemos el Stick Izquierdo (eje X)
    s16 stick_x = input_get_stick_x(0);

    // Aplicamos una "zona muerta": si el stick se mueve poco, no hacemos nada.
    // Esto evita que la cámara tiemble.
   if (stick_x > 7000 || stick_x < -7000) {
        // Convertimos el valor del stick (-32768 a 32767) a una velocidad suave
        float speed = (stick_x / 32767.0f) * 5.0f;
        camera_x += speed;
    }

    // Ponemos límites, no queremos ver más allá de la imagen
    if (camera_x < 0)   camera_x = 0;
    if (camera_x > 320) camera_x = 320;

    // Si pulsas Plus, volvemos al título
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

    if (input_get_button_down(HidNpadButton_L)) {
        left_door_on = !left_door_on; 
    }

    if (input_get_button_down(HidNpadButton_ZL)) {
        left_light_on = !left_light_on;
        if (left_light_on) right_light_on = false; 
    }

    if (input_get_button_down(HidNpadButton_R)) {
        right_door_on = !right_door_on;
    }

    if (input_get_button_down(HidNpadButton_ZR)) {
        right_light_on = !right_light_on;
        if (right_light_on) left_light_on = false;
    }
}

void game_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    if (tex_office) {
        SDL_Rect src_rect = {(int)camera_x, 0, 1280, 720};

        SDL_RenderCopy(renderer, tex_office, &src_rect, NULL);
    }

    int state_L = 0; // Por defecto: Apagado
    if (left_door_on && !left_light_on) state_L = 1;      // Solo Puerta
    else if (!left_door_on && left_light_on) state_L = 2; // Solo Luz
    else if (left_door_on && left_light_on) state_L = 3;  // Ambos

    if (tex_button_L[state_L]) {
        int btn_L_x = 6 - (int)camera_x; 
        int btn_L_y = 263;
        SDL_Rect dst_L = {btn_L_x, btn_L_y, 92, 247};
        SDL_RenderCopy(renderer, tex_button_L[state_L], NULL, &dst_L);
    }

    int state_R = 0; // Por defecto: Apagado
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
    if (tex_office) {
        SDL_DestroyTexture(tex_office);
        tex_office = NULL;
    }

    audio_stop_music();
    audio_stop_all_sfx();

    if (sfx_fan) {
        audio_free_sfx(sfx_fan);
        sfx_fan = NULL;
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
}