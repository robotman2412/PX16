
#include "main.h"
#include "memmap.h"
#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>

static double   measured_hertz;
static double   target_hertz;
static uint64_t sim_us_delay;
static uint64_t sim_ticks;

static bool exuent = false;

int main(int argc, char **argv) {
	// Add the exit handler.
	if (atexit(exithandler)) {
		fputs("Could not register exit handler; aborting!\n", stderr);
		return -1;
	}
	
	// Critical error signal handlers.
	if (signal(SIGINT,  sighandler_abort) == SIG_ERR) goto ohcrap_nosig;
	if (signal(SIGTSTP, sighandler_abort) == SIG_ERR) goto ohcrap_nosig;
	if (signal(SIGILL,  sighandler_abort) == SIG_ERR) goto ohcrap_nosig;
	if (signal(SIGABRT, sighandler_abort) == SIG_ERR) goto ohcrap_nosig;
	if (signal(SIGFPE,  sighandler_abort) == SIG_ERR) goto ohcrap_nosig;
	if (signal(SIGTERM, sighandler_abort) == SIG_ERR) goto ohcrap_nosig;
	goto ohsig;
	
	ohcrap_nosig:
	fputs("Could not register signal handler; aborting!\n", stderr);
	return -1;
	
	ohsig:
	// Non-critical signal handlers.
	signal(SIGHUP,  sighandler_exit);
	signal(SIGQUIT, sighandler_exit);
	
	// Set TTY mode to disable line buffering and echoing.
	system("stty cbreak -echo isig");
	fputs("\033[?25l\033[?1049h", stdout);
	int flags = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, flags | O_NONBLOCK);
	
	core cpu;
	memmap mem;
	badge_mmap badge;
	badge_mmap_create(&badge, &mem);
	
	// PUT TEMP.
	const word rom[] = {
		// thel nop
		// 0xFFFF, 0xFFFF, 0x0003, 0x7191,
		
		// thel scren
		0xFFFF, 0xFFFF, 0x0023, 0x0000, 0x3FFC, 0x0004, 0x0084, 0x0048,
		0x0030, 0x0000, 0x2080, 0x3FA0, 0x2000, 0x0000, 0x3180, 0x0A00,
		0x0400, 0x0A00, 0x3180, 0x0000, 0x2080, 0x3FA0, 0x2000, 0x0000,
		0x3F80, 0x2480, 0x2480, 0x2080, 0x0000, 0x007C, 0x0000, 0x007C,
		0x0054, 0x0074, 0x0000, 0xDF26, 0xFFFF, 0x7F05, 0xFFFF, 0x7FAE,
		0x002A, 0x7191, 0x7E26, 0xFFE0, 0x8E66, 0x0023, 0x03E6, 0xFFE0,
		0x7010, 0x7FAD, 0x002C, 0xD9A6,
	};
	badge.rom = rom;
	badge.rom_len = sizeof(rom) / sizeof(word);
	sim_sethertz(1000);
	core_reset(&cpu);
	
	// Show.
	redraw(&cpu, &mem);
	
	uint64_t next_time = micros() + sim_us_delay;
	while (!exuent) {
		do {
			// Check term input.
			int c;
			while ((c = fgetc(stdin)) != EOF) {
				handle_term_input(c);
			}
			if (exuent) goto exit;
			
			// Sleep for a bit.
			uint64_t sleep_time = next_time - micros();
			if (sleep_time > 1000) sleep_time = 1000;
			usleep(sleep_time);
		} while(next_time > micros());
		
		// Simulate.
		uint64_t tick_count = fast_ticks(&cpu, &mem, sim_ticks);
		uint64_t too_fast = 0;
		if (tick_count > sim_ticks) {
			too_fast = (tick_count - sim_ticks) * (1000000 / target_hertz);
		}
		
		// Set next wakeup time.
		uint64_t now = micros();
		uint64_t prev_time = next_time;
		next_time += sim_us_delay + too_fast;
		if (next_time < now - 4*sim_us_delay) next_time = now;
		
		// Measure speed.
		measured_hertz = 0.123456;
		
		// Show.
		redraw(&cpu, &mem);
	}
	
	exit:
	fflush(stdout);
	while(fgetc(stdin) != EOF);
	printf("Quit\n");
	
	return 0;
}


// Redraws the UI things.
void redraw(core *cpu, memmap *mem) {
	draw_display(cpu, mem);
	term_setxy(1, 19);
	draw_stats(target_hertz, measured_hertz);
	term_setxy(1, 22);
	draw_regs(&cpu);
	fflush(stdout);
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
	target_hertz = hertz;
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
	return target_hertz;
}

// Gets the measured frequency in hertz.
double sim_measurehertz() {
	return measured_hertz;
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
	fputs("\033[0m\033[?25h\033[?1049l", stdout);
}

// Signal handler so as to leave a sane TTY on exit.
void sighandler_abort(int sig) {
	// Flush streams.
	fflush(stderr);
	fflush(stdout);
	int flags = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, flags | O_NONBLOCK);
	usleep(10000);
	while(fgetc(stdin) != EOF);
	
	// Set TTY back to sane.
	exithandler();
	fflush(stderr);
	fflush(stdout);
	
	// Fart out an error.
	psignal(sig, "Error");
	fflush(stderr);
	fflush(stdout);
	
	// Exuent.
	exit(sig);
}

// Signal handler so as to request a clean exit.
void sighandler_exit(int sig) {
	exuent = true;
}
