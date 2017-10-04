CC=gcc
random : random.c
	$(CC) $(CFLAGS) $? -o $@ -lgmp -Wall

.PHONY: clean
clean :
	-rm random 
