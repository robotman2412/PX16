
CC=gcc
CFLAGS=-Isrc
HEADERS=$(shell find ./src -type f -name "*.h")
SOURCES=$(shell find ./src -type f -name "*.c")
OBJECTS=$(shell find ./src -type f -name "*.c" -printf '%p.o\n' | sed -e 's/src/build/')
OUTDIRS=$(shell find src -type d | sed -e 's/src/build/')

all: $(OBJECTS)
	$(CC) -o pxe $^
	
builddir: $(OUTDIRS)
	mkdir -p $^

./build/%.o: ./src/% $(HEADERS) $(OUTDIRS)
	$(CC) -ggdb -c -o $@ $< $(CFLAGS)

run: all
	./pxe

clean:
	rm -f pxe
	rm -rf ./build/*
