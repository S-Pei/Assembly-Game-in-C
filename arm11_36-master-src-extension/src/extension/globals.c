#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "globals.h"

char *to_hex(int num) {
  char *hex_str = malloc(11 * sizeof(char));
  strcpy(hex_str,"0x");
  char buffer[9];
  sprintf(buffer, "%x", num);
  strcat(hex_str, buffer);
  return hex_str;
}

Level *create_level(int portal_count, int imms_count, 
const int *rs, const int32_t *imms, const int32_t *ans, char *pc) {
    Level *lvl = (Level *) malloc(sizeof(Level));
    lvl->portal_count = portal_count;
    lvl->imms_count = imms_count;
    lvl->portals = malloc(portal_count * sizeof(Portal_info *));
    for (int i=0; i<portal_count; i++) {
        Portal_info *new_portal = (Portal_info *) malloc(sizeof(Portal_info));
        char *r_ptr = malloc(4 * sizeof(char));
        sprintf(r_ptr, "r%d", rs[i]);
        new_portal->num = r_ptr;
        char *zero = malloc(10 * sizeof(char));
        strcpy(zero, "0x0");
        new_portal->value = zero;
        new_portal->ans = to_hex(ans[i]);
        lvl->portals[i] = new_portal;
    }
    lvl->imms = malloc(imms_count * sizeof(char *));
    for (int j=0; j<imms_count; j++) {
        lvl->imms[j] = to_hex(imms[j]);
    }
    lvl->pc = pc;
    printf("PC value: %s\n", lvl->pc);
    return lvl;
}

void free_level(Level *lvl) {
    assert(lvl);
    if(lvl->portals) {
        for (int i=0; i<lvl->portal_count; i++) {
            free(lvl->portals[i]->value);
            free(lvl->portals[i]->ans);
            free(lvl->portals[i]);
        }
        free(lvl->portals);
    }
    for (int i=0; i < lvl->imms_count; i++) {
        free(lvl->imms[i]);
    }
    free(lvl->imms);
    free(lvl);
}

char *remove_lead_spaces(char *str) {
    for (int i=0; i<strlen(str); i++) {
        if (str[i] != ' ') { return str + i; }
    }
    return str;
}

int32_t get_num(char const *str) {
    char hold[strlen(str)];
    strcpy(hold, str);
    for (int i=0; i<strlen(hold); i++) {
        if (hold[i] == ' ' || hold[i] == ',' || hold[i] == ':') {
            hold[i] = '\0';
            break;
        }
    }
    return (int32_t) strtol(hold, NULL, 0);
}

char *get_found(char *str, char *found) {
    for (int i=0; i<strlen(str); i++) {
        if (str[i] == 'r' || str[i] == '#' || str[i] == '=') {
            *found = str[i];
            return str + i + 1;
        }
    }
    printf("Invalid argument!\n");
    return str;
}

bool read_assembly_line(const int mode, const char* str, int *rs, int *rs_count, int *imms, int *imms_count, char *bird_strings[NUM_BIRDS]) {
    char str_copy[strlen(str)];
    strcpy(str_copy, str); 
    char *read = strchr(str_copy, ' ');
    if (read == NULL) { printf("Invalid assembler code!\n"); }
    char found;
    int count = 1;
    while (read && count <= MAX_ARGS) {
        read = remove_lead_spaces(read);
        if (count == 4) {
            read = strchr(read, ' ');
            if (read == NULL) { printf("Invalid assembly code format!\n"); return false; }
        }
        read = get_found(read, &found);
        int32_t r_num = get_num(read);
        if (found == 'r') {
            if (mode == LEVEL_INFO) {
                assert(rs);
                assert(rs_count);
                bool in_rs = false;
                for (int i=0; i < *rs_count; i++) {
                    if (rs[i] == r_num) { in_rs = true; break; }
                }
                if (!in_rs) { rs[*rs_count] = r_num; *rs_count += 1;}
            } else if (mode == PLAYER_INPUT) {
                assert(bird_strings);
                char *r_buffer = malloc(4 * sizeof(char));
                char num_string_buffer[3];
                sprintf(num_string_buffer, "r%d", r_num);
                strcpy(r_buffer, num_string_buffer);
                bird_strings[count - 1] = r_buffer;
            }
        } else if (found == '#' || found == '=') {
            if (mode == LEVEL_INFO) {
                assert(imms);
                assert(imms_count);
                imms[*imms_count] = r_num;
                *imms_count += 1;
            } else if (mode == PLAYER_INPUT) {
                assert(bird_strings);
                bird_strings[count - 1] = to_hex(r_num);;
            }
        }
        read = strchr(read, ',');
        count ++;
    }
    return true;
}
