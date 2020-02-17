CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG) -lpthread
DEPS = hash.h bucket.h graph.h list.h worker.h
OBJ1 = bankserver.o hash.o bucket.o graph.o list.o worker.o
OBJ2 = bankclient.o hash.o bucket.o graph.o list.o

all: bankserver bankclient

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS)

bankserver: $(OBJ1)
	$(CC) -o $@ $(OBJ1) $(CFLAGS)

bankclient: $(OBJ2)
	$(CC) -o $@ $(OBJ2) $(CFLAGS)

.PHONY: clean
	
clean:
	rm -f *.o

