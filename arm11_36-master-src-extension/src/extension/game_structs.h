#ifndef GAME_STRUCTS_H
#define GAME_STRUCTS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"

typedef enum {
  IDLE, MOVE_RIGHT, MOVE_LEFT, JUMP_TO_PORTAL
} motions;

typedef struct Background {
  SDL_Texture *texture;
  SDL_Rect *rect;
} Background;

typedef struct Portal {
  SDL_Texture *texture;
  SDL_Rect *rect;
  Portal_info *info;
  SDL_Texture *num_text;
  SDL_Rect *num_rect;
  SDL_Texture *value_text;
  SDL_Rect *value_rect;
} Portal;

typedef struct Position {
  int x;
  int y;
} Position;

typedef struct Frame_info {
  int num_frames;
  int frame_time;
} Frame_info;

// Includes character, bird, egg, PC
// For characters, set value to NULL
typedef struct Object {
  SDL_Texture *texture;
  Frame_info *frame_info;
  SDL_Rect *source;
  SDL_Rect *dest;
  int state;
  float speed;
  float x_pos;
  char *value;
  SDL_Texture *value_text;
  SDL_Rect *value_rect;
} Object;

typedef struct PlayerInput {
  SDL_Texture *texture;
  SDL_Rect *rect;
  char *str;
  SDL_Texture *input_texture;
  SDL_Rect *input_rect;
  SDL_Rect *input_src_rect;
} PlayerInput;


typedef struct AnimatePanel {
  int *targets_x;
  int steps;
  // will update all portal values once player hits enter
  // will update all bird values
  char *output;
  int *targets_type;
} AnimatePanel;

typedef struct Stage {
  Portal **portal_ptrs;
  int portal_count;
  Object **egg_ptrs;
  int egg_count;
  Object **bird_ptrs;
} Stage;

//Big game end portal in main just render
//Goal screen also in main just render
//Create array of SDL_RENDER bird in main

#endif
