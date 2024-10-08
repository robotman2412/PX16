
#include "main.h"
#include "runner.h"
#include "memmap.h"
#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <termios.h>

#include <progmap.h>

double   measured_hertz;
double   target_hertz;
uint64_t sim_us_delay;
int64_t  sim_ticks;

core cpu;
memmap mem;
badge_mmap badge;

bool exuent = false;

double rolling_avg[N_ROLLING_AVG] = {0};
size_t rolling_idx = 0;
volatile bool warp_speed = false;
volatile bool running;

uint64_t sim_total_ticks = 0;

pthread_t runner_handle;

// Initial flags for termios.
struct termios origTerm;
// Is there an origTerm?
bool hasOrigTerm;
// Initial flags for STDIN.
static int fflags;
// Resets flags for STDIN.
static void resetcntl() {
    fcntl(0, F_SETFL, fflags);
	tcsetattr(0, 0, &origTerm);
}

std::string map_path;

int main(int argc, char **argv) {
	for (int i = 0; i < N_ROLLING_AVG; i++) {
		rolling_avg[i] = 0;
	}
	
	// Disable STDIN echo.
	atexit(resetcntl);
	struct termios termInfo;
	if (!tcgetattr(0, &termInfo)) {
		hasOrigTerm = false;
		origTerm = termInfo;
		termInfo.c_lflag &= ~ECHO;
		termInfo.c_lflag &= ~ICANON;
		tcsetattr(0, 0, &termInfo);
	} else {
		hasOrigTerm = true;
	}
	
	// Make STDIN nonblocking.
	fflags = fcntl(0, F_GETFL);
    fcntl(0, F_SETFL, fflags | O_NONBLOCK);
	
	// Create emulated hardware.
	running = false;
	badge_mmap_create(&badge, &mem);
	
	// PUT TEMP.
	const word rom[] = {
		// thel nop
		// 0xFFFF, 0xFFFF, 0x0003, 0x7191,
		
// FFFF FFFF 0023 0000 3FFC 0004 0084 0048
// 0030 0000 2080 3FA0 2000 0000 3180 0A00
// 0400 0A00 3180 0000 2080 3FA0 2000 0000
// 3F80 2480 2480 2080 0000 007C 0000 007C
// 0054 0074 0000 DF26 FFFF 7F05 FFFF 7FAE
// 002A 7191 7E26 FFE0 8E66 0023 03E6 FFE0
// 7010 7FAD 002C D9A6
		
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
	
	bool has_rom = false;
	bool has_map = false;
	bool start_running = false;
	
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-r")) {
			start_running = true;
			
		} else if (!strcmp(argv[i], "-x")) {
			i ++;
			if (has_rom) {
				printf("Executable file already specified\n");
				return 1;
			} else if (i < argc) {
				badge_load_rom(&badge, argv[i]);
			} else {
				printf("Missing filename for '-x'\n");
				return 1;
			}
			
			has_rom = true;
		} else if (!strcmp(argv[i], "-m")) {
			i ++;
			if (has_map) {
				printf("Map file already specified\n");
				return 1;
			} else if (i < argc) {
				map_path = argv[i];
			} else {
				printf("Missing filename for '-m'\n");
				return 1;
			}
			
			has_map = true;
		} else {
			printf("Unknown option '%s'\n", argv[i]);
			return 1;
		}
	}
	
	sim_sethertz(1000000);
	core_reset(&cpu, true);
	
	// Create runner thread.
	pthread_create(&runner_handle, NULL, &runner_main, NULL);
	running = start_running;
	
	window_main();
	
	fflush(stdout);
	
	return 0;
}


// Return unix time in seconds.
uint64_t secs() {
	struct timespec now;
	timespec_get(&now, TIME_UTC);
	return now.tv_sec;
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
	if (hertz < 50) {
		sim_ticks = 1;
		sim_us_delay = 1000000.0 / hertz;
	} else {
		sim_us_delay = 20000;
		sim_ticks    = hertz / 50.0;
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
