
CC=gcc
CFLAGS=-Isrc
HEADERS=$(shell find ./src -type f -name "*.h")
SOURCES=$(shell find ./src -type f -name "*.c")
OBJECTS=$(shell find ./src -type f -name "*.c" -printf './build/%p.o\n' | sed -e 's|/src||')

builddir: $(shell find src -type d | sed -e 's/src/build/')
	mkdir -p $^

./build/%.o: builddir ./src/% $(HEADERS)
	$(CC) -ggdb -c -o $@ $< $(CFLAGS)

all: $(OBJECTS)
	$(CC) -o pxe $^

run:
# Do a fancy cursor position and screen clear.
	echo -en '\033[2J\033[18;1H'
# Make the project normally.
	$(MAKE) --no-print-directory all && ./pxe

clean:
	rm -rf $(find build -type f)
