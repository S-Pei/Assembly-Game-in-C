#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <limits.h>
#include <assert.h>

#include "globals.h"
#include "sprites.h"
#include "game_structs.h"

AnimatePanel *create_panel(const int *targets_x, const int steps, char *output, const int *targets_type) {
    assert(targets_x);
    AnimatePanel *panel = malloc(sizeof(AnimatePanel));
    panel->steps = steps;
    panel->targets_x = calloc(steps, sizeof(int));
    panel->targets_type = calloc(steps, sizeof(int));
    for (int i=0; i<steps; i++) {
      panel->targets_x[i] = targets_x[i];
      panel->targets_type[i] = targets_type[i];
    }
    panel->output = output;
    return panel;
}

void free_panel(AnimatePanel *panel) {
    assert(panel);
    free(panel->targets_x);
    free(panel);
}


Background *create_bg(char *file, SDL_Renderer *renderer) {
  SDL_Surface *bg_surface = IMG_Load(file);
  SDL_Texture *bg_texture = SDL_CreateTextureFromSurface(renderer, bg_surface);
  if (!bg_texture) {
    printf("Error creating texture\n");
    exit(EXIT_FAILURE);
  }
  SDL_FreeSurface(bg_surface);

  SDL_Rect *bg_rect = malloc(sizeof(SDL_Rect));
  bg_rect->x = bg_rect->y = 0;
  bg_rect->h = WINDOW_HEIGHT;
  bg_rect->w = WINDOW_WIDTH;

  Background *ret = malloc(sizeof(bg_texture) + sizeof(bg_rect));
  ret->texture = bg_texture;
  ret->rect = bg_rect;
  return ret;
}

void free_background(Background *bg) {
  free(bg->rect);
  SDL_DestroyTexture(bg->texture);
  free(bg);
}

static int find_max_num_frames(Frame_info *frame_info, int size) {
  int max = INT_MIN;
  for (int i = 0; i < size; i++) {
    if (frame_info[i].num_frames > max) {
      max = frame_info[i].num_frames;
    }
  }
  return max;
}

Object *create_object(char *file, Frame_info *frame_info, Position init_pos,
                      char *value, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color) {
  Object *ret = malloc(sizeof(Object));
  SDL_Surface *char_surface = IMG_Load(file);
  SDL_Texture *char_texture = SDL_CreateTextureFromSurface(renderer, char_surface);
  if (!char_texture) {
    printf("Error creating texture\n");
    exit(EXIT_FAILURE);
  }
  SDL_FreeSurface(char_surface);
  int tex_w, tex_h;
  SDL_QueryTexture(char_texture, NULL, NULL, &tex_w, &tex_h);
  
  SDL_Rect *source_rect = malloc(sizeof(SDL_Rect));
  source_rect->x = source_rect->y = 0;
  source_rect->h = tex_h / TEXTURE_HEIGHT;
  source_rect->w = tex_w / find_max_num_frames(frame_info, TEXTURE_HEIGHT);

  SDL_Rect *dest_rect = malloc(sizeof(SDL_Rect));
  dest_rect->x = init_pos.x;
  dest_rect->y = init_pos.y;
  dest_rect->h = source_rect->h * OBJECT_RENDER_SCALE_FACTOR;
  dest_rect->w = source_rect->w * OBJECT_RENDER_SCALE_FACTOR;

  SDL_Surface *text_surface = TTF_RenderText_Solid(font, value, *color);

  SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
  if (!text_texture) {
    printf("Error creating texture text\n");
    exit(EXIT_FAILURE);
  }
  int text_w, text_h;
  SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);
  SDL_FreeSurface(text_surface);

  SDL_Rect *text_rect = malloc(sizeof(SDL_Rect));
  text_rect->x = dest_rect->x + dest_rect->w / 2;
  text_rect->y = dest_rect->y - 30;
  text_rect->h = text_h;
  text_rect->w = text_w;

  ret->texture = char_texture;
  ret->frame_info = frame_info;
  ret->source = source_rect;
  ret->dest = dest_rect;
  ret->x_pos = (float) init_pos.x;
  ret->state = (motions) IDLE;
  ret->speed = 0;
  ret->value = value; //value is a heap string (malloced before passing into here!)
  ret->value_text = text_texture;
  ret->value_rect = text_rect;
  return ret;
}

Object *create_character(char *file,
                        char *value, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color) {
  // Create the Frame_info array for character to pass into create_object
  Frame_info *frame_info = malloc(sizeof(Frame_info) * TEXTURE_HEIGHT);
  for (int i = 0; i < TEXTURE_HEIGHT; i++) {
    frame_info[i] = (Frame_info) {CHARACTER_NUM_FRAMES(i), CHARACTER_FRAME_TIME(i)};
  }

  return create_object(file, frame_info, (Position) {CHARACTER_INIT_X, OBJECT_RENDER_SCALE_FACTOR * (BG_ISLAND_FLOOR_LEVEL - CHARACTER_HEIGHT)}, value, renderer, font, color);
}

