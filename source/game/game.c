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

// ── Oficina ───────────────────────────────────────────────────────────────────
static float camera_x = 160.0f;
static SDL_Texture* tex_office_normal        = NULL;
static SDL_Texture* tex_office_light_L       = NULL;
static SDL_Texture* tex_office_light_R       = NULL;
static SDL_Texture* tex_fan[3]               = {NULL};
static SDL_Texture* tex_office_blackout      = NULL;
static SDL_Texture* tex_office_blackout_freddy = NULL;
static SDL_Texture* tex_office_bonnie        = NULL;
static SDL_Texture* tex_office_chica         = NULL;
static int fan_frame = 0, fan_timer = 0;
#define FAN_ANIM_SPEED 1

// ── Puertas y Botones ─────────────────────────────────────────────────────────
#define DOOR_FRAMES 15
static const float DOOR_ANIM_SPEED = 0.75f;
static float door_L_frame = 0.0f, door_R_frame = 0.0f;
static bool left_door_on = false, right_door_on = false;
static bool left_light_on = false, right_light_on = false;
static SDL_Texture* tex_door_L_close[DOOR_FRAMES] = {NULL};
static SDL_Texture* tex_door_R_close[DOOR_FRAMES] = {NULL};
static SDL_Texture* tex_button_L[4] = {NULL};
static SDL_Texture* tex_button_R[4] = {NULL};

// ── Victoria ──────────────────────────────────────────────────────────────────
static bool is_winning = false;
static float win_fade = 0.0f;

// ── Alucinaciones ─────────────────────────────────────────────────────────────
static SDL_Texture* tex_hallucinations[4] = {NULL};
static int hallucination_timer = 0, hallucination_hold_timer = 0;
static int current_hallucination = -1;
static int random_sound_timer = 0;

// ── Audio ─────────────────────────────────────────────────────────────────────
static Mix_Chunk* sfx_fan         = NULL;
static Mix_Chunk* sfx_light       = NULL;
static Mix_Chunk* sfx_door        = NULL;
static Mix_Chunk* sfx_circus      = NULL;
static Mix_Chunk* sfx_pounding    = NULL;
static Mix_Chunk* sfx_window_scare = NULL;
static Mix_Chunk* sfx_error       = NULL;
static Mix_Chunk* sfx_steps       = NULL;
static Mix_Chunk* sfx_jumpscare   = NULL;
static Mix_Chunk* sfx_running_fast = NULL;
static Mix_Chunk* sfx_breath[4]   = {NULL};
static Mix_Chunk* sfx_hallucination[4] = {NULL};
static Mix_Chunk* sfx_kitchen[4]  = {NULL};
static Mix_Chunk* sfx_freddy_laugh[3] = {NULL};

static int channel_light_L = -1, channel_light_R = -1;
static int channel_fan = -1, channel_breath = -1;
static int channel_circus = -1, channel_kitchen = -1;

static int breath_timer = 0, kitchen_timer = 0;
static bool bonnie_scare_played = false, chica_scare_played = false;

// ── Jumpscares ────────────────────────────────────────────────────────────────
#define JUMPSCARE_BONNIE_FRAMES  11
#define JUMPSCARE_CHICA_FRAMES   16
#define JUMPSCARE_FREDDY_FRAMES  31
static const float JUMPSCARE_ANIM_SPEED = 0.75f;

static SDL_Texture* tex_bonnie_jumpscare[JUMPSCARE_BONNIE_FRAMES] = {NULL};
static SDL_Texture* tex_chica_jumpscare[JUMPSCARE_CHICA_FRAMES]   = {NULL};
static SDL_Texture* tex_freddy_jumpscare[JUMPSCARE_FREDDY_FRAMES] = {NULL};

static bool  is_bonnie_jumpscare = false, is_chica_jumpscare = false, is_freddy_jumpscare = false;
static float bonnie_jumpscare_frame = 0.0f, chica_jumpscare_frame = 0.0f, freddy_jumpscare_frame = 0.0f;
static int   jumpscare_duration_timer = 0;
static int   bonnie_force_down_timer = 0, chica_force_down_timer = 0;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Carga un array de texturas desde un array de rutas
static void load_textures(SDL_Texture** dst, const char** paths, int n) {
    for (int i = 0; i < n; i++) dst[i] = graphics_load_texture(paths[i]);
}

