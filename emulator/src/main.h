
#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <pthread.h>
#include "px16.h"
#include "memmap.h"

#define N_ROLLING_AVG 8

extern double   measured_hertz;
extern double   target_hertz;
extern uint64_t sim_us_delay;
extern int64_t  sim_ticks;

extern core cpu;
extern memmap mem;
extern badge_mmap badge;

extern bool exuent;

extern double rolling_avg[N_ROLLING_AVG];
extern size_t rolling_idx;

extern volatile bool warp_speed;
extern volatile bool running;

extern uint64_t sim_total_ticks;

// Return unix time in seconds.
uint64_t secs();
// Return unix time in millisecods.
uint64_t millis();
// Return unix time in microseconds.
uint64_t micros();

// Sets the target frequency in hertz to simulate at.
void sim_sethertz(double hertz);
// Gets the target frequency in hertz to simulate at.
double sim_gethertz();
// Gets the measured frequency in hertz.
double sim_measurehertz();

// Handles a single char of term input.
void handle_term_input(char c);

// Handler for program exit.
void exithandler();
// Signal handler so as to leave a sane TTY on exit.
void sighandler_abort(int sig);
// Signal handler so as to request a clean exit.
void sighandler_exit(int sig);

#endif //MAIN_H
