#include "game/assets.h"
#include "engine/graphics.h"
#include "engine/audio.h"
#include "engine/input.h"
#include "game/state_manager.h"
#include <SDL2/SDL.h>
#include <stdlib.h> 

static SDL_Texture* tex_freddy[4];
static SDL_Texture* tex_static[8];
static SDL_Texture* tex_blip[8];
static SDL_Texture* tex_scanline = NULL;

static SDL_Texture* tex_title = NULL;
static SDL_Texture* tex_new_game = NULL;
static SDL_Texture* tex_continue = NULL;
static SDL_Texture* tex_selector = NULL;
static SDL_Texture* tex_night = NULL;
static SDL_Texture* tex_night_num[5];
int current_night = 1;

static int current_freddy = 0;
static int current_static = 0;
static int current_blip = 0;

static bool show_blip = false;
static Uint8 static_alpha = 150; //Trasparencia de la estática
static Uint8 blip_alpha = 255; //Trasparencia de los destellos
static float scanline_y = -38.0f; // Posición de la scanline

static int selected_option = 0; //  0 = New Game, 1 = Continue

//Relojes para saber cuándo cambiar la animación
static Uint64 last_freddy_time = 0;
static Uint64 last_static_time = 0;
static Uint64 last_blip_time = 0;

static bool is_transitioning = false;
static int transition_time = 0;

static SDL_Texture* tex_newspaper_fade = NULL;

static Mix_Chunk* sfx_static_menu = NULL;
static Mix_Chunk* sfx_menu_blip = NULL;

