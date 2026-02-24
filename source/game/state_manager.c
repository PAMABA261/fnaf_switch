#include "game/state_manager.h"
#include "engine/graphics.h"
#include "engine/input.h"

// Avisamos al mánager de que estas funciones existen en menu.c
void menu_init(void);
void menu_update(void);
void menu_draw(void);
void menu_cleanup(void);

// Avisamos al mánager de que estas funciones existen en ad.c
void ad_init(void);
void ad_update(void);
void ad_draw(void);
void ad_cleanup(void);

// Avisamos al mánager de que estas funciones existen en what_day.c
void what_day_init(void);
void what_day_update(void);
void what_day_draw(void);
void what_day_cleanup(void);

// Avisamos al mánager de que estas funciones existen en game.c
void game_init(void);
void game_update(void);
void game_draw(void);
void game_cleanup(void);

static GameState current_state;

void state_manager_init(GameState initial_state) {
    current_state = initial_state;
    
    if (current_state == STATE_TITLE) {
        menu_init();
    }
}

void state_manager_update(void) {
    switch (current_state) {
        case STATE_TITLE:
            menu_update(); 
            break;
        case STATE_AD:
            ad_update();
            break;
        case STATE_WHAT_DAY:
            what_day_update();
            break;
        case STATE_GAME:
            game_update();
            break;
        case STATE_GAMEOVER:
            break;
        case STATE_CUSTOMIZE: 
            break;
        case STATE_THE_END:   
            break;
        default: break;
    }
}

void state_manager_draw(void) {
    switch (current_state) {
        case STATE_TITLE:
            menu_draw(); 
            break;
        case STATE_AD:
            ad_draw();
            break;
        case STATE_WHAT_DAY:
            what_day_draw();
            break;
        case STATE_GAME:
            game_draw();
            break;
        case STATE_GAMEOVER:
            // Estática de televisión
            break;
        case STATE_CUSTOMIZE: 
            // Menú de Custom Night
            break;
        case STATE_THE_END:   
            // Cheques de pago
            break;
        default: break;
    }
}

void state_manager_change(GameState new_state) {
    if (current_state == STATE_TITLE) {
        menu_cleanup();
    } else if(current_state == STATE_AD) {
        ad_cleanup();
    } else if (current_state == STATE_WHAT_DAY) {
        what_day_cleanup();
    } else if (current_state == STATE_GAME) {
        game_cleanup();
    } else if (current_state == STATE_GAMEOVER) {
        // Limpiar la pantalla de Game Over
    } else if (current_state == STATE_CUSTOMIZE) {
        // Limpiar el menú de Custom Night
    } else if (current_state == STATE_THE_END) {
        // Limpiar los cheques de pago
    }

    current_state = new_state;

    if (current_state == STATE_TITLE) {
        menu_init();
    } else if(current_state == STATE_AD) {
        ad_init();
    } else if(current_state == STATE_WHAT_DAY) {
        what_day_init();
    } else if(current_state == STATE_GAME) {
        game_init();
    } else if(current_state == STATE_GAMEOVER) {
        // Inicializar la pantalla de Game Over
    } else if(current_state == STATE_CUSTOMIZE) {
        // Inicializar el menú de Custom Night
    } else if(current_state == STATE_THE_END) {
        // Inicializar los cheques de pago
    }
}

void state_manager_cleanup(void) {
    // Al cerrar el juego, limpiamos la memoria de las fotos del menú
    menu_cleanup();
}