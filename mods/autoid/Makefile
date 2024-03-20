CC=gcc
CFLAGS=-g -std=gnu99 -lconfig -fPIC -I../../lib
OBJ=../autoid.so

main:
	$(CC) -shared -o $(OBJ) $(CFLAGS) ./autoid.c
	@echo "All Done!"
