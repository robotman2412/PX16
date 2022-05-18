
CC=gcc
CFLAGS=-Isrc
DEPS=src/main.h src/term.h src/px16.h

./build/%.o: ./src/% $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: ./build/px16.c.o ./build/main.c.o ./build/term.c.o
	$(CC) -o pxe $^

run: all
	./pxe

clean:
	rm -rf ./build/*