// Destruye y anula un array de texturas
static void destroy_textures(SDL_Texture** arr, int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i]) { SDL_DestroyTexture(arr[i]); arr[i] = NULL; }
    }
}

// Libera y anula un array de sfx
static void free_sfx_array(Mix_Chunk** arr, int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i]) { audio_free_sfx(arr[i]); arr[i] = NULL; }
    }
}

// Dispara el jumpscare: para audio, pone flag y reproduce el grito
static void trigger_jumpscare(bool* flag) {
    *flag = true;
    camera_system_force_close();
    audio_stop_all_sfx();
    audio_stop_music();
    if (sfx_jumpscare) {
        audio_set_sfx_volume(sfx_jumpscare, 100);
        audio_play_sfx_chunk(sfx_jumpscare);
    }
}

// ── Init ──────────────────────────────────────────────────────────────────────

void game_init(void) {
    Mix_AllocateChannels(32);
    hud_init();
    power_system_init();
    camera_system_init();
    animatronics_init(current_night);

    // Texturas de fondo y oficina
    tex_office_normal         = graphics_load_texture(IMG_OFFICE);
    tex_office_light_L        = graphics_load_texture(IMG_OFFICE_LIGHT_L);
    tex_office_light_R        = graphics_load_texture(IMG_OFFICE_LIGHT_R);
    tex_office_blackout       = graphics_load_texture(IMG_OFFICE_BLACK_OUT);
    tex_office_blackout_freddy= graphics_load_texture(IMG_OFFICE_BLACK_OUT_FREDDY);
    tex_office_bonnie         = graphics_load_texture(IMG_OFFICE_BONNIE);
    tex_office_chica          = graphics_load_texture(IMG_OFFICE_CHICA);
    load_textures(tex_fan, (const char*[]){IMG_FAN_1, IMG_FAN_2, IMG_FAN_3}, 3);

    // Botones
    load_textures(tex_button_L, (const char*[]){IMG_BUTTON_L_1, IMG_BUTTON_L_2, IMG_BUTTON_L_3, IMG_BUTTON_L_4}, 4);
    load_textures(tex_button_R, (const char*[]){IMG_BUTTON_R_1, IMG_BUTTON_R_2, IMG_BUTTON_R_3, IMG_BUTTON_R_4}, 4);

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
    load_textures(tex_door_L_close, paths_door_L, DOOR_FRAMES);
    load_textures(tex_door_R_close, paths_door_R, DOOR_FRAMES);

    // Alucinaciones
    load_textures(tex_hallucinations, (const char*[]){
        IMG_FREDDY_HALLUTINATION, IMG_ITS_ME_1, IMG_BONNIE_HALLUTINATION, IMG_ITS_ME_2
    }, 4);
    for (int i = 0; i < 4; i++)
        if (tex_hallucinations[i]) SDL_SetTextureBlendMode(tex_hallucinations[i], SDL_BLENDMODE_BLEND);

    // Jumpscares
    load_textures(tex_bonnie_jumpscare, (const char*[]){
        IMG_BONNIE_JUMPSCARE_1,  IMG_BONNIE_JUMPSCARE_2,  IMG_BONNIE_JUMPSCARE_3,
        IMG_BONNIE_JUMPSCARE_4,  IMG_BONNIE_JUMPSCARE_5,  IMG_BONNIE_JUMPSCARE_6,
        IMG_BONNIE_JUMPSCARE_7,  IMG_BONNIE_JUMPSCARE_8,  IMG_BONNIE_JUMPSCARE_9,
        IMG_BONNIE_JUMPSCARE_10, IMG_BONNIE_JUMPSCARE_11
    }, JUMPSCARE_BONNIE_FRAMES);
    load_textures(tex_chica_jumpscare, (const char*[]){
        IMG_CHICA_JUMPSCARE_1,  IMG_CHICA_JUMPSCARE_2,  IMG_CHICA_JUMPSCARE_3,
        IMG_CHICA_JUMPSCARE_4,  IMG_CHICA_JUMPSCARE_5,  IMG_CHICA_JUMPSCARE_6,
        IMG_CHICA_JUMPSCARE_7,  IMG_CHICA_JUMPSCARE_8,  IMG_CHICA_JUMPSCARE_9,
        IMG_CHICA_JUMPSCARE_10, IMG_CHICA_JUMPSCARE_11, IMG_CHICA_JUMPSCARE_12,
        IMG_CHICA_JUMPSCARE_13, IMG_CHICA_JUMPSCARE_14, IMG_CHICA_JUMPSCARE_15,
        IMG_CHICA_JUMPSCARE_16
    }, JUMPSCARE_CHICA_FRAMES);
    load_textures(tex_freddy_jumpscare, (const char*[]){
        IMG_FREDDY_JUMPSCARE_1,  IMG_FREDDY_JUMPSCARE_2,  IMG_FREDDY_JUMPSCARE_3,
        IMG_FREDDY_JUMPSCARE_4,  IMG_FREDDY_JUMPSCARE_5,  IMG_FREDDY_JUMPSCARE_6,
        IMG_FREDDY_JUMPSCARE_7,  IMG_FREDDY_JUMPSCARE_8,  IMG_FREDDY_JUMPSCARE_9,
        IMG_FREDDY_JUMPSCARE_10, IMG_FREDDY_JUMPSCARE_11, IMG_FREDDY_JUMPSCARE_12,
        IMG_FREDDY_JUMPSCARE_13, IMG_FREDDY_JUMPSCARE_14, IMG_FREDDY_JUMPSCARE_15,
        IMG_FREDDY_JUMPSCARE_16, IMG_FREDDY_JUMPSCARE_17, IMG_FREDDY_JUMPSCARE_18,
        IMG_FREDDY_JUMPSCARE_19, IMG_FREDDY_JUMPSCARE_20, IMG_FREDDY_JUMPSCARE_21,
        IMG_FREDDY_JUMPSCARE_22, IMG_FREDDY_JUMPSCARE_23, IMG_FREDDY_JUMPSCARE_24,
        IMG_FREDDY_JUMPSCARE_25, IMG_FREDDY_JUMPSCARE_26, IMG_FREDDY_JUMPSCARE_27,
        IMG_FREDDY_JUMPSCARE_28, IMG_FREDDY_JUMPSCARE_29, IMG_FREDDY_JUMPSCARE_30,
        IMG_FREDDY_JUMPSCARE_31
    }, JUMPSCARE_FREDDY_FRAMES);

    // Audio
    sfx_fan          = audio_load_sfx("romfs:/sfx/Buzz_Fan_Florescent2.wav");
    sfx_light        = audio_load_sfx("romfs:/sfx/BallastHumMedium2.wav");
    sfx_door         = audio_load_sfx("romfs:/sfx/SFXBible_12478.wav");
    sfx_circus       = audio_load_sfx("romfs:/sfx/circus.wav");
    sfx_pounding     = audio_load_sfx("romfs:/sfx/DOOR_POUNDING_ME_D0291401.wav");
    sfx_window_scare = audio_load_sfx("romfs:/sfx/windowscare.wav");
    sfx_error        = audio_load_sfx("romfs:/sfx/error.wav");
    sfx_steps        = audio_load_sfx("romfs:/sfx/deep_steps.wav");
    sfx_jumpscare    = audio_load_sfx("romfs:/sfx/XSCREAM.wav");
    sfx_running_fast = audio_load_sfx("romfs:/sfx/running fast3.wav");

    const char* breath_paths[4]  = {"romfs:/sfx/Vocals_Breaths_S_35972006.wav","romfs:/sfx/Vocals_Breaths_S_35972008.wav","romfs:/sfx/Vocals_Breaths_S_35972012.wav","romfs:/sfx/Vocals_Breaths_S_35972014.wav"};
    const char* halluc_paths[4]  = {"romfs:/sfx/COMPUTER_DIGITAL_L2076505.wav","romfs:/sfx/garble1.wav","romfs:/sfx/garble2.wav","romfs:/sfx/garble3.wav"};
    const char* kitchen_paths[4] = {"romfs:/sfx/OVEN-DRA_1_GEN-HDF18119.wav","romfs:/sfx/OVEN-DRA_2_GEN-HDF18120.wav","romfs:/sfx/OVEN-DRA_7_GEN-HDF18121.wav","romfs:/sfx/OVEN-DRAWE_GEN-HDF18122.wav"};
    const char* laugh_paths[3]   = {"romfs:/sfx/Laugh_Giggle_Girl_1d.wav","romfs:/sfx/Laugh_Giggle_Girl_2d.wav","romfs:/sfx/Laugh_Giggle_Girl_8d.wav"};
    for (int i = 0; i < 4; i++) sfx_breath[i]       = audio_load_sfx(breath_paths[i]);
    for (int i = 0; i < 4; i++) sfx_hallucination[i] = audio_load_sfx(halluc_paths[i]);
    for (int i = 0; i < 4; i++) sfx_kitchen[i]       = audio_load_sfx(kitchen_paths[i]);
    for (int i = 0; i < 3; i++) sfx_freddy_laugh[i]  = audio_load_sfx(laugh_paths[i]);

    // Estado inicial
    camera_x = 160.0f;
    door_L_frame = door_R_frame = 0.0f;
    left_door_on = right_door_on = left_light_on = right_light_on = false;
    fan_timer = fan_frame = 0;
    is_winning = false; win_fade = 0.0f;
    random_sound_timer = 0;
    bonnie_scare_played = chica_scare_played = false;
    breath_timer = kitchen_timer = 0;
    channel_breath = channel_circus = channel_kitchen = -1;
    is_bonnie_jumpscare = is_chica_jumpscare = is_freddy_jumpscare = false;
    bonnie_jumpscare_frame = chica_jumpscare_frame = freddy_jumpscare_frame = 0.0f;
    bonnie_force_down_timer = chica_force_down_timer = jumpscare_duration_timer = 0;
    hallucination_timer = hallucination_hold_timer = 0;
    current_hallucination = -1;

    audio_play_music("romfs:/sfx/ColdPresc_B.wav");
    audio_set_music_volume(50);
    if (sfx_fan) {
        audio_set_sfx_volume(sfx_fan, 25);
        channel_fan = audio_play_sfx_loop_chunk(sfx_fan);
    }
}

