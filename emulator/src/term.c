
#include "term.h"
#include "main.h"
#include "px16.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

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
	uint64_t timeout = micros() + 100000;
	
	// Query the position.
	int flags = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, flags & ~O_NONBLOCK);
	printf("\033[6n");
	fflush(stdout);
	
	// Await the ESC char.
	pos res = {0, 0};
	int c;
	while (micros() < timeout) {
		c = fgetc(stdin);
		if (c == '\033') {
			break;
		} else if (c != EOF) {
			handle_term_input(c);
		}
	}
	
	// Await the [ char.
	while ((c = fgetc(stdin)) == EOF && micros() < timeout);
	if (c != '[') goto error;
	
	// Scan for position report.
	scanf("%d;%dR", &res.y, &res.x);
	fcntl(0, F_SETFL, flags);
	return res;
	
	// Not good.
	error:
	fcntl(0, F_SETFL, flags);
	return (pos) {-1, -1};
}

// Gets the terminal's size.
pos term_getsize() {
	static pos cache;
	static uint64_t cacheTime = 0;
	
	if (secs() != cacheTime) {
		pos orig = term_getpos();
		if (orig.x < 0) return cache;
		term_setxy(65535, 65535);
		pos size = term_getpos();
		if (size.x < 0) return cache;
		term_setpos(orig);
		return size;
	}
	return cache;
}

/* ======== Text descriptions ======== */

// Describes the HURTS in human-readable form, 3 significant figures.
// Output string is up to 10 bytes (including terminator).
void desc_speed(double hertz, char *to) {
	size_t buf_len = 10;
	char fmt = ' ';
	if (hertz >= 750000000) {
		fmt = 'G';
		hertz /= 1000000000;
	} else if (hertz >= 750000) {
		fmt = 'M';
		hertz /= 1000000;
	} else if (hertz >= 750) {
		fmt = 'K';
		hertz /= 1000;
	}
	
	// Show 3 significant figures.
	// Log (0, 1, 2).
	int log      = (hertz >= 100) ? 2
				 : (hertz >= 10)  ? 1 : 0;
	// Padding amount?
	int padding  = 4;
	// Decimals count.
	int decimals = 2 - log;
	snprintf(to, buf_len, "%*.*lf %cHz", padding, decimals, hertz, fmt);
}

/* ======== Visualisations ======== */

// Draws the matrix display.
void draw_display(core *cpu, memmap *mem) {
	static word old_screen[32] = {1};
	
	const char on_col[]  = "\033[44;97m";
	const char off_col[] = "\033[40;90m";
	
	// Copy mem to a little buf.
	word screen_addr = 0xffc0;
	word screen[32];
	for (word i = 0; i < 32; i++) {
		screen[i] = mem->mem_read(cpu, screen_addr + i, true, mem->mem_ctx);
	}
	
	if (!memcmp(old_screen, screen, sizeof(old_screen))) return;
	
	// Make a little header bar.
	// pos old = term_getpos();
	pos size = term_getsize();
	term_setxy(1 + (size.x - 14) / 2, 1);
	fputs(ANSI_CLRLN ANSI_BOLD "Matrix Display", stdout);
	term_setxy(1, 2);
	
	// And fart out everything.
	for (int y = 0; y < 16; y++) {
		term_setxy((size.x - 64) / 2, y + 2);
		fputs("\033[0m" ANSI_CLRLN, stdout);
		for (int x = 0; x < 32; x++) {
			bool val = (screen[x] >> y) & 1;
			fputs(val ? on_col : off_col, stdout);
			fputs("  ", stdout);
		}
	}
	fputc('\n', stdout);
	memcpy(old_screen, screen, sizeof(old_screen));
	
	// Reset thel TTY.
	fputs("\033[0m", stdout);
	// pos cur = term_getpos();
	// if (old.y < cur.y) {
	// 	old.y = cur.y;
	// }
	// term_setpos(old);
}

