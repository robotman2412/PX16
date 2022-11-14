
#ifndef TERM_H
#define TERM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "px16.h"
#include "memmap.h"

#define TERM_DELAY_US 2000

#define ANSI_CLRLN   "\033[2K"
#define ANSI_DEFAULT "\033[0m"
#define ANSI_BOLD    "\033[1m"

/* Foreground */
#define ANSI_BLACK_FG        "\033[30m"
#define ANSI_DARK_RED_FG     "\033[31m"
#define ANSI_DARK_GREEN_FG   "\033[32m"
#define ANSI_DARK_YELLOW_FG  "\033[33m"
#define ANSI_DARK_BLUE_FG    "\033[34m"
#define ANSI_DARK_MAGENTA_FG "\033[35m"
#define ANSI_DARK_CYAN_FG    "\033[36m"
#define ANSI_DARK_GRAY_FG    "\033[37m"
#define ANSI_GRAY_FG         "\033[90m"
#define ANSI_RED_FG          "\033[91m"
#define ANSI_GREEN_FG        "\033[92m"
#define ANSI_YELLOW_FG       "\033[93m"
#define ANSI_BLUE_FG         "\033[94m"
#define ANSI_MAGENTA_FG      "\033[95m"
#define ANSI_CYAN_FG         "\033[96m"
#define ANSI_WHITE_FG        "\033[97m"

/* Background */
#define ANSI_BLACK_BG        "\033[40m"
#define ANSI_DARK_RED_BG     "\033[41m"
#define ANSI_DARK_GREEN_BG   "\033[42m"
#define ANSI_DARK_YELLOW_BG  "\033[43m"
#define ANSI_DARK_BLUE_BG    "\033[44m"
#define ANSI_DARK_MAGENTA_BG "\033[45m"
#define ANSI_DARK_CYAN_BG    "\033[46m"
#define ANSI_DARK_GRAY_BG    "\033[47m"
#define ANSI_GRAY_BG         "\033[10m"
#define ANSI_RED_BG          "\033[101m"
#define ANSI_GREEN_BG        "\033[102m"
#define ANSI_YELLOW_BG       "\033[103m"
#define ANSI_BLUE_BG         "\033[104m"
#define ANSI_MAGENTA_BG      "\033[105m"
#define ANSI_CYAN_BG         "\033[106m"
#define ANSI_WHITE_BG        "\033[107m"

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
void draw_regs(core *cpu, memmap *mem);
// Draws some statistics;
void draw_stats(core *cpu, memmap *mem, double target_hz, double measured_hz, uint64_t tick_count);
// Draws the badge mmio stuff.
void draw_badge_mmio(core *cpu, memmap *mem, badge_mmap *badge);

#endif //TERM_H
