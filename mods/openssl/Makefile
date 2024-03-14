CC=gcc
CFLAGS=-g -fPIC -I../../lib -lcrypto -lssl
OBJ=../openssl.so

main:
	$(CC) -shared -o $(OBJ) $(CFLAGS) ./openssl.c
	@echo "All Done!"
