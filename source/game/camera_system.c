#include "game/camera_system.h"
#include "game/assets.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include "engine/input.h"
#include <SDL2/SDL.h>

static const float CAM_ANIM_SPEED = 0.5f;
static float cam_frame = 0.0f;
static bool cam_open = false;

static SDL_Texture* tex_cam[CAM_FRAMES] = {NULL};
static SDL_Texture* tex_button_cam = NULL; 
static SDL_Texture* tex_rooms[11] = {NULL}; 

// --- ESTÁTICA Y BLIP ---
static SDL_Texture* tex_static[8] = {NULL};
static float static_frame = 0.0f;

static SDL_Texture* tex_blip[9] = {NULL};
static bool is_blipping = false;
static float blip_anim_frame = 0.0f;

// --- GRÁFICOS DE LA UI ---
static SDL_Texture* tex_map[2] = {NULL};
static SDL_Texture* tex_cam_border = NULL;
static SDL_Texture* tex_rec = NULL;
static SDL_Texture* tex_btn_normal = NULL;
static SDL_Texture* tex_btn_selected = NULL;

static SDL_Texture* tex_cam_txt[11] = {NULL};
static SDL_Texture* tex_room_names[11] = {NULL};
static SDL_Texture* tex_kitchen_sound = NULL;

// Coordenadas exactas de los 11 botones en el mapa
static const SDL_Rect btn_rects[11] = {
    {954, 334, 60, 40}, {934, 390, 60, 40}, {902, 468, 60, 40}, {954, 584, 60, 40}, 
    {954, 624, 60, 40}, {870, 566, 60, 40}, {1060, 585, 60, 40}, {1060, 625, 60, 40}, 
    {828, 417, 60, 40}, {1157, 549, 60, 40}, {1166, 418, 60, 40}
};

// Coordenadas de los textos
static const SDL_Rect txt_rects[11] = {
    {962, 341, 31, 25}, {940, 397, 31, 25}, {909, 475, 31, 25}, {961, 590, 31, 25}, 
    {961, 630, 31, 25}, {878, 574, 31, 25}, {1067, 592, 31, 25}, {1067, 632, 31, 25}, 
    {835, 424, 31, 25}, {1164, 556, 31, 25}, {1173, 424, 31, 25}
};

// Nombres de las salas
static const SDL_Rect room_name_rects[11] = {
    {832, 292, 217, 26}, {832, 292, 239, 26}, {832, 292, 228, 24}, {832, 292, 192, 26}, 
    {832, 292, 305, 26}, {832, 292, 195, 26}, {832, 292, 192, 26}, {832, 292, 305, 26}, 
    {832, 292, 195, 26}, {832, 292, 151, 26}, {832, 292, 196, 26}
};

static CameraID current_cam = CAM_1A; 
static float cam_pan_x = 0.0f;
static int cam_pan_dir = 1; 
static int cam_pause_timer = 0; 
static const float CAM_PAN_SPEED = 0.8f; 

// --- ANIMACIÓN UI (VELOCIDAD 2 = 0.02f) ---
static float map_anim_frame = 0.0f;
static float rec_blink_frame = 0.0f;
static const float UI_ANIM_SPEED = 0.02f;
static int map_frame = 0;
static bool rec_visible = true;

static Mix_Chunk* sfx_cam_up = NULL;
static Mix_Chunk* sfx_cam_down = NULL;
static Mix_Chunk* sfx_cam_blip = NULL;      
static Mix_Chunk* sfx_cam_static = NULL;    
static int channel_cam_static = -1;

