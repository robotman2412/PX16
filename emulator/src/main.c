
#include "main.h"
#include "memmap.h"
#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

static double sim_hertz;
static uint64_t sim_us_delay;
static uint64_t sim_ticks;

static bool exuent = false;

int main(int argc, char **argv) {
	// Add the exit handler.
	if (atexit(exithandler)) {
		fputs("Could not register exit handler; aborting!\n", stderr);
		return -1;
	}
	
	// Set TTY mode to disable line buffering and echoing.
	system("stty cbreak -echo isig");
	int flags = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, flags | O_NONBLOCK);
	
	core cpu;
	memmap mem;
	badge_mmap badge;
	badge_mmap_create(&badge, &mem);
	
	// PUT TEMP.
	const word rom[] = {
		// 0x0000, 0x0000, 0x0003, 0x7E26, 0x0005, 0x7E00, 0xffff, 0x7FA6, 0x0007,
		
		0xFFFF, 0xFFFF, 0x0023, 0x0000, 0x3FFC, 0x0004, 0x0084, 0x0048,
		0x0030, 0x0000, 0x2080, 0x3FA0, 0x2000, 0x0000, 0x3180, 0x0A00,
		0x0400, 0x0A00, 0x3180, 0x0000, 0x2080, 0x3FA0, 0x2000, 0x0000,
		0x3F80, 0x2480, 0x2480, 0x2080, 0x0000, 0x007C, 0x0000, 0x007C,
		0x0054, 0x0074, 0x0000, 0x7F26, 0xFFFF, 0x7FAE, 0x0028, 0x7191,
		0x7E26, 0xFFE0, 0x8E66, 0x0023, 0x03E6, 0xFFE0, 0x7010, 0x7FAD,
		0x002A, 0xD9A6,
	};
	badge.rom = rom;
	badge.rom_len = sizeof(rom) / sizeof(word);
	sim_sethertz(1000);
	core_reset(&cpu);
	
	pos reg_pos = term_getpos();
	while (!exuent) {
		usleep(sim_us_delay);
		uint64_t tick_count = fast_ticks(&cpu, &mem, sim_ticks);
		draw_display(&cpu, &mem);
		term_setpos(reg_pos);
		draw_regs(&cpu);
		fflush(stdout);
		
		int c;
		fputc('q', stdin);
		while ((c = fgetc(stdin)) != EOF) {
			handle_term_input(c);
		}
	}
	
	fflush(stdout);
	while(fgetc(stdin) != EOF);
	printf("Quit\n");
	
	return 0;
}

// Return unix time in millisecods.
uint64_t millis() {
	struct timespec now;
	timespec_get(&now, TIME_UTC);
	return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

// Return unix time in microseconds.
uint64_t micros() {
	struct timespec now;
	timespec_get(&now, TIME_UTC);
	return now.tv_sec * 1000000 + now.tv_nsec / 1000;
}

// Sets the frequency in hertz to simulate at.
void sim_sethertz(double hertz) {
	sim_hertz = hertz;
	if (hertz < 100) {
		sim_ticks = 1;
		sim_us_delay = 1000000.0 / hertz;
	} else {
		sim_us_delay = 10000;
		sim_ticks    = hertz / 100.0;
		if (!sim_ticks) sim_ticks = 1;
	}
}

// Gets the frequency in hertz to simulate at.
double sim_gethertz() {
	return sim_hertz;
}

// Handles a single char of term input.
void handle_term_input(char c) {
	if (c >= 'A' && c <= 'Z') c |= 0x60;
	if (c == 'q') exuent = true;
}

// Handler for program exit.
void exithandler() {
	// Restore TTY to sane.
	system("stty sane");
}
