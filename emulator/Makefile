
CC=gcc
CFLAGS=-Isrc
HEADERS=src/main.h src/term.h src/px16.h src/memmap.h

./build/%.o: ./src/% $(HEADERS)
	$(CC) -ggdb -c -o $@ $< $(CFLAGS)

all: ./build/px16.c.o ./build/main.c.o ./build/term.c.o ./build/memmap.c.o
	$(CC) -o pxe $^

run:
# Do a fancy cursor position and screen clear.
	echo -en '\033[2J\033[18;1H'
# Make the project normally.
	$(MAKE) --no-print-directory all && ./pxe

clean:
	rm -rf ./build/*
