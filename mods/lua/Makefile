CC=gcc
CFLAGS=-g -fPIC -I../../lib -llua
OBJ=../lua.so

main:
	$(CC) -shared -o $(OBJ) $(CFLAGS) ./wrappers.c ./events.c ./handlers.c ./lua.c
	@echo "All Done!"
