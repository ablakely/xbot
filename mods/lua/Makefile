CC=gcc
CFLAGS=-fPIC -I../../lib -llua
OBJ=../lua.so

main:
	$(CC) -shared -o $(OBJ) $(CFLAGS) ./lua.c
	@echo "All Done!"
