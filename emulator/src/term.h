
#ifndef TERM_H
#define TERM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "px16.h"

#define ANSI_CLRLN   "\033[2K"
#define ANSI_DEFAULT "\033[0m"

#define ANSI_BLUE_FG "\033[94m"

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

// Draws the matrix display.
void draw_display(core *cpu, memmap *mem);
// Draws the registers.
void draw_regs(core *cpu);
// Draws some statistics;
void draw_stats(double target_hz, double measured_hz);

#endif //TERM_H
