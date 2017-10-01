CC=gcc
random : random.c
	$(CC) $(CFLAGS) $? -o $@ -lgmp -lpopt -Wall -pedantic

.PHONY: clean
clean :
	-rm random 
