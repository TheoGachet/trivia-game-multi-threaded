# Makefile
CC=gcc
CFLAGS= -D_POSIX_C_SOURCE=199309L -W -Wall -ansi -pedantic
DEPS = client.h server.h functions.h shared.h
OBJ = main.o client.o server.o functions.o 

# Regle de compilation
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Regle d'édition de liens
my_program: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

# Nettoyage des fichiers temporaires
.PHONY: clean

clean:
	rm -f *.o
