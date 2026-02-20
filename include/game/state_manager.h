#pragma once

// Lista de pantallas (Frames de Clickteam)
typedef enum {
    STATE_TITLE,        // Frame 1: El Menú Principal (Freddy parpadeando)
    STATE_AD,           // Frame 10: El Periódico (Help Wanted)
    STATE_WHAT_DAY,     // Frame 2: El texto "12:00 AM - 1st Night"
    STATE_GAME,         // Frame 3: La Oficina 
    STATE_GAMEOVER,     // Frame 8: La estática y el "Game Over"
    STATE_CUSTOMIZE,    // Frame 12: Custom Night 
    STATE_THE_END       // Frames 9, 11, 13: Los cheques de pago
} GameState;

void state_manager_init(GameState initial_state);
void state_manager_update(void);
void state_manager_draw(void);
void state_manager_change(GameState new_state);
void state_manager_cleanup(void);