void menu_init(void) {
    //Cargar a Freddy
    tex_freddy[0] = graphics_load_texture(IMG_MENU_BASE);
    tex_freddy[1] = graphics_load_texture(IMG_MENU_VAR_1);
    tex_freddy[2] = graphics_load_texture(IMG_MENU_VAR_2);
    tex_freddy[3] = graphics_load_texture(IMG_MENU_VAR_3);

    // Cargar Estática 
    const char* static_paths[8] = {IMG_STATIC_1, IMG_STATIC_2, IMG_STATIC_3, IMG_STATIC_4, IMG_STATIC_5, IMG_STATIC_6, IMG_STATIC_7, IMG_STATIC_8};
    for(int i = 0; i < 8; i++) {
        tex_static[i] = graphics_load_texture(static_paths[i]);
        SDL_SetTextureBlendMode(tex_static[i], SDL_BLENDMODE_BLEND); 
    }

    // Cargar Blips
    const char* blip_paths[8] = {IMG_BLIP_1, IMG_BLIP_2, IMG_BLIP_3, IMG_BLIP_4, IMG_BLIP_5, IMG_BLIP_6, IMG_BLIP_7, IMG_BLIP_8};
    for(int i = 0; i < 8; i++) {
        tex_blip[i] = graphics_load_texture(blip_paths[i]);
        SDL_SetTextureBlendMode(tex_blip[i], SDL_BLENDMODE_BLEND);
    }

    // Cargar Scanline
    tex_scanline = graphics_load_texture(IMG_SCANLINE);
    if (tex_scanline) {
        SDL_SetTextureBlendMode(tex_scanline, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(tex_scanline, 100); // Transparencia fija para la scanline
    }

    tex_title = graphics_load_texture(IMG_TITLE);
    if (tex_title) SDL_SetTextureBlendMode(tex_title, SDL_BLENDMODE_BLEND);

    tex_new_game = graphics_load_texture(IMG_NEW_GAME);

    if (tex_new_game) SDL_SetTextureBlendMode(tex_new_game, SDL_BLENDMODE_BLEND);
    tex_continue = graphics_load_texture(IMG_CONTINUE);

    if (tex_continue) SDL_SetTextureBlendMode(tex_continue, SDL_BLENDMODE_BLEND);

    tex_selector = graphics_load_texture(IMG_SELECTOR);
    if (tex_selector) SDL_SetTextureBlendMode(tex_selector, SDL_BLENDMODE_BLEND);

    tex_night = graphics_load_texture(IMG_NIGHT);
    if (tex_night) SDL_SetTextureBlendMode(tex_night, SDL_BLENDMODE_BLEND);

    const char* num_paths[5] = {IMG_NIGHT_NUM_1, IMG_NIGHT_NUM_2, IMG_NIGHT_NUM_3, IMG_NIGHT_NUM_4, IMG_NIGHT_NUM_5};
    for(int i = 0; i < 5; i++) {
        tex_night_num[i] = graphics_load_texture(num_paths[i]);
        if (tex_night_num[i]) SDL_SetTextureBlendMode(tex_night_num[i], SDL_BLENDMODE_BLEND);
    }

    tex_newspaper_fade = graphics_load_texture(IMG_NEWSPAPER);
    if (tex_newspaper_fade) {
        SDL_SetTextureBlendMode(tex_newspaper_fade, SDL_BLENDMODE_BLEND); // Le activamos el modo de transparencia
    }

    audio_play_music("romfs:/sfx/darkness_music.wav");
    sfx_static_menu = audio_load_sfx("romfs:/sfx/static2.wav");
    audio_play_sfx_loop_chunk(sfx_static_menu);
    sfx_menu_blip = audio_load_sfx("romfs:/sfx/blip3.wav");
}

void menu_update(void) {
    Uint64 current_time = SDL_GetTicks64(); // Qué hora es ahora mismo en milisegundos

    // Mover la scanline lentamente hacia abajo
    scanline_y += 1.0f; // Velocidad de caída
    if (scanline_y > 720.0f) { // Si se sale de la pantalla, vuelve a empezar desde arriba
        scanline_y = -38.0f;
    }

    // LÓGICA DE FREDDY (Cada 80ms tiramos el dado de 100 caras)
    if (current_time > last_freddy_time + 80) {
        int r = rand() % 100; // Número aleatorio entre 0 y 99
        if (r == 99) current_freddy = 3; 
        else if (r == 98) current_freddy = 2; 
        else if (r == 97) current_freddy = 1; 
        else current_freddy = 0; // Freddy normal 

        last_freddy_time = current_time; // Actualizamos el reloj de Freddy 
    }

    // LÓGICA DE LA ESTÁTICA (Cada 40ms cambiamos de frame y de transparencia)
    if (current_time > last_static_time + 40) {
        current_static = (current_static + 1) % 8;
        static_alpha = 50 + (rand() % 100);  
        last_static_time = current_time;
    }

    // LÓGICA DE LOS BLIPS (Cada 300ms decidimos si aparece)
    if (current_time > last_blip_time + 300) {
        show_blip = ((rand() % 3) == 1); 
        if (show_blip) {
            current_blip = rand() % 8; 
            blip_alpha = 100 + (rand() % 100); 
        }
        last_blip_time = current_time;
    }

    // LÓGICA DEL MENÚ
    if (is_transitioning) {
        transition_time++; 
        if (selected_option == 0) {
            if (transition_time >= 120) {
                audio_stop_all_sfx();
                state_manager_change(STATE_AD);
            }
        } else if (selected_option == 1) {
            if (transition_time >= 20) {
                audio_stop_all_sfx();
                audio_stop_music();
                state_manager_change(STATE_WHAT_DAY);
            }
        }
    } else {
        if (input_get_button_down(HidNpadButton_Up)) {
            selected_option = 0;
            audio_play_sfx_chunk(sfx_menu_blip); 
        }

        if(input_get_button_down(HidNpadButton_Down)) {
            selected_option = 1;
            audio_play_sfx_chunk(sfx_menu_blip); 
        }

        if (input_get_button_down(HidNpadButton_A)) {
            is_transitioning = true;
            transition_time = 0;
        }
    }
}

void menu_draw(void) {
    SDL_Renderer* renderer = graphics_get_renderer();

    // Dibujar a Freddy
    if (tex_freddy[current_freddy]) {
        SDL_RenderCopy(renderer, tex_freddy[current_freddy], NULL, NULL);
    }

    // Dibujar el Blip encima de Freddy 
    if (show_blip && tex_blip[current_blip]) {
        SDL_SetTextureAlphaMod(tex_blip[current_blip], blip_alpha); // Aplicamos la transparencia
        SDL_RenderCopy(renderer, tex_blip[current_blip], NULL, NULL);
    }

    if (tex_scanline) {
        SDL_Rect dst_rect = {-19, (int)scanline_y, 1328, 32}; 
        SDL_RenderCopy(renderer, tex_scanline, NULL, &dst_rect);
    }

    if (tex_static[current_static]) {
        SDL_SetTextureAlphaMod(tex_static[current_static], static_alpha); // Aplicamos la transparencia
        SDL_RenderCopy(renderer, tex_static[current_static], NULL, NULL);
    }

    if(tex_title) {
        SDL_Rect title_rect = {175, 79, 201, 212};
        SDL_RenderCopy(renderer, tex_title, NULL, &title_rect);
    }

    if(tex_new_game) {
        SDL_Rect new_game_rect = {174, 404, 203, 33};
        SDL_RenderCopy(renderer, tex_new_game, NULL, &new_game_rect);
    }
    
    if(tex_continue) {
        SDL_Rect continue_rect = {173, 475, 204, 34};
        SDL_RenderCopy(renderer, tex_continue, NULL, &continue_rect);
    }

    if (selected_option == 1) {
        if (tex_night) {
            SDL_Rect night_rect = {175, 517, 63, 22}; 
            SDL_RenderCopy(renderer, tex_night, NULL, &night_rect);
        }
        if (tex_night_num[current_night - 1]) { 
            SDL_Rect num_rect = {248, 519, 14, 17}; 
            SDL_RenderCopy(renderer, tex_night_num[current_night - 1], NULL, &num_rect);
        }
    }

    if (tex_selector) {
        SDL_Rect sel_rect = {111, 0, 43, 26}; 
        if (selected_option == 0) sel_rect.y = 408; // Al lado de New Game
        if (selected_option == 1) sel_rect.y = 480; // Al lado de Continue
        
        SDL_RenderCopy(renderer, tex_selector, NULL, &sel_rect);
    }

    if (is_transitioning) {
        if (selected_option == 0 && tex_newspaper_fade) {
            int alpha = (transition_time * 255) / 120; // Calculamos la transparencia en función del tiempo de transición
            if (alpha > 255) alpha = 255; // Aseguramos que no supere el máximo

            SDL_SetTextureAlphaMod(tex_newspaper_fade, alpha);
            SDL_Rect fade_rect = {0, 0, 1280, 720};
            SDL_RenderCopy(renderer, tex_newspaper_fade, NULL, &fade_rect);
        }
    }
}

void menu_cleanup(void) {
    // Vaciar la memoria al salir 
    for(int i = 0; i < 4; i++) if (tex_freddy[i]) SDL_DestroyTexture(tex_freddy[i]);
    for(int i = 0; i < 8; i++) if (tex_static[i]) SDL_DestroyTexture(tex_static[i]);
    for(int i = 0; i < 8; i++) if (tex_blip[i]) SDL_DestroyTexture(tex_blip[i]);
    if (tex_scanline) SDL_DestroyTexture(tex_scanline);

    if (tex_title) SDL_DestroyTexture(tex_title);
    if (tex_new_game) SDL_DestroyTexture(tex_new_game);
    if (tex_continue) SDL_DestroyTexture(tex_continue);
    if (tex_selector) SDL_DestroyTexture(tex_selector);

    for(int i = 0; i < 5; i++) if (tex_night_num[i]) SDL_DestroyTexture(tex_night_num[i]);

    if (tex_newspaper_fade) SDL_DestroyTexture(tex_newspaper_fade);

    if (sfx_static_menu) audio_free_sfx(sfx_static_menu);
    if (sfx_menu_blip) audio_free_sfx(sfx_menu_blip);

    // Esto esta para que al volver al menú si mueres el juego no se quede pillado
    is_transitioning = false;
    transition_time = 0;
}