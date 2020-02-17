CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG)
OBJ1 = channels.c board.c
OBJ2 = channels.c boardpost.c
DEPS = board.h boardpost.h channels.h

all: board boardpost

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS)

board: $(OBJ1)
	$(CC) -o $@ $(OBJ1) $(CFLAGS)

boardpost: $(OBJ2)
	$(CC) -o $@ $(OBJ2) $(CFLAGS)

clean:
	rm -f *.o
