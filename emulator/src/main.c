
#include "main.h"
#include "memmap.h"
#include "term.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	// Add the exit handler.
	if (atexit(exithandler)) {
		fputs("Could not register exit handler; aborting!\n", stderr);
		return -1;
	}
	
	// Set TTY mode to disable line buffering and echoing.
	system("stty cbreak -echo -isig");
	
	core cpu;
	memmap mem;
	badge_mmap badge;
	badge_mmap_create(&badge, &mem);
	
	// PUT TEMP.
	const word rom[] = {
		0xFFFF, 0xFFFF, 0x0023, 0x0000, 0x3FFC, 0x0004, 0x0084, 0x0048,
		0x0030, 0x0000, 0x2080, 0x3FA0, 0x2000, 0x0000, 0x3180, 0x0A00,
		0x0400, 0x0A00, 0x3180, 0x0000, 0x2080, 0x3FA0, 0x2000, 0x0000,
		0x3F80, 0x2480, 0x2480, 0x2080, 0x0000, 0x007C, 0x0000, 0x007C,
		0x0054, 0x0074, 0x0000, 0x7F26, 0xFFFF, 0x7FAE, 0x0028, 0x7191,
		0x7E26, 0xFFE0, 0x8E66, 0x0023, 0x03E6, 0xFFE0, 0x7010, 0x7FAD,
		0x002A, 0xD9A6,
	};
	badge.rom = rom;
	badge.rom_len = 50;
	
	core_reset(&cpu);
	fast_ticks(&cpu, &mem, 10000);
	draw_display(&cpu, &mem);
	draw_regs(&cpu);
	
	return 0;
}

// Handler for program exit.
void exithandler() {
	// Restore TTY to sane.
	system("stty sane");
}