// ── Update helpers ────────────────────────────────────────────────────────────

static void update_hallucination(void) {
    if (random_sound_timer % 60 == 0 && (rand() % 1000) == 0) {
        hallucination_timer = 100;
        int s = rand() % 4;
        if (sfx_hallucination[s]) {
            audio_set_sfx_volume(sfx_hallucination[s], 100);
            audio_play_sfx_chunk(sfx_hallucination[s]);
        }
    }
    if (hallucination_timer > 0) {
        hallucination_timer--;
        if (hallucination_hold_timer <= 0) {
            hallucination_hold_timer = 2;
            current_hallucination = ((rand() % 10) < 3) ? rand() % 4 : -1;
        } else {
            hallucination_hold_timer--;
        }
    } else {
        current_hallucination = -1;
        hallucination_hold_timer = 0;
    }
}

static void update_kitchen_audio(void) {
    if (animatronics_get_chica_room() == CAM_6) {
        if (++kitchen_timer >= 240) {
            kitchen_timer = 0;
            if (rand() % 2 == 0) {
                int r = rand() % 4;
                if (sfx_kitchen[r]) {
                    audio_set_sfx_volume(sfx_kitchen[r], 128);
                    channel_kitchen = audio_play_sfx_chunk(sfx_kitchen[r]);
                }
            }
        }
    } else {
        kitchen_timer = 0;
    }
    if (channel_kitchen != -1) {
        if (Mix_Playing(channel_kitchen)) {
            int vol = 30;
            if (camera_system_is_open())
                vol = (camera_system_get_current_cam() == CAM_6) ? 100 : 50;
            audio_set_channel_volume(channel_kitchen, vol);
        } else {
            channel_kitchen = -1;
        }
    }
}

