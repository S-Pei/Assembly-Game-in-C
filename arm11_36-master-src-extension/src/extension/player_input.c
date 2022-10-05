#include <stdlib.h>

#include "player_input.h"
#include "sprites.h"

#define INPUT_BAR_X_OFFSET 50
#define INPUT_BAR_Y_OFFSET 20
#define INPUT_TEXT_X_PADDING 20
#define INPUT_TEXT_Y_PADDING 10
#define CHAR_WIDTH 16

PlayerInput *create_player_input(char *file, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color) {
  SDL_Surface *input_surface = IMG_Load(file);
  if (!input_surface) { printf("Error creating input surface!\n"); exit(EXIT_FAILURE); }
  SDL_Texture *input_texture = SDL_CreateTextureFromSurface(renderer, input_surface);
  if (!input_texture) { printf("Error creating input texture!\n"); exit(EXIT_FAILURE); }
  SDL_FreeSurface(input_surface);
  int tex_w, tex_h;
  SDL_QueryTexture(input_texture, NULL, NULL, &tex_w, &tex_h);

  SDL_Rect *input_rect = malloc(sizeof(SDL_Rect));
  input_rect->h = tex_h * OBJECT_RENDER_SCALE_FACTOR;
  input_rect->w = tex_w * OBJECT_RENDER_SCALE_FACTOR;
  input_rect->x = (WINDOW_WIDTH - input_rect->w) - INPUT_BAR_X_OFFSET;
  input_rect->y = (WINDOW_HEIGHT - input_rect->h)- INPUT_BAR_Y_OFFSET;

  SDL_Surface *text_surface = TTF_RenderText_Solid(font, " ", *color);

  SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
  if (!text_texture) {
    printf("Error creating input texture\n");
    exit(EXIT_FAILURE);
  }
  int text_w, text_h;
  SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);
  SDL_FreeSurface(text_surface);

  SDL_Rect *text_rect = malloc(sizeof(SDL_Rect));
  text_rect->x = input_rect->x + INPUT_TEXT_X_PADDING;
  text_rect->y = input_rect->y + INPUT_TEXT_Y_PADDING;
  text_rect->h = input_rect->h - 2 * INPUT_TEXT_Y_PADDING;
  text_rect->w = INPUT_TEXT_X_PADDING;

  SDL_Rect *text_src_rect = malloc(sizeof(SDL_Rect));
  text_src_rect->x = 0;
  text_src_rect->y = 0;
  text_src_rect->h = text_rect->h;
  text_src_rect->w = 0;


  PlayerInput *player_input = malloc(sizeof(PlayerInput));
  player_input->texture = input_texture;
  player_input->rect = input_rect;
  player_input->str = malloc(MAX_LINE_INPUT * sizeof(char));
  strcpy(player_input->str, "");
  player_input->input_texture = text_texture;
  player_input->input_rect = text_rect;
  player_input->input_src_rect = text_src_rect;
  return player_input;
}

void update_player_input_value(PlayerInput *player_input, SDL_Renderer *renderer, TTF_Font *font, SDL_Color *color, int mode, int size) {
  if (strlen(player_input->str) > 0) {
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, player_input->str, *color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (!text_texture) {
      printf("Error creating input texture to update\n");
      exit(EXIT_FAILURE);
    }
    player_input->input_texture = text_texture;
  } else {
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, " ", *color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (!text_texture) {
      printf("Error creating input texture to update (empty string case)\n");
      exit(EXIT_FAILURE);
    }
    player_input->input_texture = text_texture;
  }

  if (mode == 0) {player_input->input_src_rect->w += CHAR_WIDTH;}
  else if (mode == 1) {
    if (strlen(player_input->str) > 0) player_input->input_src_rect->w -= CHAR_WIDTH;
    else player_input->input_src_rect->w = 0;
  }
  else if (mode == 2) {player_input->input_src_rect->w += size * CHAR_WIDTH;}
  else if (mode == 3) {player_input->input_src_rect->w = 0;}

  player_input->input_rect->w = player_input->input_src_rect->w;
}