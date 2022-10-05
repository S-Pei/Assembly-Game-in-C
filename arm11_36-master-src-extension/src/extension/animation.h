#include <SDL2/SDL.h>
#include <stdbool.h>
#include "globals.h"
#include "game_structs.h"

#ifndef ANIMATION_H
#define ANIMATION_H

//Updates value of the object and the value shown on the sreen
void change_value(Object *obj, SDL_Renderer *renderer, char *new_value, TTF_Font *font, SDL_Color *color);

// returns 1 if the object has reached its destination
int move(Object *obj, int target_x, int *current_frame, SDL_Renderer *renderer, 
        bool change_value, TTF_Font *font, SDL_Color *color, Object **birds, char *new_value);

int egg_hatch_animation(Object *egg, int *frame);

#endif
