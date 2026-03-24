#include "game/camera_system.h"
#include "game/assets.h"
#include "engine/graphics.h"
#include "game/animatronics.h"
#include "engine/audio.h"
#include "engine/input.h"
#include <SDL2/SDL.h>
#include <stdlib.h> 

static const float CAM_ANIM_SPEED = 0.5f;
static float cam_frame = 0.0f;
static bool cam_open = false;
extern int current_night;

static SDL_Texture* tex_cam[CAM_FRAMES] = {NULL};
static SDL_Texture* tex_button_cam = NULL; 
static SDL_Texture* tex_rooms[11] = {NULL}; 
static SDL_Texture* tex_rooms_bonnie[11] = {NULL}; 
static SDL_Texture* tex_stage_no_bonnie = NULL;    
static SDL_Texture* tex_west_hall_dark = NULL;
static SDL_Texture* tex_dining_bonnie_close = NULL;
static SDL_Texture* tex_backstage_bonnie_close = NULL;
static SDL_Texture* tex_bonnie_twitch_1 = NULL;
static SDL_Texture* tex_bonnie_twitch_2 = NULL;

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
    {832, 292, 305, 26}, {832, 292, 284, 26}, {832, 292, 192, 26}, {832, 292, 305, 26}, 
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
static bool is_backstage_bonnie_close = false; // Variable del Easter Egg