static void play_sfx_vol(Mix_Chunk* sfx, int vol) {
    if (!sfx) return;
    audio_set_sfx_volume(sfx, vol);
    audio_play_sfx_chunk(sfx);
}

static void update_animatronic_sounds(void) {
    if (animatronics_get_bonnie_moved_timer() == 10)
        play_sfx_vol(sfx_steps, 50);

    if (animatronics_get_chica_moved_timer() == 10) {
        if (animatronics_get_chica_room() != CAM_6 && animatronics_get_chica_prev_room() != CAM_6)
            play_sfx_vol(sfx_steps, 50);
    }

    if (animatronics_get_freddy_moved_timer() == 10) {
        int r = rand() % 3;
        play_sfx_vol(sfx_freddy_laugh[r], 100);
        play_sfx_vol(sfx_running_fast, 70);
    }
}

static void update_breath_and_death(void) {
    bool bonnie_in = (animatronics_get_bonnie_room() == ROOM_OFFICE);
    bool chica_in  = (animatronics_get_chica_room()  == ROOM_OFFICE);

    if (!bonnie_in && !chica_in) {
        breath_timer = bonnie_force_down_timer = chica_force_down_timer = 0;
        return;
    }

    if (camera_system_is_open()) {
        if (++breath_timer >= 300) {
            breath_timer = 0;
            if ((channel_breath == -1 || !Mix_Playing(channel_breath)) && rand() % 3 == 0) {
                int r = rand() % 4;
                if (sfx_breath[r]) {
                    audio_set_sfx_volume(sfx_breath[r], 100);
                    channel_breath = audio_play_sfx_chunk(sfx_breath[r]);
                }
            }
        }
        if (bonnie_in) bonnie_force_down_timer++;
        if (chica_in)  chica_force_down_timer++;
    }

    bool kill_bonnie = (bonnie_force_down_timer >= 1800) ||
        (input_get_button_down(HidNpadButton_A) && camera_system_is_open() && bonnie_in);
    bool kill_chica  = (chica_force_down_timer  >= 1800) ||
        (input_get_button_down(HidNpadButton_A) && camera_system_is_open() && chica_in);

    if (kill_bonnie && !is_bonnie_jumpscare && !is_chica_jumpscare && !is_freddy_jumpscare)
        trigger_jumpscare(&is_bonnie_jumpscare);
    else if (kill_chica && !is_chica_jumpscare && !is_bonnie_jumpscare && !is_freddy_jumpscare)
        trigger_jumpscare(&is_chica_jumpscare);
}