// Draws the registers.
void draw_regs(core *cpu, memmap *mem) {
	
	// Make a little header bar.
	pos old = {0, 22}; //term_getpos();
	pos size = term_getsize();
	term_setxy(1 + (size.x - 9) / 2, old.y);
	fputs(ANSI_CLRLN ANSI_DEFAULT ANSI_BOLD "Registers", stdout);
	term_setxy(1, old.y+1);
	
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
	// Collect HIDDEN.
	regs[9]  = cpu->par_bus_a;
	regs[10] = cpu->par_bus_b;
	regs[11] = cpu->AR;
	regs[12] = cpu->data_bus;
	regs[13] = cpu->addr_bus;
	
	// Names to fart out.
	const char *names[14] = {
		ANSI_GREEN_FG "R0  ", "R1  ", "R2  ", "R3  ", ANSI_YELLOW_FG "ST  ", "PF  ", "PC  ",
		ANSI_RED_FG   "imm0", "imm1", "PbA ", "PbB ", "AR  ", "Db  ", "Ab  ",
	};
	
	// Calculate spacing.
	int left = (size.x - 4*7) / 8;
	int delta = left + 4;
	char *strleft = malloc(left + 1);
	memset(strleft, ' ', left);
	strleft[left] = 0;
	
	// Start outputting the thing.
	for (int y = 0; y < 2; y++) {
		fputs(ANSI_DEFAULT, stdout);
		fputs(strleft, stdout);
		for (int x = 0; x < 7; x++) {
			fputs(names[y*7+x], stdout);
			fputs(strleft, stdout);
		}
		fputs("\n" ANSI_DEFAULT ANSI_BLUE_FG, stdout);
		fputs(strleft, stdout);
		for (int x = 0; x < 7; x++) {
			printf("%04X", regs[y*7+x]);
			fputs(strleft, stdout);
		}
		fputs(ANSI_DEFAULT "\n", stdout);
	}
	free(strleft);
}

// Draws some statistics;
void draw_stats(core *cpu, memmap *mem, double target_hz, double measured_hz, uint64_t tick_count) {
	static core last_shown;
	static bool shown = false;
	
	// Make a little header bar.
	pos old = {0, 19};
	//term_getpos();
	pos size = term_getsize();
	term_setxy(1 + (size.x - 10) / 2, old.y);
	fputs(ANSI_CLRLN ANSI_DEFAULT ANSI_BOLD "Statistics", stdout);
	
	// Clear lines.
	term_setxy(1, old.y+1);
	fputs(ANSI_CLRLN "\n" ANSI_CLRLN, stdout);
	
	// Draw freq.
	if (target_hz < 0) {
		char m[10];
		desc_speed(measured_hz, m);
		term_setxy(6, old.y+1);
		printf(ANSI_DEFAULT "Frequency");
		term_setxy(6, old.y+2);
		printf(ANSI_BLUE_FG "%s " ANSI_DEFAULT "/" ANSI_BLUE_FG "    ∞\n", m);
	} else {
		char m[10], t[10];
		desc_speed(measured_hz, m);
		desc_speed(target_hz, t);
		term_setxy(6, old.y+1);
		printf(ANSI_DEFAULT "Frequency");
		term_setxy(6, old.y+2);
		printf(ANSI_BLUE_FG "%s " ANSI_DEFAULT "/" ANSI_BLUE_FG " %s\n", m, t);
	}
	
	// Draw CU state.
	term_setxy(30, old.y+1);
	printf(ANSI_DEFAULT "State");
	term_setxy(30, old.y+2);
	for (int i = 0; i < n_cu_states; i++) {
		if (cpu->state.array[i]) {
			printf(ANSI_BLUE_FG "%s", cu_state_names[i]);
			break;
		}
	}
	
	// Draw total tick count.
	term_setxy(40, old.y+1);
	printf(ANSI_DEFAULT "Ticks");
	term_setxy(40, old.y+2);
	printf(ANSI_BLUE_FG "%ld", tick_count);
	
	last_shown = *cpu;
	shown      = true;
}

// Draws the badge mmio stuff.
void draw_badge_mmio(core *cpu, memmap *mem, badge_mmap *badge) {
	pos old  = {0, 27}; //term_getpos();
	pos size = term_getsize();
	term_setxy(1 + (size.x - 4) / 2, old.y);
	fputs(ANSI_CLRLN ANSI_BOLD "MMIO", stdout);
	
	// Draw timer.
	term_setxy(6, old.y + 1);
	printf(
		ANSI_DEFAULT "Timer: %s %c%c",
		badge->timer0_en ? "On " : "Off",
		badge->timer0_irq ? 'I' : '-',
		badge->timer0_nmi ? 'N' : '-'
	);
	term_setxy(6, old.y + 2);
	double diff = ((int64_t) badge->timer0_value - (int64_t) badge->timer0_limit) / 1000.0;
	printf(ANSI_CLRLN ANSI_BLUE_FG "%08x / %08x (%.1lfms)", badge->timer0_value, badge->timer0_limit, diff);
}
