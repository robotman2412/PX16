
.SUFFIXES:

CC            = gcc
CFLAGS        = -Isrc
CFLAGS_DEBUG  = -Isrc -ggdb -DDEBUG
HEADERS       = $(shell find ./src -type f -name "*.h")
SOURCES       = $(shell find ./src -type f -name "*.c")
OBJECTS       = $(shell find ./src -type f -name "*.c" -printf '%p.o\n' | sed -e 's/src/build/')
OBJECTS_DEBUG = $(shell find ./src -type f -name "*.c" -printf '%p.debug.o\n' | sed -e 's/src/build/')

.PHONY: all debug run clean

all: ./build/main.o
	cp $< ./pxe

debug: ./build/debug.o
	cp $< ./pxe

./build/main.o: $(OBJECTS)
	$(CC) -o $@ $^

./build/%.o: ./src/% $(HEADERS)
	@mkdir -p $(shell dirname $@)
	$(CC) -c -o $@ $< $(CFLAGS)

./build/debug.o: $(OBJECTS_DEBUG)
	$(CC) -o $@ $^

./build/%.debug.o: ./src/% $(HEADERS)
	@mkdir -p $(shell dirname $@)
	$(CC) -c -o $@ $< $(CFLAGS_DEBUG)

run: ./pxe
	./pxe

clean:
	rm -f pxe
	rm -rf ./build/*