static void update_door_frame(float* frame, bool door_on) {
    if (door_on)  { if (*frame < DOOR_FRAMES - 1) *frame += DOOR_ANIM_SPEED; }
    else          { if (*frame > 0.0f)             *frame -= DOOR_ANIM_SPEED; }
}

static void toggle_light(bool* this_light, bool* other_light, int* this_ch, int* other_ch) {
    *this_light = !*this_light;
    if (*this_light) {
        *other_light = false;
        audio_stop_channel(*other_ch);
        *this_ch = audio_play_sfx_loop_chunk(sfx_light);
    } else {
        audio_stop_channel(*this_ch);
        *this_ch = -1;
    }
}

// ── Update ────────────────────────────────────────────────────────────────────

void game_update(void) {
    if (is_bonnie_jumpscare || is_chica_jumpscare || is_freddy_jumpscare) {
        if (is_bonnie_jumpscare) {
            bonnie_jumpscare_frame += JUMPSCARE_ANIM_SPEED;
            if (bonnie_jumpscare_frame >= JUMPSCARE_BONNIE_FRAMES) bonnie_jumpscare_frame = 0.0f;
        } else if (is_chica_jumpscare) {
            chica_jumpscare_frame += JUMPSCARE_ANIM_SPEED;
            if (chica_jumpscare_frame >= JUMPSCARE_CHICA_FRAMES) chica_jumpscare_frame = 0.0f;
        } else if (is_freddy_jumpscare) {
            if (freddy_jumpscare_frame < JUMPSCARE_FREDDY_FRAMES - 1) freddy_jumpscare_frame += 0.5f;
        }
        if (++jumpscare_duration_timer >= 85) state_manager_change(STATE_TITLE);
        return;
    }

    hud_update();
    camera_system_update();

    int items_on = left_door_on + right_door_on + left_light_on + right_light_on + camera_system_is_open();
    bool just_blacked_out = power_system_update(items_on);

    if (just_blacked_out) {
        audio_stop_channel(channel_light_L); audio_stop_channel(channel_light_R);
        audio_stop_channel(channel_fan);     audio_stop_channel(channel_kitchen);
        channel_light_L = channel_light_R = channel_fan = channel_kitchen = -1;
        left_light_on = right_light_on = false;
        if (left_door_on)  { left_door_on  = false; audio_play_sfx_chunk(sfx_door); }
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
        if (win_fade >= 255.0f) state_manager_change(STATE_6AM);
        return;
    }

    random_sound_timer++;
    if (random_sound_timer % 300 == 0 && (rand() % 30) == 0) {
        if (channel_circus == -1 || !Mix_Playing(channel_circus))
            channel_circus = audio_play_sfx_chunk(sfx_circus);
    }
    if (random_sound_timer % 600 == 0 && (rand() % 50) == 0) {
        int vol = 10 + (rand() % 40);
        audio_set_sfx_volume(sfx_pounding, vol);
        audio_play_sfx_chunk(sfx_pounding);
    }

    if (!is_power_out) {
        animatronics_update(left_door_on, right_door_on, camera_system_is_open());
        update_animatronic_sounds();

        if (animatronics_get_bonnie_room() != ROOM_DOOR_LEFT)  bonnie_scare_played = false;
        if (animatronics_get_chica_room()  != ROOM_DOOR_RIGHT) chica_scare_played  = false;

        if (animatronics_get_bonnie_room() == ROOM_OFFICE && left_light_on) {
            left_light_on = false; audio_stop_channel(channel_light_L); channel_light_L = -1;
        }
        if (animatronics_get_chica_room() == ROOM_OFFICE && right_light_on) {
            right_light_on = false; audio_stop_channel(channel_light_R); channel_light_R = -1;
        }

        update_hallucination();
        update_kitchen_audio();
        update_breath_and_death();
    } else {
        hallucination_timer = current_hallucination = 0; current_hallucination = -1;
        breath_timer = bonnie_force_down_timer = chica_force_down_timer = kitchen_timer = 0;
    }

    if (!camera_system_is_open() && camera_system_get_frame() <= 0.0f) {
        s16 stick_x = input_get_stick_x(0);
        if (stick_x > 7000 || stick_x < -7000) camera_x += (stick_x / 32767.0f) * 5.0f;
        if (camera_x < 0)   camera_x = 0;
        if (camera_x > 320) camera_x = 320;
    }

    if (input_get_button_down(HidNpadButton_Plus)) state_manager_change(STATE_TITLE);

    if (!is_power_out) {
        if (!camera_system_is_open()) {
            if (input_get_button_down(HidNpadButton_L)) {
                if (animatronics_get_bonnie_room() == ROOM_OFFICE) audio_play_sfx_chunk(sfx_error);
                else if (door_L_frame <= 0.0f || door_L_frame >= DOOR_FRAMES - 1) {
                    left_door_on = !left_door_on;
                    audio_play_sfx_chunk(sfx_door);
                }
            }
            if (input_get_button_down(HidNpadButton_ZL)) {
                if (animatronics_get_bonnie_room() == ROOM_OFFICE) audio_play_sfx_chunk(sfx_error);
                else toggle_light(&left_light_on, &right_light_on, &channel_light_L, &channel_light_R);
            }
            if (input_get_button_down(HidNpadButton_R)) {
                if (animatronics_get_chica_room() == ROOM_OFFICE) audio_play_sfx_chunk(sfx_error);
                else if (door_R_frame <= 0.0f || door_R_frame >= DOOR_FRAMES - 1) {
                    right_door_on = !right_door_on;
                    audio_play_sfx_chunk(sfx_door);
                }
            }
            if (input_get_button_down(HidNpadButton_ZR)) {
                if (animatronics_get_chica_room() == ROOM_OFFICE) audio_play_sfx_chunk(sfx_error);
                else toggle_light(&right_light_on, &left_light_on, &channel_light_R, &channel_light_L);
            }
        }

        if (input_get_button_down(HidNpadButton_A)) {
            camera_system_toggle();
            if (camera_system_is_open()) {
                left_light_on = right_light_on = false;
                audio_stop_channel(channel_light_L); audio_stop_channel(channel_light_R);
                channel_light_L = channel_light_R = -1;
                audio_set_sfx_volume(sfx_fan, 10);
            } else {
                audio_set_sfx_volume(sfx_fan, 25);
                if      (animatronics_get_bonnie_room() == ROOM_OFFICE && !is_chica_jumpscare && !is_freddy_jumpscare)
                    trigger_jumpscare(&is_bonnie_jumpscare);
                else if (animatronics_get_chica_room()  == ROOM_OFFICE && !is_bonnie_jumpscare && !is_freddy_jumpscare)
                    trigger_jumpscare(&is_chica_jumpscare);
                else if (animatronics_get_freddy_room() == ROOM_OFFICE && !is_bonnie_jumpscare && !is_chica_jumpscare)
                    trigger_jumpscare(&is_freddy_jumpscare);
            }
        }
    }

    fan_timer++;
    if (!is_power_out && fan_timer >= FAN_ANIM_SPEED) {
        fan_timer = 0;
        if (++fan_frame > 2) fan_frame = 0;
    }

    update_door_frame(&door_L_frame, left_door_on);
    update_door_frame(&door_R_frame, right_door_on);
}

