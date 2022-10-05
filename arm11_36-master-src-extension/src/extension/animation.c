#include <stdio.h>
#include <SDL2/SDL.h>

#include "animation.h"
#include "sprites.h"

void change_value(Object *obj, SDL_Renderer *renderer, char *new_value, TTF_Font *font, SDL_Color *color) {
        SDL_Surface *text_surf = TTF_RenderText_Solid(font, new_value, *color);
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surf);
        obj->value_text = text_texture;
        obj->value = new_value;
        SDL_QueryTexture(text_texture, NULL, NULL, &obj->value_rect->w, &obj->value_rect->h);
        SDL_FreeSurface(text_surf);
}

int move(Object *obj, int target_x, int *current_frame, SDL_Renderer *renderer, bool update_value, TTF_Font *font, SDL_Color *color, Object **birds, char *new_value) {
    int res = 0;
    if (*current_frame == obj->frame_info[obj->state].frame_time) {
        if (abs(obj->dest->x - target_x) < 10) {
            obj->speed = 0;
            obj->state = IDLE;
            res = 1;
        } else {
            if (obj->dest->x > target_x) {
                obj->state = MOVE_LEFT;
                obj->speed  = -SPEED;
            } else {
                obj->state = MOVE_RIGHT;
                obj->speed  = SPEED;
            }
        }    
        // update positions
        obj->x_pos += obj->speed / 60;
        obj->dest->x = (int) obj->x_pos;
    
        if (birds != NULL) {
            if (update_value) {
                change_value(birds[0], renderer, new_value, font, color);
            }
            for (int i = 0; i < NUM_BIRDS; i++) {              
                birds[i]->speed = obj->speed;
                birds[i]->x_pos += birds[i]->speed / 60;
                birds[i]->dest->x = (int) obj->x_pos - BIRD_CHARACTER_OFFSET  - ((i + 1) * (OBJECT_RENDER_SCALE_FACTOR * (BIRD_WIDTH + 10)));
                birds[i]->value_rect->x = (int) birds[i]->x_pos + (birds[i]->dest->w / 3);
                change_value(birds[i], renderer, birds[i]->value, font, color);
            }
        }

        *current_frame = 0;
        SDL_Rect *src = obj->source;
        src->y = obj->state * src->h;
        src->x += src->w;
        if (src->x >= obj->frame_info[obj->state].num_frames * src->w) {
            src->x = 0;
        }
        if (update_value) {
            for (int i = 0; i < NUM_BIRDS; i++) {
                SDL_Rect *bird = birds[i]->source;
                src->y = 2 * src->h;
                bird->x += bird->w;
                if (bird->x >= 4* bird->w) {
                    bird->x = 0;
                }
            }
        } else {
            for (int i = 0; i < NUM_BIRDS; i++) {
                SDL_Rect *bird = birds[i]->source;
                bird->x += bird->w;
                if (bird->x >= 2 * bird->w) {
                    bird->x = 0;
                }
            }
        }
    }
    return res;
}


// Returns 1 when animation is still running, 0 otherwise
int egg_hatch_animation(Object *egg, int *frame) {
    if (*frame == egg->frame_info[IDLE].frame_time) {
        if (egg->source->x >= egg->source->w * egg->frame_info[IDLE].num_frames) {
            return 0;
        }
        egg->source->x += egg->source->w;
    }
    return 1;
}