CC      = gcc
LIBS    = -lSDL2 -lSDL2_image -lSDL2_ttf
CFLAGS  = `sdl2-config --libs --cflags` -Wall -ggdb3 -O0 -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Werror -pedantic -lm
ASDIR   = ../assemble_files/
ASOBJS  = classify.o assem_funcs.o multiply_a.o data_processing_a.o branch_a.o sdt_a.o assemble_main.o
ASSOBJS = $(ASOBJS:%.o=$(ASDIR)%.o)
EMDIR   = ../emulate_files/
EMOBJS  = emulate_funcs.o data_process.o branch.o multiply.o single_data_transfer.o dc_ex.o
EMUOBJS = $(EMOBJS:%.o=$(EMDIR)%.o)
OBJS    = globals.o parse_level_info.o sprites.o parse_player_input.o animation.o player_input.o game.o
EXEC    = game

.SUFFIXES: .c .o

.PHONY: all clean

all: $(OBJS) $(EXEC)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC): $(OBJS) $(ASSOBJS) $(EMUOBJS) 
	$(CC) $(OBJS) $(ASSOBJS) $(EMUOBJS) -o ../$(EXEC) $(LIBS)

clean:
	rm -f $(wildcard *.o)
	rm -f game
