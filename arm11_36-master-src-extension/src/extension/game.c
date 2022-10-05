#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "globals.h"
#include "parse_level_info.h"
#include "sprites.h"
#include "parse_player_input.h"
#include "game_structs.h"
#include "animation.h"
#include "player_input.h"

#define DISPLAY_NUM 0

#define BG_IMG_PATH "resources/background.png"
#define BG_SUCC_IMG_PATH "resources/success-background.png"
#define PORTAL_IMG_PATH "resources/portal.png"
#define EGG_IMG_PATH "resources/hatched-egg.png"
#define BIRD_IMG_PATH "resources/bird.png"
#define EXIT_IMG_PATH "resources/exit-portal.png"
#define INPUT_BAR_IMG_PATH "resources/input_bar.png"
#define CHAR_JUMP_IMG_PATH "resources/raspberry_jump.png"

#define CHARACTER_TEXTURE_COUNT 3
#define CHARACTER_TEXTURE_PATH "resources/character.png"

#define BIRD_COUNT 3
#define BIRD_CHARACTER_SPACE 35
#define SPACE_BETWEEN_BIRDS 25

#define EXIT_X 248
#define EXIT_Y 14

#define WAIT_NUM 400


int main(int argc, char **argv) {

    // Object **objects = malloc(sizeof(Object*));

    if (TTF_Init() < 0) {
        printf("error creating text: %s\n", SDL_GetError());
        SDL_Quit();
	    return 1;
    }

    // TTF
    SDL_Color color = { 144, 77, 255, 255 };
    SDL_Color pc_color = { 80, 80, 80, 255 };
    SDL_Color egg_val_color = { 210, 255, 195, 255 };
    SDL_Color goal_val_color = { 140, 255, 0, 255 };


    TTF_Font *font = TTF_OpenFont("resources/pixelmix.ttf", 20);

    Level *lvl = parse_level_info("add04");
    if (lvl == NULL) { printf("Error creating level!\n"); return EXIT_FAILURE; }
    assert(lvl);
    
    printf("Portals:\n");
    for (int i=0; i<lvl->portal_count; i++) {
        printf("portal %d: %s\n", atoi(lvl->portals[i]->num + 1), lvl->portals[i]->ans);
    }

    printf("imms:\n");
    for (int i=0; i<lvl->imms_count; i++) {
        printf("%s\n", lvl->imms[i]);
    }


    // Initialise SDL with video, audio, timer
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    printf("Initialised SDL!\n");

    // Get display info
    SDL_DisplayMode *display_mode = malloc(sizeof(SDL_DisplayMode));
    if (SDL_GetCurrentDisplayMode(DISPLAY_NUM, display_mode)) {
        printf("Failed to get display mode\n");
        exit(EXIT_FAILURE);
    }

   //  Create game window
    SDL_Window *window = SDL_CreateWindow(
                                "Raspberry Matcha's Assembly Adventure",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                WINDOW_WIDTH,
                                WINDOW_HEIGHT,
                                0);
    if (window == NULL) {
        printf("Failed to create window\n");
        exit(EXIT_FAILURE);
    }

    // Get window dimensions
    int screen_w, screen_h;
    SDL_GetWindowSize(window, &screen_w, &screen_h);

    // Initialise image flags
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        printf("Error initialising image\n");
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
      printf("Error creating renderer: %s\n", SDL_GetError());
      SDL_DestroyWindow(window);
      SDL_Quit();
      exit(EXIT_FAILURE);
    }

    PlayerInput *player_input = create_player_input(INPUT_BAR_IMG_PATH, renderer, font, &color);
    // Create backgrounds
    Background *bg = create_bg(BG_IMG_PATH, renderer);
    Background *succ_bg = create_bg(BG_SUCC_IMG_PATH, renderer);

    // Create jump sprite
    SDL_Surface *jump_surf = IMG_Load(CHAR_JUMP_IMG_PATH);
    SDL_Texture *jump_tex = SDL_CreateTextureFromSurface(renderer, jump_surf);
    int jump_w, jump_h;
    SDL_QueryTexture(jump_tex, NULL, NULL, &jump_w, &jump_h);
    SDL_FreeSurface(jump_surf);

    // Create portals
    Portal **portals = malloc(sizeof(Portal *) * (lvl->portal_count));
    if (init_portals(PORTAL_IMG_PATH, lvl, portals, renderer, font, &color)) {
        printf("Failed to initialise portals\n");
        exit(EXIT_FAILURE);
    }

    // Create eggs
    Object **eggs = malloc(sizeof(Object) * lvl->imms_count);
    init_eggs(EGG_IMG_PATH, lvl, eggs, renderer, font, &egg_val_color);

    // Create birds
    Object **birds = malloc(sizeof(Object) * NUM_BIRDS);
    init_birds(BIRD_IMG_PATH, lvl, birds, renderer, font, &pc_color);

    // Initialise the character
    Object *character = create_character(CHARACTER_TEXTURE_PATH,
                                 " ",
                                 renderer,
                                 font, 
                                 &color
                                );
    char pc[512];
    sprintf(pc, "%ld", strtol(lvl->pc, NULL, 0));
    Object *pc_value = create_text_object(pc, renderer,  (Position) {PC_MIDDLE_X, PC_MIDDLE_Y}, font, &pc_color);
    pc_value->dest->w = pc_value->dest->w * PC_SCALE_FACTOR;
    pc_value->dest->h = pc_value->dest->h * PC_SCALE_FACTOR;

    Object *goal_value = create_text_object("76", renderer, (Position) {GOAL_MIDDLE_X - GOAL_OFFSET, GOAL_MIDDLE_Y}, font, &goal_val_color);
    goal_value->dest->w = goal_value->dest->w * GOAL_SCALE_FACTOR;
    goal_value->dest->h = goal_value->dest->h * GOAL_SCALE_FACTOR;

     // set to 1 when window close button is pressed
    int close_requested = 0;

    // dummy values for parse_input output
    int frame = 0;
    int curr_step = 0;
    int is_idle = 0;
    int wait_count = 0;
    int playing_egg_hatch = 0;
    int has_found_egg = 0;
    Object *egg_found = NULL;

    SDL_StartTextInput();

    Stage *stage = create_stage(portals, eggs, birds, lvl);
    AnimatePanel *panel = NULL;
    int instr_cnt = 0;
    int change_bg = 0;
    int all_correct = 0;
    int to_jump = 0;
    int missing = 0;
    // animation loop
    while (!close_requested){
        // The rerender text flag

        // process events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
                
            if (event.type == SDL_QUIT)
            {
                close_requested = 1;
            } 
            else if( event.type == SDL_KEYDOWN )
            {
                //Handle backspace
                if( event.key.keysym.sym == SDLK_BACKSPACE && strlen(player_input->str) > 0 )
                {
                    //lop off character
                    player_input->str[strlen(player_input->str) - 1] = '\0';
                    update_player_input_value(player_input, renderer, font, &color, 1, 0);
                }
                //Handle copy
                else if( event.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
                {
                    SDL_SetClipboardText( player_input->str );
                }
                //Handle paste
                else if( event.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
                {
                    player_input->str = SDL_GetClipboardText();
                    update_player_input_value(player_input, renderer, font, &color, 2, strlen(SDL_GetClipboardText()));
                }
                else if ( event.key.keysym.sym == SDLK_RETURN) {
                    panel = parse_player_input(player_input->str, stage);
                    // Panel not NULL when player executes an instr.
                    if (panel != NULL) {
                        instr_cnt++;
                    }

                    // Check if PC exceeded
                    if (strtol(lvl->pc, NULL, 0) / 4 - 2 < instr_cnt) {
                        printf("You exceeded PC - game over!\n");
                        return(EXIT_FAILURE);
                    }


                    strcpy(player_input->str, "");
                    update_player_input_value(player_input, renderer, font, &color, 3, 0);
                    
                    // PC still not exceeded - check if player is now correct

                }
            }
            //Special text input event
            else if( event.type == SDL_TEXTINPUT )
            {
                //Not copy or pasting
                if( !( SDL_GetModState() & KMOD_CTRL && ( event.text.text[ 0 ] == 'c' || event.text.text[ 0 ] == 'C' || event.text.text[ 0 ] == 'v' || event.text.text[ 0 ] == 'V' ) ) )
                {
                    //Append character
                    strcat(player_input->str, event.text.text);
                    update_player_input_value(player_input, renderer, font, &color, 0, 0);
                }
            }
        }
        if (!change_bg) {
            if (panel != NULL) {
                if (is_idle) {
                    if (WAIT_NUM == wait_count && curr_step != panel->steps) {
                        if (curr_step < panel->steps - 1) {
                            curr_step++;
                            move(character, character->x_pos, &frame, renderer, false, font, &color, stage->bird_ptrs, panel->output);
                        } else {
                            move(character, character->x_pos, &frame, renderer, true, font, &color, stage->bird_ptrs, panel->output);
                            panel = NULL;
                            curr_step = 0;
                            printf("%d",all_correct);
                            if (all_correct) change_bg = 1;
                            for (int p = 0; p < stage->portal_count; p++) {
                                Portal *portal = stage->portal_ptrs[p];
                                char * new_value = portal->info->value;
                                SDL_Surface *text_surf = TTF_RenderText_Solid(font, new_value, color);
                                SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surf);
                                portal->value_text = text_texture;
                                SDL_QueryTexture(text_texture, NULL, NULL, &portal->value_rect->w, &portal->value_rect->h);
                                SDL_FreeSurface(text_surf);
                            }
                        }
                    is_idle = 0;
                    wait_count = 0;
                    } else {
                        if (!playing_egg_hatch) wait_count++;
                        else {
                            if (!has_found_egg) {
                                char *egg_value = stage->bird_ptrs[curr_step]->value;
                                if (!egg_value) {printf("No such egg found!\n"); continue;}
                                for (int e=0;e < stage->egg_count;e++) {
                                    if (!strcmp(egg_value, stage->egg_ptrs[e]->value)) {
                                        egg_found = stage->egg_ptrs[e];
                                        has_found_egg = 1;
                                        break;
                                    }
                                }
                            }
                            if (!egg_found) {printf("No such egg found!\n"); continue;}
                            assert(egg_found);
                            playing_egg_hatch = egg_hatch_animation(egg_found, &frame);
                            if (!playing_egg_hatch) {
                                stage->egg_ptrs = del_egg_in_array(egg_found->value, stage->egg_ptrs, stage->egg_count);
                                stage->egg_count -= 1;
                                egg_found = NULL;
                                has_found_egg = 0;
                            }
                        }
                        move(character, character->x_pos, &frame, renderer, false, font, &color, stage->bird_ptrs, panel->output);
                    }
                } else {
                    is_idle = move(character, panel->targets_x[curr_step], &frame, renderer, false, font, &color, stage->bird_ptrs, panel->output);
                    if (is_idle && panel->targets_type[curr_step] == EGG) {
                        playing_egg_hatch = 1;
                    }
                }
            } else {
                move(character, character->x_pos, &frame, renderer, 0, font, &color, stage->bird_ptrs, 0);
            }
        }

        for (int i = 0; i < lvl->portal_count; i++) {
            if (strcmp((stage->portal_ptrs[i])->info->ans, (stage->portal_ptrs[i])->info->value)) {
               break;
            } else if (i == lvl->portal_count - 1) {
                all_correct = 1;
            }
        }

        if (all_correct) {
            if ((strtol(lvl->pc, NULL, 0) / 4) - 2 != instr_cnt) {
                printf("You almost got it - try to match PC this time!\n");
                SDL_Delay(5000);
                return(EXIT_SUCCESS);
            } else {
                printf("Congratulations, you got it!\n");
            }
        }

        if (!missing && change_bg) {
            if (to_jump || move(character, 800, &frame, renderer, false, font, &color, birds, 0)) {
                to_jump = 1;
                character->texture = jump_tex;
                SDL_Rect *src = character->source;
                src->w = jump_w / 12;
                src->h = jump_h;
                src->x += src->w;
                if (src->x >= 12 * src->w) {
                    missing = 1;
                }
                character->x_pos += (300 / 60);
                character->dest->x = (int) character->x_pos;
            }
            SDL_Delay(5);
        }
                
        frame++;

        // clear the window
        SDL_RenderClear(renderer);
        if (change_bg) {
            SDL_RenderCopy(renderer, succ_bg->texture, NULL, succ_bg->rect);

        } else {
            SDL_RenderCopy(renderer, bg->texture, NULL, bg->rect);
            
        }
        SDL_RenderCopy(renderer, player_input->texture, NULL, player_input->rect);
        SDL_RenderCopy(renderer, player_input->input_texture, player_input->input_src_rect, player_input->input_rect);
    
        // render portals
        for (int i = 0; i < lvl->portal_count; i++) {
            SDL_RenderCopy(renderer, portals[i]->texture, NULL, portals[i]->rect);
            SDL_RenderCopy(renderer, portals[i]->value_text, NULL, portals[i]->value_rect);
            SDL_RenderCopy(renderer, portals[i]->num_text, NULL, portals[i]->num_rect);
        }

        // draw the image to the window
        if (!missing) {
            SDL_RenderCopy(renderer, character->texture, character->source, character->dest);
            SDL_RenderCopy(renderer, character->value_text, NULL, character->value_rect);
        }
        // render eggs and their text values
        for (int i = 0; i < stage->egg_count; i++) {
            SDL_RenderCopy(renderer, stage->egg_ptrs[i]->texture, stage->egg_ptrs[i]->source, stage->egg_ptrs[i]->dest);
            SDL_RenderCopy(renderer, stage->egg_ptrs[i]->value_text, NULL, stage->egg_ptrs[i]->value_rect);
        }
        
        // render birds
        for (int i = 0; i < curr_step; i++) {
            SDL_RenderCopy(renderer, birds[i]->texture, birds[i]->source, birds[i]->dest);
            SDL_RenderCopy(renderer, birds[i]->value_text, NULL, birds[i]->value_rect);
        }

        // render PC and goal values
         if (!change_bg) {
            SDL_RenderCopy(renderer, pc_value->texture, NULL, pc_value->dest);
            for (int i = 0; i < lvl->portal_count; i++) {
                char *goal_str = calloc(sizeof(char), strlen(portals[i]->info->ans) + sizeof("r0: "));
                sprintf(goal_str, "r%d: %s", i + 1, portals[i]->info->ans);
                Object *goal_text = create_text_object(goal_str, renderer, (Position) {GOAL_MIDDLE_X - 65, GOAL_MIDDLE_Y}, font, &goal_val_color);
                int h, w;
                goal_text->dest->h = goal_text->dest->h * GOAL_SCALE_FACTOR;
                goal_text->dest->w = goal_text->dest->w * GOAL_SCALE_FACTOR;
                SDL_QueryTexture(goal_text->texture, NULL, NULL, &w, &h);
                goal_text->dest->y += h * i;
                SDL_RenderCopy(renderer, goal_text->texture, NULL, goal_text->dest);
            }
        }
        
        SDL_RenderPresent(renderer);

        frame++;
    }

    SDL_StopTextInput();
    TTF_CloseFont(font);
    TTF_Quit();
    
    // clean up resources before exiting
    // clean portals[i] resources before exiting
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    for (int i = 0; i < lvl->portal_count; i++) {
        free(portals[i]->num_rect);
        free(portals[i]->value_rect);
        free(portals[i]->rect);
        SDL_DestroyTexture(portals[i]->num_text);
        SDL_DestroyTexture(portals[i]->value_text);
        free(portals[i]);
    }
    free(portals);

    for (int i = 0; i < stage->portal_count; i++) {
        free(stage->portal_ptrs[i]);
    }
    free(stage->portal_ptrs);
    free(stage);
    free(player_input->rect);
    SDL_DestroyTexture(player_input->texture);
    SDL_DestroyTexture(player_input->input_texture);
    free(player_input->str);
    free(player_input->input_rect);
    free(player_input->input_src_rect);

    free(player_input);
    free(lvl->pc);
    for (int i = 0; i < lvl->portal_count; i++) {
        free(portals[i]);
    }
    free(lvl->portals);
    free(lvl);
    free_text_object(pc_value);
    free_background(bg);
    free_background(succ_bg);
    free_object(character);
    free(character);
    free_portals(lvl, portals);
    free_birds(birds);
    free(birds);
    free_eggs(lvl, eggs);
    free_level(lvl);
    free(display_mode);
}

