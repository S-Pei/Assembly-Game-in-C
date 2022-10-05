#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "parse_level_info.h"
#include "globals.h"

static char *TEST_DIR =  "../../arm11_testsuite/test_cases/";


Level *parse_level_info(const char* lvlname) {
    // READ .s FILE
    char toread[strlen(TEST_DIR) + strlen(lvlname) + 4];
    strcpy(toread, TEST_DIR);
    strcat(toread, lvlname);
    strcat(toread, ".s");
    FILE *s_fp;
    s_fp = fopen(toread, "r");
    if (s_fp == NULL) { printf("error opening file!\n"); return NULL; }
    assert(s_fp);
    char buffer[MAX_LINE_INPUT];
    int rs_count = 0;
    int imms_count = 0;
    int rs[14];
    int32_t imms[10];
    for (int i=0; i<sizeof(rs)/sizeof(int); i++) {
        rs[i] = -1;
    }

    char *read = fgets(buffer, MAX_LINE_INPUT, s_fp);
    if (read == NULL) { printf("error reading file!\n"); return NULL; }
    while (read) {
        read[strlen(read) - 1] = '\0';
        if(!(read_assembly_line(LEVEL_INFO, read, rs, &rs_count, imms, &imms_count, NULL))) {
            printf("Error reading assembly line!\n");
            return NULL;
        }
        read = fgets(buffer, MAX_LINE_INPUT, s_fp);
    }
    fclose(s_fp);

    // READ .out FILE
    strcpy(toread, TEST_DIR);
    strcat(toread, lvlname);
    strcat(toread, ".out");
    FILE *out_fp;
    out_fp = fopen(toread, "r");
    if (out_fp == NULL) { printf("error opening file!\n"); return NULL; }
    assert(out_fp);

    int32_t ans[rs_count];
    int got_ans_count = 0;
    char *pc;

    read = fgets(buffer, MAX_LINE_INPUT, out_fp);
    if (read == NULL) { printf("error reading file!\n"); return NULL; }
    while (read && got_ans_count < rs_count + 1) {
        char *reg_line = strchr(read, '$');
        if (reg_line == NULL) {
            if(read[0] == 'P') {
                read = strchr(read, ':');
                char *no_leading_space = remove_lead_spaces(read + 1);
                pc = to_hex(get_num(no_leading_space));
                got_ans_count ++;
            }
            read = fgets(buffer, MAX_LINE_INPUT, out_fp);
            continue;
        }
        char *no_leading_space = remove_lead_spaces(reg_line + 1);
        int reg_num = get_num(no_leading_space);
        read = strchr(no_leading_space, ':');
        no_leading_space = remove_lead_spaces(read + 1);
        int32_t reg_value = get_num(no_leading_space);
        for (int i=0; i<rs_count; i++) {
            if (rs[i] != reg_num) continue;
            ans[i] = reg_value;
            got_ans_count ++;
            break;
        }
        read = fgets(buffer, MAX_LINE_INPUT, out_fp); 
    }
    fclose(out_fp);

    Level *lvl = create_level(rs_count, imms_count, rs, imms, ans, pc);
    return lvl;
}