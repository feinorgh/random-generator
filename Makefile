CC=gcc
random : random.c
	$(CC) $(CFLAGS) $? -o $@ -lgmp -lpopt -Wall -pedantic -std=gnu99

.PHONY: clean
clean :
	-rm random 
