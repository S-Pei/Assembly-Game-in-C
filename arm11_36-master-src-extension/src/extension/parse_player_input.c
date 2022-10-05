#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "parse_player_input.h"

int32_t finished_instrs[MAX_BYTES / 4];
int count = 0;
int symbol_count = 0;
int jump_count = 0;
char *unresolved_labels[MAX_BYTES / 4];
pending_instr *pending_instrs[MAX_BYTES / 4];
int pending_count = 0;

struct state state = {NULL, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }};

op_num op_step_pairs[] = {
  {"add", 3},
  {"sub", 3},
  {"rsb", 3},
  {"and", 3},
  {"eor", 3},
  {"orr", 3},
  {"mov", 2},
  {"mul", 3},
  {"mla", 4},
  {"ldr", 2},
  {"str", 2}
};

Object **del_egg_in_array(char *value, Object **objs, int size) {
  Object **res = calloc(size - 1, sizeof(Object *));
  int insert_index = 0;
  bool has_met = false;
  for (int i = 0; i < size; i++) {
    insert_index = has_met ? i-1 : i;
    if (strcmp(value, objs[i]->value) == 0) { 
      has_met = true;
      free_object(objs[i]);
    } else {
      res[insert_index] = objs[i];
    }
  }
  return res;
}

bool get_steps(const char *str, int *num_steps) {
  char input_copy[strlen(str)];
  char *buffer = strcpy(input_copy, str);
  char *op = strsep(&buffer, " ");
  int num_op_step_pairs = sizeof(op_step_pairs) / sizeof(op_num);
  for (int i=0; i < num_op_step_pairs; i++) {
    if (strcmp(op, op_step_pairs[i].op) == 0) { *num_steps = op_step_pairs[i].num; break; }
    else if (i == num_op_step_pairs - 1) { 
      printf("Not a valid operation, cannot get number of steps!\n"); 
      return false; 
    }
  }
  return true;
}

bool get_bird_and_target_values(const char *str, const int num_steps, int *output_reg, Stage *stage, int targets_x[num_steps], int targets_type[num_steps]) {
  char input_copy[strlen(str)];
  char *bird_strings[NUM_BIRDS] = {"", "", ""};
  strcpy(input_copy, str);
  if (!(read_assembly_line(PLAYER_INPUT, input_copy, NULL, NULL, NULL, NULL, bird_strings))) {
    printf("Error reading assembly line!\n");
    return false;
  }
  for (int i=0; i<num_steps; i++) {
    if (*bird_strings[i] == '\0') { printf("Bird string is null!\n"); return false; }  
    else if (i == 0) {
      if (*bird_strings[i] != 'r') { printf("Invalid destination in assembly code!\n"); return false; }
      targets_type[num_steps - 1] = PORTAL;
      *output_reg = atoi(bird_strings[i] + 1);
      for (int j = 0; j < stage->portal_count; j++) {
        if (!(strcmp(bird_strings[i], stage->portal_ptrs[j]->info->num))) { 
          targets_x[num_steps-1] = stage->portal_ptrs[j]->rect->x;
          break;
        } else if (j == stage->portal_count - 1) {
          // Can print a message to the player about input error here (no such register)
          printf("Invalid destination register input!\n");
          return false;
        }
      }
      continue;
    }

    if (*bird_strings[i] == 'r') {
      targets_type[i - 1] = PORTAL;
      stage->bird_ptrs[i-1]->value = to_hex(state.regs[atoi(bird_strings[i] + 1)]);
      for (int j = 0; j < stage->portal_count; j++) {
        if (!(strcmp(stage->portal_ptrs[j]->info->num, bird_strings[i]))) { 
          targets_x[i-1] = stage->portal_ptrs[j]->rect->x;
          break;
        } else if (j == stage->portal_count - 1) {
          // Can print a message to the player about input error here (no such register)
          printf("Invalid register input!\n");
          return false;
        }
      }
    } else {
      targets_type[i - 1] = EGG;
      char *hex_str = to_hex(strtol(bird_strings[i], NULL, 0));
      stage->bird_ptrs[i-1]->value = hex_str;
      for (int j = 0; j < stage->egg_count; j++) {
        if (strcmp(stage->egg_ptrs[j]->value, hex_str) == 0) {
          targets_x[i-1] = stage->egg_ptrs[j]->dest->x;
          break;
        } else if (j == stage->egg_count - 1) {
          // Can print a message to the player about input error here (no such immediate value)
          printf("Invalid immediate value input!\n");
          return false;
        }
      }
    }
  }
  return true;
}


AnimatePanel *parse_player_input(const char *input, Stage *stage) {
  assert(input);
  char input_store[strlen(input)];
  strcpy(input_store, input);
  int32_t pre_fetch;
  int32_t pre_decode;
  int pre_decode_func;
  
  // get number of execution steps based on operation
  int num_steps;
  if (!get_steps(input, &num_steps)) return NULL;
  
  int output_reg;
  int targets_x[num_steps];
  int targets_type[num_steps];
  if(!get_bird_and_target_values(input, num_steps, &output_reg, stage, targets_x, targets_type)) return NULL;

  assemble(input_store, true, &pre_fetch);
  decode(&pre_decode, &pre_fetch, &pre_decode_func);
  if (!execute(NULL, pre_decode_func, 0, &pre_decode)) {
    printf("Error executing player input!\n");
    return NULL;
  }

  for (int i=0; i<stage->portal_count; i++) {
    free(stage->portal_ptrs[i]->info->value);
    stage->portal_ptrs[i]->info->value = to_hex(state.regs[atoi(stage->portal_ptrs[i]->info->num + 1)]);
  }

  char *output = to_hex(state.regs[output_reg]);
  AnimatePanel *panel = create_panel(targets_x, num_steps, output, targets_type);
  if (panel == NULL) {
    printf("Failed to initialise panel!\n");
    return NULL;
  }
  return panel;
}
