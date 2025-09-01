CC=gcc
CFLAGS=-g -fPIC -I../../lib
LDFLAGS=-llua5.3
OBJ=../lua.so

main:
	$(CC) -shared -o $(OBJ) $(CFLAGS) ./wrappers.c ./events.c ./handlers.c ./lua.c $(LDFLAGS)
	@echo "All Done!"
