
.SUFFIXES:

CC=gcc
CFLAGS=-Isrc
HEADERS=$(shell find ./src -type f -name "*.h")
SOURCES=$(shell find ./src -type f -name "*.c")
OBJECTS=$(shell find ./src -type f -name "*.c" -printf '%p.o\n' | sed -e 's/src/build/')

.PHONY: all debug run clean

all: ./pxe

debug: debugdef all

debugdef:
	$(eval CFLAGS = $(CFLAGS) -ggdb -DDEBUG)

./pxe: $(OBJECTS)
	$(CC) -o $@ $^

./build/%.o: ./src/% $(HEADERS)
	mkdir -p $(shell dirname $@)
	$(CC) -c -o $@ $< $(CFLAGS)

run: ./pxe
	./pxe

clean:
	rm -f pxe
	rm -rf ./build/*
