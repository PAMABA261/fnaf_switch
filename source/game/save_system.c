#include "game/save_system.h"
#include <stdio.h>

extern int current_night;

// En emulador -> "save.ini", en switch real -> sdmc:/save.ini""
#define SAVE_FILE "save.ini" 

void save_system_load(void) {
    FILE* file = fopen(SAVE_FILE, "r"); 
    if (file) {
        fscanf(file, "%d", &current_night);
        fclose(file);
    } else {
        current_night = 1;
    }
}

void save_system_save(void) {
    FILE* file = fopen(SAVE_FILE, "w"); 
    if (file) {
        fprintf(file, "%d", current_night);
        fclose(file);
    }
}