static Mix_Chunk* sfx_cam_up = NULL;
static Mix_Chunk* sfx_cam_down = NULL;
static Mix_Chunk* sfx_cam_blip = NULL;      
static Mix_Chunk* sfx_cam_static = NULL;    
static int channel_cam_static = -1;
static Mix_Chunk* sfx_garble[3] = {NULL};

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

    tex_rooms_bonnie[CAM_1A] = graphics_load_texture(IMG_SHOW_STAGE_1); 
    tex_rooms_bonnie[CAM_1B] = graphics_load_texture(IMG_DINNING_AREA_2); 
    tex_rooms_bonnie[CAM_2A] = graphics_load_texture(IMG_WEST_HALL_3); 
    tex_rooms_bonnie[CAM_2B] = graphics_load_texture(IMG_WEST_HALL_CORNER_2); 
    tex_rooms_bonnie[CAM_3]  = graphics_load_texture(IMG_SUPPLY_CLOSET_2); 
    tex_rooms_bonnie[CAM_5]  = graphics_load_texture(IMG_BACKSTAGE_2);

    tex_map[0] = graphics_load_texture(IMG_MAP_1);
    tex_map[1] = graphics_load_texture(IMG_MAP_2);
    tex_cam_border = graphics_load_texture(IMG_CAM_BORDER);
    tex_rec = graphics_load_texture(IMG_REC);
    tex_btn_normal = graphics_load_texture(IMG_CAM_1);
    tex_btn_selected = graphics_load_texture(IMG_CAM_2);
    tex_stage_no_bonnie = graphics_load_texture(IMG_SHOW_STAGE_2); 
    tex_west_hall_dark  = graphics_load_texture(IMG_WEST_HALL_2);
    tex_dining_bonnie_close = graphics_load_texture(IMG_DINNING_AREA_3);
    tex_backstage_bonnie_close = graphics_load_texture(IMG_BACKSTAGE_3);
    tex_bonnie_twitch_1 = graphics_load_texture(IMG_WEST_HALL_CORNER_3); 
    tex_bonnie_twitch_2 = graphics_load_texture(IMG_WEST_HALL_CORNER_4);

    const char* paths_txt[11] = { IMG_1A, IMG_1B, IMG_1C, IMG_2A, IMG_2B, IMG_3, IMG_4A, IMG_4B, IMG_5, IMG_6, IMG_7 };
    for (int i = 0; i < 11; i++) tex_cam_txt[i] = graphics_load_texture(paths_txt[i]);

    const char* paths_names[11] = { IMG_SHOW_STAGE_TEXT, IMG_DINING_AREA_TEXT, IMG_PIRATE_COVE_TEXT, IMG_W_HALL_TEXT, IMG_W_HALL_CORNER_TEXT, IMG_SUPPLY_CLOSET_TEXT, IMG_E_HALL_TEXT, IMG_E_HALL_CORNER_TEXT, IMG_BACKSTAGE_TEXT, IMG_KITCHEN_TEXT, IMG_RESTROOMS_TEXT };
    for (int i = 0; i < 11; i++) tex_room_names[i] = graphics_load_texture(paths_names[i]);

    tex_kitchen_sound = graphics_load_texture(IMG_KITCHEN_SOUND_TEXT);

    sfx_cam_up = audio_load_sfx("romfs:/sfx/CAMERA_VIDEO_LOA_60105303.wav");
    sfx_cam_down = audio_load_sfx("romfs:/sfx/put_down.wav");

    sfx_cam_blip = audio_load_sfx("romfs:/sfx/blip3.wav"); 
    sfx_cam_static = audio_load_sfx("romfs:/sfx/MiniDV_Tape_Eject_1.wav");

    sfx_garble[0] = audio_load_sfx("romfs:/sfx/garble1.wav");
    sfx_garble[1] = audio_load_sfx("romfs:/sfx/garble2.wav");
    sfx_garble[2] = audio_load_sfx("romfs:/sfx/garble3.wav");

    if (sfx_cam_static) {
        channel_cam_static = audio_play_sfx_loop_chunk(sfx_cam_static);
        audio_set_channel_volume(channel_cam_static, 0); 
    }

    cam_open = false; cam_frame = 0.0f; static_frame = 0.0f; current_cam = CAM_1A;
    cam_pan_x = 0.0f; cam_pan_dir = 1; cam_pause_timer = 0; 
    map_anim_frame = 0.0f; rec_blink_frame = 0.0f;
    map_frame = 0; rec_visible = true; is_blipping = false; blip_anim_frame = 0.0f;
    is_backstage_bonnie_close = false;
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
                case CAM_5:  next_cam = CAM_1A; break; 
                case CAM_1B: next_cam = CAM_1A; break; 
                case CAM_7:  next_cam = CAM_1A; break; 
                case CAM_1C: next_cam = CAM_1B; break; 
                case CAM_2A: next_cam = CAM_1C; break; 
                case CAM_3:  next_cam = CAM_1C; break; 
                case CAM_2B: next_cam = CAM_2A; break; 
                case CAM_6:  next_cam = CAM_7;  break; 
                case CAM_4A: next_cam = CAM_1C; break; 
                case CAM_4B: next_cam = CAM_4A; break; 
                default: break;
            }
        }
        else if (input_get_button_down(HidNpadButton_Down)) {
            switch(current_cam) {
                case CAM_1A: next_cam = CAM_1B; break; 
                case CAM_5:  next_cam = CAM_1C; break; 
                case CAM_1B: next_cam = CAM_1C; break; 
                case CAM_7:  next_cam = CAM_6;  break; 
                case CAM_1C: next_cam = CAM_2A; break; 
                case CAM_2A: next_cam = CAM_2B; break; 
                case CAM_4A: next_cam = CAM_4B; break; 
                case CAM_3:  next_cam = CAM_2A; break; 
                case CAM_6:  next_cam = CAM_4A; break; 
                default: break;
            }
        }
        else if (input_get_button_down(HidNpadButton_Left)) {
            switch(current_cam) {
                case CAM_1A: next_cam = CAM_5;  break; 
                case CAM_1B: next_cam = CAM_5;  break; 
                case CAM_7:  next_cam = CAM_1B; break; 
                case CAM_1C: next_cam = CAM_5;  break; 
                case CAM_2A: next_cam = CAM_3;  break; 
                case CAM_2B: next_cam = CAM_3;  break; 
                case CAM_6:  next_cam = CAM_4A; break; 
                case CAM_4A: next_cam = CAM_2A; break; 
                case CAM_4B: next_cam = CAM_2B; break; 
                default: break;
            }
        }
        else if (input_get_button_down(HidNpadButton_Right)) {
            switch(current_cam) {
                case CAM_1A: next_cam = CAM_7;  break; 
                case CAM_5:  next_cam = CAM_1B; break; 
                case CAM_1B: next_cam = CAM_7;  break; 
                case CAM_1C: next_cam = CAM_7;  break; 
                case CAM_3:  next_cam = CAM_2A; break; 
                case CAM_2A: next_cam = CAM_4A; break; 
                case CAM_2B: next_cam = CAM_4B; break; 
                case CAM_4A: next_cam = CAM_6;  break; 
                case CAM_4B: next_cam = CAM_6;  break; 
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
            
            // --- PUNTO 3: Bajamos la variable aquí para saber dónde está Bonnie ---
            int bonnie_room = animatronics_get_bonnie_room();
            // ----------------------------------------------------------------------

            // --- RELOJ INTERNO DE 50ms (ESTILO CLICKTEAM FUSION) ---
            static Uint64 last_fx_time = 0;
            static int current_twitch_frame = 0;

            Uint64 current_time = SDL_GetTicks64();
            if (current_time - last_fx_time >= 50) { 
                // Cada 50ms exactos tiramos los dados
                current_twitch_frame = rand() % 3;     // Dado de 3 caras para los espasmos
                
                // --- PUNTO 3: AUDIO DE ESPASMOS ---
                // Si Bonnie está en la esquina (2B), es noche 4+ y le estamos mirando...
                if (current_cam == CAM_2B && bonnie_room == CAM_2B && current_night >= 4) {
                    // Tiramos un dado de 10% de probabilidad CADA 50 MILISEGUNDOS
                    if (rand() % 100 < 10) {
                        int r_snd = rand() % 3; // Elegimos garble1, garble2 o garble3
                        if (sfx_garble[r_snd]) {
                            // Ponemos volumen moderado para no reventar los oídos
                            audio_set_sfx_volume(sfx_garble[r_snd], 60); 
                            audio_play_sfx_chunk(sfx_garble[r_snd]);
                        }
                    }
                }
                // ----------------------------------

                last_fx_time = current_time;
            }
            // --------------------------------------------------------

            // --- NUEVA LÓGICA DE DIBUJADO DE HABITACIONES ---
            SDL_Texture* bg_to_draw = tex_rooms[current_cam]; // Por defecto, vacía

            // 1. Manejo especial del Show Stage (1A)
            if (current_cam == CAM_1A) {
                if (bonnie_room == CAM_1A) {
                    bg_to_draw = tex_rooms_bonnie[CAM_1A]; // Todos en el escenario
                } else {
                    bg_to_draw = tex_stage_no_bonnie; // Bonnie se ha ido
                }
            }
            // 2. Si Bonnie está en la cámara que estamos mirando
            else if (bonnie_room == current_cam && tex_rooms_bonnie[current_cam] != NULL) {
                
                int bonnie_pose = animatronics_get_bonnie_pose();

                // Lógica de dos poses fijas para Dining Area (1B)
                if (current_cam == CAM_1B) {
                    bg_to_draw = (bonnie_pose == 1) ? tex_dining_bonnie_close : tex_rooms_bonnie[CAM_1B];
                }
                // Lógica de dos poses fijas para Backstage (5) - DADO AL ABRIR CÁMARA
                else if (current_cam == CAM_5) {
                    bg_to_draw = (is_backstage_bonnie_close) ? tex_backstage_bonnie_close : tex_rooms_bonnie[CAM_5];
                }
                // Lógica de ESPASMOS en West Hall Corner (2B) - Solo Noche 4+
                else if (current_cam == CAM_2B) {
                    if (current_night >= 4) {
                        if (current_twitch_frame == 0) bg_to_draw = tex_rooms_bonnie[CAM_2B];
                        else if (current_twitch_frame == 1) bg_to_draw = tex_bonnie_twitch_1;
                        else bg_to_draw = tex_bonnie_twitch_2;
                    } else {
                        // Noches 1, 2 y 3: Se queda quieto
                        bg_to_draw = tex_rooms_bonnie[CAM_2B];
                    }
                }
                // Para CAM_3 (Armario) y CAM_2A (Pasillo) dibujamos la foto normal de Bonnie
                else {
                    bg_to_draw = tex_rooms_bonnie[current_cam];
                }
            }

            // 3. Efecto de parpadeo en el pasillo izquierdo (CAM 2A)
            if (current_cam == CAM_2A) {
                if (rand() % 100 < 70) {
                    bg_to_draw = tex_west_hall_dark; 
                }
            }

            // 4. Dibujar el fondo elegido con su paneo correspondiente
            if (bg_to_draw) {
                SDL_Rect src_rect = {(int)cam_pan_x, 0, 1280, 720};
                SDL_RenderCopy(renderer, bg_to_draw, &src_rect, NULL);
            }
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

        // --- NUEVO: RASTREADOR GPS DE BONNIE ---
        int bonnie_room = animatronics_get_bonnie_room();
        
        // Nos aseguramos de que esté en una cámara normal (0 a 10) y no en la puerta u oficina
        if (bonnie_room >= 0 && bonnie_room < 11) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            
            // Color Índigo (R: 100, G: 0, B: 255) con transparencia (Alpha: 150)
            SDL_SetRenderDrawColor(renderer, 100, 0, 255, 150); 
            
            // Dibujamos el rectángulo encima del botón de la cámara donde está Bonnie
            SDL_RenderFillRect(renderer, &btn_rects[bonnie_room]);
            
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }
        // ---------------------------------------

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
    
    for (int i = 0; i < CAM_FRAMES; i++) {
        if (tex_cam[i]) SDL_DestroyTexture(tex_cam[i]);
    }
    
    for (int i = 0; i < 8; i++) {
        if (tex_static[i]) SDL_DestroyTexture(tex_static[i]);
    }
    
    for (int i = 0; i < 11; i++) {
        if (tex_rooms[i]) SDL_DestroyTexture(tex_rooms[i]);
        if (tex_rooms_bonnie[i]) SDL_DestroyTexture(tex_rooms_bonnie[i]); 
        if (tex_cam_txt[i]) SDL_DestroyTexture(tex_cam_txt[i]);
        if (tex_room_names[i]) SDL_DestroyTexture(tex_room_names[i]);
    }

    if (tex_map[0]) SDL_DestroyTexture(tex_map[0]);
    if (tex_map[1]) SDL_DestroyTexture(tex_map[1]);
    if (tex_cam_border) SDL_DestroyTexture(tex_cam_border);
    if (tex_rec) SDL_DestroyTexture(tex_rec);
    if (tex_btn_normal) SDL_DestroyTexture(tex_btn_normal);
    if (tex_btn_selected) SDL_DestroyTexture(tex_btn_selected);

    // Texturas especiales de Bonnie y las salas
    if (tex_stage_no_bonnie) SDL_DestroyTexture(tex_stage_no_bonnie);
    if (tex_west_hall_dark) SDL_DestroyTexture(tex_west_hall_dark);
    if (tex_dining_bonnie_close) SDL_DestroyTexture(tex_dining_bonnie_close);
    if (tex_backstage_bonnie_close) SDL_DestroyTexture(tex_backstage_bonnie_close);
    if (tex_bonnie_twitch_1) SDL_DestroyTexture(tex_bonnie_twitch_1);
    if (tex_bonnie_twitch_2) SDL_DestroyTexture(tex_bonnie_twitch_2);

    // Audio 
    if (sfx_cam_up) audio_free_sfx(sfx_cam_up);
    if (sfx_cam_down) audio_free_sfx(sfx_cam_down);
    if (sfx_cam_static) {
        audio_stop_channel(channel_cam_static);
        audio_free_sfx(sfx_cam_static);
    }
    if (sfx_cam_blip) audio_free_sfx(sfx_cam_blip);

    // --- PUNTO 4: Limpiar sonidos de espasmo ---
    for (int i = 0; i < 3; i++) {
        if (sfx_garble[i]) {
            audio_free_sfx(sfx_garble[i]);
            sfx_garble[i] = NULL;
        }
    }
    // -------------------------------------------
}

void camera_system_toggle(void) {
    if (cam_frame <= 0.0f || cam_frame >= (CAM_FRAMES - 1)) {
        cam_open = !cam_open;
        if (cam_open) {
            audio_play_sfx_chunk(sfx_cam_up);
            is_blipping = true; blip_anim_frame = 0.0f;
            audio_set_channel_volume(channel_cam_static, 100);

            // --- NUEVO: DADO DE PANTALLA RARA (10%) ---
            is_backstage_bonnie_close = (rand() % 100 < 10);
            // ------------------------------------------

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