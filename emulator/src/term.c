
#include "term.h"
#include "px16.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

/* ======== General terminal utilities ======== */

// Sets the terminal's cursor position.
void term_setpos(pos to) {
	term_setxy(to.x, to.y);
}

// Sets the terminal's cursor position.
void term_setxy(uint32_t x, uint32_t y) {
	printf("\033[%d;%dH", y, x);
}

// Gets the terminal's cursor position.
pos term_getpos() {
	// Query the position.
	printf("\033[6n");
	pos res;
	scanf("\033[%d;%dR", &res.y, &res.x);
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

/* ======== Text descriptions ======== */

// Describes the HURTS in human-readable form, 3 significant figures.
// Output string is up to 10 bytes (including terminator).
void desc_speed(double hertz, char *to) {
	if (hertz >= 750000000) {
		sprintf(to, "%4lf GHz", hertz / 1000000000);
	} else if (hertz >= 750000) {
		sprintf(to, "%4lf MHz", hertz / 1000000);
	} else if (hertz >= 750) {
		sprintf(to, "%4lf KHz", hertz / 1000);
	} else {
		sprintf(to, "%4lf  Hz", hertz);
	}
}

/* ======== Visualisations ======== */

// Draws the matrix display.
void draw_display(core *cpu, memmap *mem) {
	const char on_col[]  = "\033[44;97m";
	const char off_col[] = "\033[40;90m";
	
	// Copy mem to a little buf.
	word screen_addr = 0xffc0;
	word screen[32];
	for (word i = 0; i < 32; i++) {
		screen[i] = mem->mem_read(cpu, screen_addr + i, true, mem->mem_ctx);
	}
	
	// Make a little header bar.
	pos old = term_getpos();
	pos size = term_getsize();
	term_setxy(1 + (size.x - 14) / 2, 1);
	fputs("\033[2KMatrix Display", stdout);
	term_setxy(1, 2);
	
	// And fart out everything.
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 32; x++) {
			bool val = (screen[x] >> y) & 1;
			fputs(val ? on_col : off_col, stdout);
			fputs("  ", stdout);
		}
		fputc('\n', stdout);
	}
	
	// Reset thel TTY.
	fputs("\033[0m", stdout);
	pos cur = term_getpos();
	if (old.y < cur.y) {
		old.y = cur.y;
	}
	term_setpos(old);
}

// Draws the registers.
void draw_regs(core *cpu) {
	/*
	 * Stuff to collect:
	 * R0,   R1,   R2,  R3,  PC, ST, PF, PC
	 * imm0, imm1, PbA, PbB, Rb, Db, Ab, AR
	 */
	word regs[14];
	// Collect the regfile.
	memcpy(regs, cpu->regfile, sizeof(word) * 7);
	// Collect IMM.
	regs[7]  = cpu->imm0;
	regs[8]  = cpu->imm1;
	// Collect busses.
	regs[9]  = cpu->par_bus_a;
	regs[10] = cpu->par_bus_b;
	regs[11] = cpu->data_bus;
	regs[12] = cpu->addr_bus;
	// Collect AR.
	regs[13] = cpu->AR;
	
	// Names to fart out.
	const char names[14][5] = {
		"R0  ", "R1  ", "R2  ", "R3  ", "ST  ", "PF  ", "PC  ",
		"imm0", "imm1", "PbA ", "PbB ", "Db  ", "Ab  ", "AR  ",
	};
	
	// Calculate spacing.
	int left = 4;
	int spacing = 2;
	char *strleft = malloc(left + 1);
	memset(strleft, ' ', left);
	strleft[left] = 0;
	char *strspacing = malloc(spacing + 1);
	memset(strspacing, ' ', spacing);
	strspacing[spacing] = 0;
	
	// Start outputting the thing.
	for (int y = 0; y < 2; y++) {
		fputs("\033[0m", stdout);
		fputs(strleft, stdout);
		for (int x = 0; x < 7; x++) {
			fputs(names[y*7+x], stdout);
			fputs(strleft, stdout);
		}
		fputs("\n\033[94m", stdout);
		fputs(strleft, stdout);
		for (int x = 0; x < 7; x++) {
			printf("%04X", regs[y*7+x]);
			fputs(strleft, stdout);
		}
		fputs("\033[0m\n", stdout);
	}
	
	free(strleft);
	free(strspacing);
}