// ── Draw ──────────────────────────────────────────────────────────────────────

static void draw_jumpscare(SDL_Texture** frames, int total, float anim_frame) {
    int f = (int)anim_frame;
    if (f >= total) f = total - 1;
    if (!frames[f]) return;
    SDL_Rect src = {160, 0, 1280, 720}, dst = {0, 0, 1280, 720};
    SDL_RenderCopy(graphics_get_renderer(), frames[f], &src, &dst);
}

void game_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();
    SDL_Texture* bg = tex_office_normal;

    if (is_power_out) {
        if      (powerout_state == 0) bg = tex_office_blackout;
        else if (powerout_state == 1) bg = show_freddy ? tex_office_blackout_freddy : tex_office_blackout;
        else if (powerout_state == 2) bg = (rand() % 2 == 0) ? tex_office_blackout : NULL;
        else bg = NULL;
    } else {
        int flicker = rand() % 10;
        if (left_light_on && flicker > 1) {
            if (animatronics_get_bonnie_room() == ROOM_DOOR_LEFT) {
                bg = tex_office_bonnie;
                if (!bonnie_scare_played) { audio_play_sfx_chunk(sfx_window_scare); bonnie_scare_played = true; }
            } else bg = tex_office_light_L;
        } else if (right_light_on && flicker > 1) {
            if (animatronics_get_chica_room() == ROOM_DOOR_RIGHT) {
                bg = tex_office_chica;
                if (!chica_scare_played) { audio_play_sfx_chunk(sfx_window_scare); chica_scare_played = true; }
            } else bg = tex_office_light_R;
        }
    }

    if (bg) {
        SDL_Rect src = {(int)camera_x, 0, 1280, 720};
        SDL_RenderCopy(renderer, bg, &src, NULL);
    }

    // Puertas
    int fl = (int)door_L_frame;
    if (fl >= 0 && tex_door_L_close[fl]) {
        SDL_Rect dst = {72 - (int)camera_x, -1, 223, 720};
        SDL_RenderCopy(renderer, tex_door_L_close[fl], NULL, &dst);
    }
    int fr = (int)door_R_frame;
    if (fr >= 0 && tex_door_R_close[fr]) {
        SDL_Rect dst = {1270 - (int)camera_x, -2, 248, 720};
        SDL_RenderCopy(renderer, tex_door_R_close[fr], NULL, &dst);
    }

    camera_system_draw_room();
    camera_system_draw_ui();

    if (!is_power_out) {
        bool cam_full = (camera_system_is_open() && camera_system_get_frame() >= (CAM_FRAMES - 1));
        if (!cam_full) {
            int sL = (left_door_on ? 1 : 0) | (left_light_on  ? 2 : 0);
            int sR = (right_door_on? 1 : 0) | (right_light_on ? 2 : 0);
            if (tex_button_L[sL]) {
                SDL_Rect dst = {6  - (int)camera_x, 263, 92, 247};
                SDL_RenderCopy(renderer, tex_button_L[sL], NULL, &dst);
            }
            if (tex_button_R[sR]) {
                SDL_Rect dst = {1497 - (int)camera_x, 273, 92, 247};
                SDL_RenderCopy(renderer, tex_button_R[sR], NULL, &dst);
            }
            if (tex_fan[fan_frame]) {
                int w, h;
                SDL_QueryTexture(tex_fan[fan_frame], NULL, NULL, &w, &h);
                SDL_Rect dst = {780 - (int)camera_x, 303, w, h};
                SDL_RenderCopy(renderer, tex_fan[fan_frame], NULL, &dst);
            }
        }
        hud_draw(camera_system_is_open(), camera_system_get_frame());
        power_system_draw_hud(camera_system_is_open(), camera_system_get_frame());
        camera_system_draw_button();
    }

    power_system_draw_jumpscare();
    camera_system_draw_animation();

    // Alucinaciones
    if (hallucination_timer > 0 && !is_power_out && current_hallucination >= 0 && tex_hallucinations[current_hallucination]) {
        SDL_Rect dst = {0, 0, 1280, 720};
        SDL_RenderCopy(renderer, tex_hallucinations[current_hallucination], NULL, &dst);
    }

    // Jumpscares
    if      (is_bonnie_jumpscare) draw_jumpscare(tex_bonnie_jumpscare, JUMPSCARE_BONNIE_FRAMES, bonnie_jumpscare_frame);
    else if (is_chica_jumpscare)  draw_jumpscare(tex_chica_jumpscare,  JUMPSCARE_CHICA_FRAMES,  chica_jumpscare_frame);
    else if (is_freddy_jumpscare) draw_jumpscare(tex_freddy_jumpscare, JUMPSCARE_FREDDY_FRAMES, freddy_jumpscare_frame);

    if (is_winning) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)win_fade);
        SDL_Rect full = {0, 0, 1280, 720};
        SDL_RenderFillRect(renderer, &full);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

