CC = gcc
CFLAGS = -Wall -Wextra

all: tournament gladiator

tournament: tournament.c
	$(CC) $(CFLAGS) -o tournament tournament.c

gladiator: gladiator.c
	$(CC) $(CFLAGS) -o gladiator gladiator.c

clean:
	rm -f tournament gladiator
