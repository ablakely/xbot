CC=gcc
CFLAGS=-g  -std=gnu99 -c -lconfig -ldl -I./lib
BINFLAGS=-g -rdynamic -ldl -lconfig 
SRC=./src
OBJ=./build
OBJECTS=$(OBJ)/*.o
EXEC=xbot

#MODS_DIR := $(filter-out $(wildcard mods/*.so), $(wildcard mods/*))
MODS_DIR := $(filter-out $(wildcard mods/*.so) $(wildcard mods/*.dll) $(wildcard mods/*.bat) $(wildcard mods/*.obj) $(wildcard mods/*.lib) $(wildcard mods/*.exp), $(wildcard mods/*))

.PHONY: mods $(MODS_DIR)

main:
	@rm -rf build
	@mkdir build
	$(CC) $(CFLAGS) $(SRC)/config.c -o $(OBJ)/config.o
	$(CC) $(CFLAGS) $(SRC)/main.c   -o $(OBJ)/main.o
	$(CC) $(CFLAGS) $(SRC)/irc.c    -o $(OBJ)/irc.o
	$(CC) $(CFLAGS) $(SRC)/util.c   -o $(OBJ)/util.o
	$(CC) $(CFLAGS) $(SRC)/events.c -o $(OBJ)/events.o
	$(CC) $(CFLAGS) $(SRC)/module.c -o $(OBJ)/module.o
	$(CC) $(CFLAGS) $(SRC)/channel.c -o $(OBJ)/channel.o
	$(CC) $(CFLAGS) $(SRC)/timers.c -o $(OBJ)/timers.o
	$(CC) $(CFLAGS) $(SRC)/db.c -o $(OBJ)/db.o

	$(CC) -o $(EXEC) $(OBJECTS) $(BINFLAGS)
	@echo "All Done!"

mods: $(MODS_DIR)

$(MODS_DIR):
	$(MAKE) -C $@

all: main mods

clean:
	@rm -rf build $(EXEC)
	@rm -rf mods/*.so
