
#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include "px16.h"

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

// Redraws the UI things.
void redraw(core *cpu, memmap *mem);
// Handles a single char of term input.
void handle_term_input(char c);

// Handler for program exit.
void exithandler();
// Signal handler so as to leave a sane TTY on exit.
void sighandler_abort(int sig);
// Signal handler so as to request a clean exit.
void sighandler_exit(int sig);

#endif //MAIN_H
