#include <SDL2/SDL.h>

#include "globals.h"
#include "game_structs.h"

#ifndef SPRITES_H
#define SPRITES_H

#define DISPLAY_NUM 0
#define PORTAL_INTEVERAL(n) (WINDOW_WIDTH / n)
#define TEXTURE_HEIGHT 3
#define OBJECT_RENDER_SCALE_FACTOR 4.0

#define BG_ISLAND_START 23
#define BG_ISLAND_END 233
#define BG_ISLAND_WIDTH (BG_ISLAND_END - BG_ISLAND_START)
#define BG_ISLAND_CENTER (BG_ISLAND_START + (1 / BG_ISLAND_WIDTH))
#define BG_ISLAND_FLOOR_LEVEL 112

#define PORTAL_OFFSET 15

#define CHARACTER_EDGE_OFFSET 55
#define CHARACTER_INIT_X (BG_ISLAND_START + CHARACTER_EDGE_OFFSET)
#define CHARACTER_HEIGHT 31
#define CHARACTER_NUM_FRAMES(state) ((state == (motions) IDLE) ? 2 : 4)
#define CHARACTER_FRAME_TIME(state) ((state == (motions) IDLE) ? 120 : 50)

#define EGG_HEIGHT 11
#define EGG_WIDTH 15
#define EGG_NUM_FRAMES 7
#define EGG_FRAME_TIME 120
#define EGG_SPACING (BG_ISLAND_WIDTH / (MAX_IMMS + 1))
#define EGG_LABEL_OFFSET_1 20
#define EGG_TEXT_SCALE_FACTOR 0.75

#define BIRD_WIDTH 15
#define BIRD_CHARACTER_OFFSET 12
#define BIRD_NUM_FRAMES(state) (state == 2 ? 4 : 2)
#define BIRD_FRAME_TIME 7

#define PC_MIDDLE_X (WINDOW_WIDTH - 205) 
#define PC_MIDDLE_Y 290
#define PC_SCALE_FACTOR 1.5

#define GOAL_MIDDLE_X 115
#define GOAL_MIDDLE_Y 110
#define GOAL_SCALE_FACTOR 0.7
#define GOAL_OFFSET 15


AnimatePanel *create_panel(const int *targets_x, const int steps, char *output, const int *targets_type);

void free_panel();

Background *create_bg(char *file, SDL_Renderer *renderer);

void free_background(Background *bg);

Object *create_object(char *file, Frame_info *frame_info, Position init_pos,
                      char *value, SDL_Renderer *renderer,
                      TTF_Font *font, SDL_Color *color);

Object *create_character(char *file,
                        char *value, SDL_Renderer *renderer,
                        TTF_Font *font, SDL_Color *color);

void free_object(Object *objs);

int init_portals(char *file, Level *lvl, Portal **portals, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color);

void free_portals(Level *lvl, Portal **portals);

void init_eggs(char *file, Level *lvl, Object **eggs, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color);

void free_eggs(Level *lvl, Object **eggs);

void init_birds(char *file, Level *lvl, Object **birds, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color);

void free_birds(Object **birds);

Object *create_text_object(char *value, SDL_Renderer *renderer, Position init_pos, TTF_Font *font, SDL_Color *color);

void free_text_object(Object *obj);

Stage *create_stage(Portal **portal_ptrs, Object **egg_ptrs, Object **bird_ptrs, Level *lvl);

#endif
