
#include "term.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

// Sets the terminal's cursor position.
void term_setpos(pos to) {
	term_setxy(to.x, to.y);
}

// Sets the terminal's cursor position.
void term_setxy(uint32_t x, uint32_t y) {
	printf("\033[%d;%dH", x, y);
}

// Gets the terminal's cursor position.
pos term_getpos() {
	// Query the position.
	printf("\033[6n");
	pos res;
	scanf("\033[%d;%dR", &res.x, &res.y);
	return res;
}

// Gets the terminal's size.
pos term_getsize() {
	pos orig = term_getpos();
	term_setxy(65535, 65535);
	pos size = term_getpos();
	term_setpos(orig);
	return size;
}