void free_object(Object *obj) {
  if(obj->texture) {free(obj->texture); obj->texture = NULL;}
  if(obj->frame_info) {free(obj->frame_info); obj->frame_info = NULL;}
  if(obj->source) {free(obj->source); obj->source = NULL;}
  if(obj->dest) {free(obj->dest); obj->dest = NULL;}
  if(obj->value) {free(obj->value); obj->value = NULL;}
  if(obj->value_text) {free(obj->value_text); obj->value_text = NULL;}
  if(obj->value_rect) {free(obj->value_rect); obj->value_rect = NULL;}
  free(obj);
}


int init_portals(char *file, Level *lvl, Portal **portals, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color) {
  assert(lvl);
  int rect_w = 0;
  int rect_h = 0;
  SDL_Texture *p_texture = NULL;
  if (file != NULL) {
    SDL_Surface *p_surface = IMG_Load(file);
    p_texture = SDL_CreateTextureFromSurface(renderer, p_surface);
    if (!p_texture) {
      printf("Error creating texture\n");
      return 1;
    }
    rect_w = p_surface->w * OBJECT_RENDER_SCALE_FACTOR;
    rect_h = p_surface->h * OBJECT_RENDER_SCALE_FACTOR;
    SDL_FreeSurface(p_surface);
  }

  for (int i = 0; i < lvl->portal_count; i++) {
    Portal *portal = malloc(sizeof(Portal));
    if (!portal) {
      printf("Error creating portal\n");
      return 1;
    }
    SDL_Rect *p_rect = malloc(sizeof(SDL_Rect));
    if (!p_rect) {
      printf("Error creating rectangle\n");
      return 1;
    }
    p_rect->x = (BG_ISLAND_END  - (i * (BG_ISLAND_WIDTH / (lvl->portal_count + 1))) - (rect_w)) * OBJECT_RENDER_SCALE_FACTOR;
    p_rect->y = OBJECT_RENDER_SCALE_FACTOR * (BG_ISLAND_FLOOR_LEVEL) - rect_h - (OBJECT_RENDER_SCALE_FACTOR * 6);
    p_rect->h = rect_h;
    p_rect->w = rect_w;
    assert(p_texture);
    portal->texture = p_texture;
    portal->rect = p_rect;
    printf("num:%s\n", lvl->portals[i]->num);

    SDL_Surface *text_surface_num = TTF_RenderText_Solid(font, lvl->portals[i]->num, *color);
    SDL_Surface *text_surface_val = TTF_RenderText_Solid(font, lvl->portals[i]->value, *color);

    SDL_Texture *text_texture_num = SDL_CreateTextureFromSurface(renderer, text_surface_num);
    SDL_Texture *text_texture_val = SDL_CreateTextureFromSurface(renderer, text_surface_val);

    if (!text_texture_num || !text_texture_val) {
      printf("Error creating portal texture text; ttn %p, ttv %p\n",(void *) text_surface_num, (void *) text_surface_val);
      exit(EXIT_FAILURE);
    }
    int text_num_w, text_num_h, text_val_w, text_val_h;
    SDL_QueryTexture(text_texture_num, NULL, NULL, &text_num_w, &text_num_h);
    SDL_QueryTexture(text_texture_val, NULL, NULL, &text_val_w, &text_val_h);
    SDL_FreeSurface(text_surface_num); SDL_FreeSurface(text_surface_val);

    SDL_Rect *text_rect_num = malloc(sizeof(SDL_Rect));
    text_rect_num->h = text_num_h;
    text_rect_num->w = text_num_w;
    text_rect_num->x = p_rect->x + p_rect->w / 2 - text_num_w / 2.5;
    text_rect_num->y = p_rect->y - 30;

    SDL_Rect *text_rect_val = malloc(sizeof(SDL_Rect));
    text_rect_val->h = text_val_h;
    text_rect_val->w = text_val_w;
    text_rect_val->x = p_rect->x + p_rect->w / 2 - text_num_w / 2.5;
    text_rect_val->y = text_rect_num->y - text_num_h - 10;

    portal->num_rect = text_rect_num;
    portal->value_rect = text_rect_val;
    portal->num_text = text_texture_num;
    portal->value_text = text_texture_val;

    portal->info = lvl->portals[i];
    *(portals + i) = portal;
  }

  free(lvl->portals);
  lvl->portals = NULL;
  return 0;
}

void free_portals(Level *lvl, Portal **portals) {
  if (!lvl->portal_count) return;
  for (int i = 0; i < lvl->portal_count; i++) {
    if (portals[i]->rect) { 
      free(portals[i]->rect);
      portals[i]->rect = NULL;
    }
    free(portals[i]);
    portals[i] = NULL;
  }
  free(portals);
}

