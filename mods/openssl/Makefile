CC=gcc
CFLAGS=-g -fPIC -I../../lib
LDFLAGS=-lssl -lcrypto
OBJ=../openssl.so

main:
	$(CC) -shared -o $(OBJ) ./openssl.c $(CFLAGS) $(LDFLAGS)
	@echo "All Done!"
