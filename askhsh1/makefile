CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG)
DEPS = hash.h bucket.h graph.h operations.h list.h
OBJECTS = bank.o hash.o bucket.o graph.o operations.o list.o

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS)

elegxos: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CFLAGS)

.PHONY: clean
	
clean:
	rm -f *.o