void camera_system_init(void) {
    tex_button_cam = graphics_load_texture(IMG_BUTTON_CAM);
    const char* paths_cam[CAM_FRAMES] = {
        IMG_CAM_OPEN_1, IMG_CAM_OPEN_2, IMG_CAM_OPEN_3, IMG_CAM_OPEN_4, IMG_CAM_OPEN_5,
        IMG_CAM_OPEN_6, IMG_CAM_OPEN_7, IMG_CAM_OPEN_8, IMG_CAM_OPEN_9, IMG_CAM_OPEN_10, IMG_CAM_OPEN_11
    };
    for (int i = 0; i < CAM_FRAMES; i++) tex_cam[i] = graphics_load_texture(paths_cam[i]);

    const char* paths_static[8] = { IMG_STATIC_1, IMG_STATIC_2, IMG_STATIC_3, IMG_STATIC_4, IMG_STATIC_5, IMG_STATIC_6, IMG_STATIC_7, IMG_STATIC_8 };
    for (int i = 0; i < 8; i++) {
        tex_static[i] = graphics_load_texture(paths_static[i]);
        if (tex_static[i]) {
            SDL_SetTextureBlendMode(tex_static[i], SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(tex_static[i], 55); 
        }
    }

    const char* paths_blip[9] = { IMG_BLIP_FLASH_3, IMG_BLIP_FLASH_4, IMG_BLIP_FLASH_5, IMG_BLIP_FLASH_6, IMG_BLIP_FLASH_7, IMG_BLIP_FLASH_8, IMG_BLIP_FLASH_9, IMG_BLIP_FLASH_10, IMG_BLIP_FLASH_11 };
    for (int i = 0; i < 9; i++) tex_blip[i] = graphics_load_texture(paths_blip[i]);

    tex_rooms[CAM_1A] = graphics_load_texture(IMG_SHOW_STAGE_1);
    tex_rooms[CAM_1B] = graphics_load_texture(IMG_DINNING_AREA_1);
    tex_rooms[CAM_1C] = graphics_load_texture(IMG_PIRATE_COVE_1);
    tex_rooms[CAM_2A] = graphics_load_texture(IMG_WEST_HALL_1);
    tex_rooms[CAM_2B] = graphics_load_texture(IMG_WEST_HALL_CORNER_1);
    tex_rooms[CAM_3]  = graphics_load_texture(IMG_SUPPLY_CLOSET_1);
    tex_rooms[CAM_4A] = graphics_load_texture(IMG_EAST_HALL_1);
    tex_rooms[CAM_4B] = graphics_load_texture(IMG_EAST_HALL_CORNER_1);
    tex_rooms[CAM_5]  = graphics_load_texture(IMG_BACKSTAGE_1);
    tex_rooms[CAM_6]  = NULL; 
    tex_rooms[CAM_7]  = graphics_load_texture(IMG_RESTROOMS_1);

    tex_map[0] = graphics_load_texture(IMG_MAP_1);
    tex_map[1] = graphics_load_texture(IMG_MAP_2);
    tex_cam_border = graphics_load_texture(IMG_CAM_BORDER);
    tex_rec = graphics_load_texture(IMG_REC);
    tex_btn_normal = graphics_load_texture(IMG_CAM_1);
    tex_btn_selected = graphics_load_texture(IMG_CAM_2);

    const char* paths_txt[11] = { IMG_1A, IMG_1B, IMG_1C, IMG_2A, IMG_2B, IMG_3, IMG_4A, IMG_4B, IMG_5, IMG_6, IMG_7 };
    for (int i = 0; i < 11; i++) tex_cam_txt[i] = graphics_load_texture(paths_txt[i]);

    const char* paths_names[11] = { IMG_SHOW_STAGE_TEXT, IMG_DINING_AREA_TEXT, IMG_PIRATE_COVE_TEXT, IMG_W_HALL_TEXT, IMG_W_HALL_CORNER_TEXT, IMG_SUPPLY_CLOSET_TEXT, IMG_E_HALL_TEXT, IMG_E_HALL_CORNER_TEXT, IMG_BACKSTAGE_TEXT, IMG_KITCHEN_TEXT, IMG_RESTROOMS_TEXT };
    for (int i = 0; i < 11; i++) tex_room_names[i] = graphics_load_texture(paths_names[i]);

    tex_kitchen_sound = graphics_load_texture(IMG_KITCHEN_SOUND_TEXT);

    sfx_cam_up = audio_load_sfx("romfs:/sfx/CAMERA_VIDEO_LOA_60105303.wav");
    sfx_cam_down = audio_load_sfx("romfs:/sfx/put_down.wav");

    sfx_cam_blip = audio_load_sfx("romfs:/sfx/blip3.wav"); 
    sfx_cam_static = audio_load_sfx("romfs:/sfx/MiniDV_Tape_Eject_1.wav");

    if (sfx_cam_static) {
        channel_cam_static = audio_play_sfx_loop_chunk(sfx_cam_static);
        audio_set_channel_volume(channel_cam_static, 0); // Empieza a volumen 0
    }

    cam_open = false; cam_frame = 0.0f; static_frame = 0.0f; current_cam = CAM_1A;
    cam_pan_x = 0.0f; cam_pan_dir = 1; cam_pause_timer = 0; 
    map_anim_frame = 0.0f; rec_blink_frame = 0.0f;
    map_frame = 0; rec_visible = true; is_blipping = false; blip_anim_frame = 0.0f;
}

void camera_system_update(void) {
    if (cam_open) {
        if (cam_frame < CAM_FRAMES - 1) cam_frame += CAM_ANIM_SPEED;
    } else {
        if (cam_frame > 0.0f) cam_frame -= CAM_ANIM_SPEED;
    }

    if (cam_open && cam_frame >= (CAM_FRAMES - 1)) {
        CameraID next_cam = current_cam;
        if (input_get_button_down(HidNpadButton_Up)) {
            switch(current_cam) {
                case CAM_5:  next_cam = CAM_1A; break; // 5 -> 1A
                case CAM_1B: next_cam = CAM_1A; break; // 1B -> 1A
                case CAM_7:  next_cam = CAM_1A; break; // 7 -> 1A
                case CAM_1C: next_cam = CAM_1B; break; // 1C -> 1B
                case CAM_2A: next_cam = CAM_1C; break; // 2A -> 1C
                case CAM_3:  next_cam = CAM_1C; break; // 3 -> 1C
                case CAM_2B: next_cam = CAM_2A; break; // 2B -> 2A
                case CAM_6:  next_cam = CAM_7;  break; // 6 -> 7
                case CAM_4A: next_cam = CAM_1C; break; // 4A -> 1C
                case CAM_4B: next_cam = CAM_4A; break; // 4B -> 4A
                default: break;
            }
        }
        else if (input_get_button_down(HidNpadButton_Down)) {
            switch(current_cam) {
                case CAM_1A: next_cam = CAM_1B; break; // 1A -> 1B
                case CAM_5:  next_cam = CAM_1C; break; // 5 -> 1C
                case CAM_1B: next_cam = CAM_1C; break; // 1B -> 1C
                case CAM_7:  next_cam = CAM_6;  break; // 7 -> 6
                case CAM_1C: next_cam = CAM_2A; break; // 1C -> 2A
                case CAM_2A: next_cam = CAM_2B; break; // 2A -> 2B
                case CAM_4A: next_cam = CAM_4B; break; // 4A -> 4B
                case CAM_3:  next_cam = CAM_2A; break; // 3 -> 2A
                case CAM_6:  next_cam = CAM_4A; break; // 6 -> 4A
                default: break;
            }
        }
        else if (input_get_button_down(HidNpadButton_Left)) {
            switch(current_cam) {
                case CAM_1A: next_cam = CAM_5;  break; // 1A -> 5
                case CAM_1B: next_cam = CAM_5;  break; // 1B -> 5
                case CAM_7:  next_cam = CAM_1B; break; // 7 -> 1B
                case CAM_1C: next_cam = CAM_5;  break; // 1C -> 5
                case CAM_2A: next_cam = CAM_3;  break; // 2A -> 3
                case CAM_2B: next_cam = CAM_3;  break; // 2B -> 3
                case CAM_6:  next_cam = CAM_4A; break; // 6 -> 4A
                case CAM_4A: next_cam = CAM_2A; break; // 4A -> 2A
                case CAM_4B: next_cam = CAM_2B; break; // 4B -> 2B
                default: break;
            }
        }
        else if (input_get_button_down(HidNpadButton_Right)) {
            switch(current_cam) {
                case CAM_1A: next_cam = CAM_7;  break; // 1A -> 7
                case CAM_5:  next_cam = CAM_1B; break; // 5 -> 1B
                case CAM_1B: next_cam = CAM_7;  break; // 1B -> 7
                case CAM_1C: next_cam = CAM_7;  break; // 1C -> 7
                case CAM_3:  next_cam = CAM_2A; break; // 3 -> 2A
                case CAM_2A: next_cam = CAM_4A; break; // 2A -> 4A
                case CAM_2B: next_cam = CAM_4B; break; // 2B -> 4B
                case CAM_4A: next_cam = CAM_6;  break; // 4A -> 6
                case CAM_4B: next_cam = CAM_6;  break; // 4B -> 6
                default: break;
            }
        }

        if (next_cam != current_cam) {
            current_cam = next_cam;
            is_blipping = true;
            blip_anim_frame = 0.0f;
            audio_play_sfx_chunk(sfx_cam_blip);
        }

        if (is_blipping) {
            blip_anim_frame += 0.70f; 
            if (blip_anim_frame >= 9.0f) { is_blipping = false; blip_anim_frame = 0.0f; }
        }

        static_frame += 1.0f; 
        if (static_frame >= 8.0f) static_frame = 0.0f;

        // --- ANIMACIÓN UI A VELOCIDAD 2 (0.02f) ---
        map_anim_frame += UI_ANIM_SPEED;
        if (map_anim_frame >= 2.0f) map_anim_frame -= 2.0f;
        map_frame = (int)map_anim_frame;

        rec_blink_frame += UI_ANIM_SPEED;
        if (rec_blink_frame >= 2.0f) rec_blink_frame -= 2.0f;
        rec_visible = ((int)rec_blink_frame == 0);

        // --- PANEO ---
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
        
        // 1. Lógica de fondo de habitación
        if (current_cam == CAM_6 || tex_rooms[current_cam] == NULL) {
            // Fondo negro para la cocina o si falla la textura
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect fullscreen = {0, 0, 1280, 720};
            SDL_RenderFillRect(renderer, &fullscreen);

            // DIBUJAR TEXTO "SOUND ONLY" (X: 384 + 80 = 464)
            if (current_cam == CAM_6 && tex_kitchen_sound) {
                SDL_Rect dst_sound = {464, 69, 371, 54};
                SDL_RenderCopy(renderer, tex_kitchen_sound, NULL, &dst_sound);
            }
        } else {
            // Dibujar fondo normal con paneo
            SDL_Rect src_rect = {(int)cam_pan_x, 0, 1280, 720};
            SDL_RenderCopy(renderer, tex_rooms[current_cam], &src_rect, NULL);
        }

        // 2. Dibujar Estática por encima (Declaramos s_idx UNA SOLA VEZ aquí)
        int s_idx = (int)static_frame;
        if (tex_static[s_idx]) {
            SDL_RenderCopy(renderer, tex_static[s_idx], NULL, NULL);
        }

        // 3. Dibujar el BLIP FLASH
        if (is_blipping) {
            int b_idx = (int)blip_anim_frame;
            if (b_idx >= 0 && b_idx < 9 && tex_blip[b_idx]) {
                SDL_RenderCopy(renderer, tex_blip[b_idx], NULL, NULL);
            }
        }
    }
}

void camera_system_draw_ui(void) {
    if (cam_open && cam_frame >= (CAM_FRAMES - 1)) {
        SDL_Renderer* renderer = graphics_get_renderer();
        if (tex_cam_border) SDL_RenderCopy(renderer, tex_cam_border, NULL, NULL);
        if (tex_rec && rec_visible) {
            SDL_Rect dst_rec = {68, 52, 50, 50};
            SDL_RenderCopy(renderer, tex_rec, NULL, &dst_rec);
        }
        if (tex_map[map_frame]) {
            SDL_Rect dst_map = {848, 313, 400, 400};
            SDL_RenderCopy(renderer, tex_map[map_frame], NULL, &dst_map);
        }
        for (int i = 0; i < 11; i++) {
            SDL_Texture* btn_tex = (current_cam == i) ? tex_btn_selected : tex_btn_normal;
            if (btn_tex) SDL_RenderCopy(renderer, btn_tex, NULL, &btn_rects[i]);
            if (tex_cam_txt[i]) SDL_RenderCopy(renderer, tex_cam_txt[i], NULL, &txt_rects[i]);
        }
        if (tex_room_names[current_cam]) {
            SDL_RenderCopy(renderer, tex_room_names[current_cam], NULL, &room_name_rects[current_cam]);
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
        int anim_idx = (int)cam_frame; 
        if (tex_cam[anim_idx]) {
            SDL_Rect dst = {0, 0, 1280, 720}; 
            SDL_RenderCopy(graphics_get_renderer(), tex_cam[anim_idx], NULL, &dst);
        }
    }
}

void camera_system_cleanup(void) {
    if (tex_button_cam) SDL_DestroyTexture(tex_button_cam);
    if (tex_kitchen_sound) SDL_DestroyTexture(tex_kitchen_sound);
    for (int i = 0; i < CAM_FRAMES; i++) if (tex_cam[i]) SDL_DestroyTexture(tex_cam[i]);
    for (int i = 0; i < 8; i++) if (tex_static[i]) SDL_DestroyTexture(tex_static[i]);
    for (int i = 0; i < 11; i++) {
        if (tex_rooms[i]) SDL_DestroyTexture(tex_rooms[i]);
        if (tex_cam_txt[i]) SDL_DestroyTexture(tex_cam_txt[i]);
        if (tex_room_names[i]) SDL_DestroyTexture(tex_room_names[i]);
    }
    if (tex_map[0]) SDL_DestroyTexture(tex_map[0]);
    if (tex_map[1]) SDL_DestroyTexture(tex_map[1]);
    if (tex_cam_border) SDL_DestroyTexture(tex_cam_border);
    if (tex_rec) SDL_DestroyTexture(tex_rec);
    if (tex_btn_normal) SDL_DestroyTexture(tex_btn_normal);
    if (tex_btn_selected) SDL_DestroyTexture(tex_btn_selected);
    if (sfx_cam_up) audio_free_sfx(sfx_cam_up);
    if (sfx_cam_down) audio_free_sfx(sfx_cam_down);
    if (sfx_cam_up) audio_free_sfx(sfx_cam_up);
    if (sfx_cam_down) audio_free_sfx(sfx_cam_down);
    if (sfx_cam_static) {
        audio_stop_channel(channel_cam_static);
        audio_free_sfx(sfx_cam_static);
    }
    if (sfx_cam_blip) audio_free_sfx(sfx_cam_blip);
}

void camera_system_toggle(void) {
    if (cam_frame <= 0.0f || cam_frame >= (CAM_FRAMES - 1)) {
        cam_open = !cam_open;
        if (cam_open) {
            audio_play_sfx_chunk(sfx_cam_up);
            is_blipping = true; blip_anim_frame = 0.0f;
            audio_set_channel_volume(channel_cam_static, 100);
        } else {
            audio_play_sfx_chunk(sfx_cam_down);
            audio_set_channel_volume(channel_cam_static, 0);
        }
    }
}

void camera_system_force_close(void) { 
    cam_open = false; 
    audio_set_channel_volume(channel_cam_static, 0); 
}
bool camera_system_is_open(void) { return cam_open; }
float camera_system_get_frame(void) { return cam_frame; }