#include <SDL2/SDL.h>
#include <stdbool.h>

#include "../emulate_files/state.h"

#ifndef GLOBALS_H
#define GLOBALS_H

#define WINDOW_WIDTH (1280)
#define WINDOW_HEIGHT (896)
// speed in pixels/second
#define SPEED (1000)

#define MAX_ARGS 5
#define NUM_BIRDS 3
#define MAX_IMMS 5

enum read_assembly_line_mode { LEVEL_INFO, PLAYER_INPUT };
enum target_types { PORTAL, EGG };

typedef struct Portal_info {
    char *num;
    char *value;
    char *ans;
} Portal_info;

typedef struct Level {
    Portal_info **portals;
    int portal_count;
    char **imms;
    int imms_count;
    char *pc;
} Level;

char *to_hex(int num);

Level *create_level();
void free_level();

char *remove_lead_spaces(char *str);

bool read_assembly_line(const int mode, const char* str, int *rs, int *rs_count, int *imms, int *imms_count, char *bird_strings[NUM_BIRDS]);

int32_t get_num(char const *str);

char *get_found(char *str, char *found);

#endif