// ── Cleanup ───────────────────────────────────────────────────────────────────

void game_cleanup(void) {
    hud_cleanup();
    power_system_cleanup();
    camera_system_cleanup();
    animatronics_cleanup();

    SDL_Texture* single_texs[] = {
        tex_office_normal, tex_office_light_L, tex_office_light_R,
        tex_office_blackout, tex_office_blackout_freddy,
        tex_office_bonnie, tex_office_chica
    };
    for (int i = 0; i < 7; i++)
        if (single_texs[i]) { SDL_DestroyTexture(single_texs[i]); single_texs[i] = NULL; }

    destroy_textures(tex_door_L_close, DOOR_FRAMES);
    destroy_textures(tex_door_R_close, DOOR_FRAMES);
    destroy_textures(tex_button_L, 4);
    destroy_textures(tex_button_R, 4);
    destroy_textures(tex_fan, 3);
    destroy_textures(tex_hallucinations, 4);
    destroy_textures(tex_bonnie_jumpscare, JUMPSCARE_BONNIE_FRAMES);
    destroy_textures(tex_chica_jumpscare,  JUMPSCARE_CHICA_FRAMES);
    destroy_textures(tex_freddy_jumpscare, JUMPSCARE_FREDDY_FRAMES);

    audio_stop_music();
    audio_stop_all_sfx();

    Mix_Chunk* single_sfx[] = {sfx_fan, sfx_light, sfx_door, sfx_circus, sfx_pounding,
                                sfx_window_scare, sfx_error, sfx_steps, sfx_jumpscare, sfx_running_fast};
    for (int i = 0; i < 10; i++)
        if (single_sfx[i]) audio_free_sfx(single_sfx[i]);

    free_sfx_array(sfx_breath,       4);
    free_sfx_array(sfx_hallucination,4);
    free_sfx_array(sfx_kitchen,      4);
    free_sfx_array(sfx_freddy_laugh, 3);
}
