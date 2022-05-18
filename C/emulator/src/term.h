
#ifndef TERM_H
#define TERM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct s_pos;

typedef struct s_pos pos;

struct s_pos {
	uint32_t x, y;
};

// Sets the terminal's cursor position.
void term_setpos(pos to);
// Sets the terminal's cursor position.
void term_setxy(uint32_t x, uint32_t y);
// Gets the terminal's cursor position.
pos term_getpos();
// Gets the terminal's size.
pos term_getsize();

#endif //TERM_H
