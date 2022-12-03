
.SUFFIXES:

MAKEFLAGS    += --jobs=$(shell nproc)
CC            = gcc
LDFLAGS       = -lpthread -lX11 -lm -lstdc++ $(shell pkg-config --libs gtk+-3.0) $(shell pkg-config --libs gtkmm-3.0)
CFLAGS_ALL    = -Isrc -Isrc/window -Wno-unused-result -Wno-format-truncation -Wno-narrowing $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags gtkmm-3.0)
CFLAGS        = $(CFLAGS_ALL) -O3
CFLAGS_DEBUG  = $(CFLAGS_ALL) -O0 -ggdb -DDEBUG
HEADERS       = $(shell find ./src -type f -name "*.h")
SOURCES       = $(shell find ./src -type f -name "*.c" -o -name "*.cpp")
OBJECTS       = $(shell find ./src -type f -name "*.c" -o -name "*.cpp" -printf '%p.o\n' | sed -e 's/src/build/')
OBJECTS_DEBUG = $(shell find ./src -type f -name "*.c" -o -name "*.cpp" -printf '%p.debug.o\n' | sed -e 's/src/build/')

.PHONY: all debug run clean

all: ./build/main.o
	cp $< ./pxe

debug: ./build/debug.o
	cp $< ./pxe

./build/main.o: $(OBJECTS)
	@echo LD $@
	@$(CC) -o $@ $^ $(LDFLAGS)

./build/%.o: ./src/% $(HEADERS) Makefile
	@mkdir -p $(shell dirname $@)
	@echo CC $<
	@$(CC) -c -o $@ $< $(CFLAGS)

./build/debug.o: $(OBJECTS_DEBUG)
	@echo LD $@
	@$(CC) -o $@ $^ $(LDFLAGS)

./build/%.debug.o: ./src/% $(HEADERS) Makefile
	@mkdir -p $(shell dirname $@)
	@echo CC $<
	@$(CC) -c -o $@ $< $(CFLAGS_DEBUG)

debug-run: debug
	./pxe

run: all
	./pxe

clean:
	rm -f pxe
	rm -rf ./build/*
