#include "game_structs.h"
#include "globals.h"
#include "../assemble_files/defs.h"
#include "../assemble_files/assemble_main.h"
#include "../emulate_files/dc_ex.h"
#include "../assemble_files/data_processing_a.h"
#include "game_structs.h"
#include "sprites.h"
#include "globals.h"

#ifndef PARSE_PLAYER_INPUT_H
#define PARSE_PLAYER_INPUT_H

Object **del_egg_in_array(char *value, Object **objs, int size);

AnimatePanel *parse_player_input();

#endif
