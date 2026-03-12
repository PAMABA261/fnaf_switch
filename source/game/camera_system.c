#include "game/camera_system.h"
#include "game/assets.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include "engine/input.h"
#include <SDL2/SDL.h>

static const float CAM_ANIM_SPEED = 0.6f;
static float cam_frame = 0.0f;
static bool cam_open = false;

static SDL_Texture* tex_cam[CAM_FRAMES] = {NULL};
static SDL_Texture* tex_button_cam = NULL; 
static SDL_Texture* tex_rooms[11] = {NULL}; 

static CameraID current_cam = CAM_1A; 
static float cam_pan_x = 0.0f;
static int cam_pan_dir = 1; 
static int cam_pause_timer = 0; 
static const float CAM_PAN_SPEED = 1.0f; 

static Mix_Chunk* sfx_cam_up = NULL;
static Mix_Chunk* sfx_cam_down = NULL;

void camera_system_init(void) {
    tex_button_cam = graphics_load_texture(IMG_BUTTON_CAM);
    const char* paths_cam[CAM_FRAMES] = {
        IMG_CAM_OPEN_1, IMG_CAM_OPEN_2, IMG_CAM_OPEN_3, IMG_CAM_OPEN_4, IMG_CAM_OPEN_5,
        IMG_CAM_OPEN_6, IMG_CAM_OPEN_7, IMG_CAM_OPEN_8, IMG_CAM_OPEN_9, IMG_CAM_OPEN_10, 
        IMG_CAM_OPEN_11
    };
    for (int i = 0; i < CAM_FRAMES; i++) {
        tex_cam[i] = graphics_load_texture(paths_cam[i]);
    }

    tex_rooms[CAM_1A] = graphics_load_texture(IMG_SHOW_STAGE_1);
    tex_rooms[CAM_1B] = graphics_load_texture(IMG_DINNING_AREA_1);
    tex_rooms[CAM_1C] = graphics_load_texture(IMG_PIRATE_COVE_1);
    tex_rooms[CAM_2A] = graphics_load_texture(IMG_WEST_HALL_1);
    tex_rooms[CAM_2B] = graphics_load_texture(IMG_WEST_HALL_CORNER_1);
    tex_rooms[CAM_3]  = graphics_load_texture(IMG_SUPPLY_CLOSET_1);
    tex_rooms[CAM_4A] = graphics_load_texture(IMG_EAST_HALL_1);
    tex_rooms[CAM_4B] = graphics_load_texture(IMG_EAST_HALL_CORNER_1);
    tex_rooms[CAM_5]  = graphics_load_texture(IMG_BACKSTAGE_1);
    tex_rooms[CAM_6]  = NULL; // La cocina solo tiene audio
    tex_rooms[CAM_7]  = graphics_load_texture(IMG_RESTROOMS_1);

    sfx_cam_up = audio_load_sfx("romfs:/sfx/CAMERA_VIDEO_LOA_60105303.wav");
    sfx_cam_down = audio_load_sfx("romfs:/sfx/put_down.wav");

    cam_open = false;
    cam_frame = 0.0f;
    current_cam = CAM_1A;
    cam_pan_x = 0.0f;
    cam_pan_dir = 1;
    cam_pause_timer = 0; 
}

void camera_system_update(void) {
    // Animación física de la tableta
    if (cam_open) {
        if (cam_frame < CAM_FRAMES - 1) cam_frame += CAM_ANIM_SPEED;
    } else {
        if (cam_frame > 0.0f) cam_frame -= CAM_ANIM_SPEED;
    }

    // Paneo 
    if (cam_open && cam_frame >= (CAM_FRAMES - 1)) {
        if (cam_pan_dir == 1) {
            cam_pan_x += CAM_PAN_SPEED; 
            if (cam_pan_x >= 320.0f) { cam_pan_x = 320.0f; cam_pan_dir = 2; cam_pause_timer = 0; }
        }
        else if (cam_pan_dir == 2) {
            cam_pause_timer++;
            if (cam_pause_timer >= 100) cam_pan_dir = -1; 
        }
        else if (cam_pan_dir == -1) {
            cam_pan_x -= CAM_PAN_SPEED;
            if (cam_pan_x <= 0.0f) { cam_pan_x = 0.0f; cam_pan_dir = -2; cam_pause_timer = 0; }
        }
        else if (cam_pan_dir == -2) {
            cam_pause_timer++;
            if (cam_pause_timer >= 100) cam_pan_dir = 1; 
        }
    }
}

void camera_system_draw_room(void) {
    if (cam_open && cam_frame >= (CAM_FRAMES - 1)) {
        SDL_Renderer* renderer = graphics_get_renderer();
        if (current_cam == CAM_6) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect fullscreen = {0, 0, 1280, 720};
            SDL_RenderFillRect(renderer, &fullscreen);
        } else if (tex_rooms[current_cam]) {
            SDL_Rect src_rect = {(int)cam_pan_x, 0, 1280, 720};
            SDL_RenderCopy(renderer, tex_rooms[current_cam], &src_rect, NULL);
        }
    }
}

void camera_system_draw_button(void) {
    if (cam_frame <= 0.0f || cam_frame >= (CAM_FRAMES - 1)) {
        if (tex_button_cam) {
            SDL_Rect dst_cam = {340, 630, 600, 60};
            SDL_RenderCopy(graphics_get_renderer(), tex_button_cam, NULL, &dst_cam);
        }
    }
}

void camera_system_draw_animation(void) {
    if (cam_frame > 0.0f && cam_frame < (CAM_FRAMES - 1)) {
        int current_cam_anim = (int)cam_frame; 
        if (tex_cam[current_cam_anim]) {
            SDL_Rect dst_cam_anim = {0, 0, 1280, 720}; 
            SDL_RenderCopy(graphics_get_renderer(), tex_cam[current_cam_anim], NULL, &dst_cam_anim);
        }
    }
}

void camera_system_cleanup(void) {
    if (tex_button_cam) { SDL_DestroyTexture(tex_button_cam); tex_button_cam = NULL; }
    for (int i = 0; i < CAM_FRAMES; i++) {
        if (tex_cam[i]) { SDL_DestroyTexture(tex_cam[i]); tex_cam[i] = NULL; }
    }
    for (int i = 0; i < 11; i++) {
        if (tex_rooms[i]) { SDL_DestroyTexture(tex_rooms[i]); tex_rooms[i] = NULL; }
    }
    if (sfx_cam_up) { audio_free_sfx(sfx_cam_up); sfx_cam_up = NULL; }
    if (sfx_cam_down) { audio_free_sfx(sfx_cam_down); sfx_cam_down = NULL; }
}

void camera_system_toggle(void) {
    if (cam_frame <= 0.0f || cam_frame >= (CAM_FRAMES - 1)) {
        cam_open = !cam_open;
        if (cam_open) audio_play_sfx_chunk(sfx_cam_up);
        else audio_play_sfx_chunk(sfx_cam_down);
    }
}

void camera_system_force_close(void) {
    cam_open = false;
}

bool camera_system_is_open(void) {
    return cam_open;
}

float camera_system_get_frame(void) {
    return cam_frame;
}