Object *create_egg(char *file, Position init_pos,
                        char *value, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color) {
  Frame_info *frame_info = malloc(sizeof(Frame_info) * TEXTURE_HEIGHT);
  for (int i = 0; i < TEXTURE_HEIGHT; i++) {
    frame_info[i] = (Frame_info) {EGG_NUM_FRAMES, EGG_FRAME_TIME};
  }

  return create_object(file, frame_info, init_pos, value, renderer, font, color);
}

void init_eggs(char *file, Level *lvl, Object **eggs, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color) {
  assert(lvl);
  for (int i = 0; i < lvl->imms_count; i++) {
    int egg_x = OBJECT_RENDER_SCALE_FACTOR * (BG_ISLAND_START + (i + 1) * EGG_SPACING);
    eggs[i] = create_egg(file,
                         (Position) {egg_x,
                                     OBJECT_RENDER_SCALE_FACTOR * (BG_ISLAND_FLOOR_LEVEL - EGG_HEIGHT)},
                         lvl->imms[i],
                         renderer, font, color);
    eggs[i]->value_rect->w = eggs[i]->value_rect->w * EGG_TEXT_SCALE_FACTOR;
    eggs[i]->value_rect->h = eggs[i]->value_rect->h * EGG_TEXT_SCALE_FACTOR;
    eggs[i]->value_rect->x = egg_x + (OBJECT_RENDER_SCALE_FACTOR * EGG_WIDTH / 2) - eggs[i]->value_rect->w / 2;
    eggs[i]->value_rect->y = OBJECT_RENDER_SCALE_FACTOR * (BG_ISLAND_FLOOR_LEVEL + EGG_HEIGHT) - 2 * EGG_LABEL_OFFSET_1;
  }
}

void free_eggs(Level *lvl, Object **eggs) {
  if (!lvl->imms_count) return;
  for (int i = 0; i < lvl->imms_count; i++) {
    free_object(eggs[i]);
  }
  free(eggs);
}

Object *create_bird(char *file, Position init_pos,
                        char *value, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color) {
  Frame_info *frame_info = malloc(sizeof(Frame_info) * TEXTURE_HEIGHT);
  for (int i = 0; i < TEXTURE_HEIGHT; i++) {
    frame_info[i] = (Frame_info) {BIRD_NUM_FRAMES(i), BIRD_FRAME_TIME};
  }

  return create_object(file, frame_info, init_pos, value, renderer, font, color);
}

void init_birds(char *file, Level *lvl, Object **birds, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color) {
  assert(lvl);
  for (int i = 0; i < NUM_BIRDS; i++) {
    birds[i] = create_bird(file,
                         (Position) {CHARACTER_INIT_X - BIRD_CHARACTER_OFFSET  - ((i + 1) * (OBJECT_RENDER_SCALE_FACTOR * (BIRD_WIDTH + 10))), OBJECT_RENDER_SCALE_FACTOR * (BG_ISLAND_FLOOR_LEVEL - CHARACTER_HEIGHT) - 60},
                         " ",
                         renderer,
                         font,
                         color);
    birds[i]->value_rect->x = birds[i]->dest->x + birds[i]->dest->w / 4;
  }
}

Object *create_text_object(char *value, SDL_Renderer *renderer, Position init_pos, TTF_Font *font, SDL_Color *color) {
  Object *ret = malloc(sizeof(Object));

  SDL_Surface *surface = TTF_RenderText_Solid(font, value, *color);

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) {
    printf("Error creating texture text\n");
    exit(EXIT_FAILURE);
  }
  int text_w, text_h;
  SDL_QueryTexture(texture, NULL, NULL, &text_w, &text_h);
  SDL_FreeSurface(surface);

  SDL_Rect *rect = malloc(sizeof(SDL_Rect));
  rect->x = init_pos.x;
  rect->y = init_pos.y;
  rect->h = text_h;
  rect->w = text_w;

  ret->texture = texture;
  ret->frame_info = &((Frame_info) {1, 1});
  ret->dest = rect;
  ret->value = value;

  return ret;
}

void free_text_object(Object *obj) {
  free(obj->dest);
  free(obj);
}

void free_birds(Object **birds) {
  for (int i = 0; i < NUM_BIRDS; i++) {
    free_object(birds[i]);
  }
}

Stage *create_stage(Portal **portal_ptrs, Object **egg_ptrs, Object **bird_ptrs, Level *lvl) {
  Stage *stage = malloc(sizeof(Stage));
  stage->portal_ptrs = portal_ptrs;
  stage->portal_count = lvl->portal_count;
  stage->egg_ptrs = egg_ptrs;
  stage->egg_count = lvl->imms_count;
  stage->bird_ptrs = bird_ptrs;
  return stage;
}
