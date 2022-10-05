#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "game_structs.h"

#ifndef PLAYER_INPUT_H
#define PLAYER_INPUT_H

#define MAX_LINE_INPUT 512

PlayerInput *create_player_input(char *file, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color);

void update_player_input_value(PlayerInput *player_input, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color, int mode, int size);
#endif
