
#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

// Return unix time in millisecods.
uint64_t millis();
// Return unix time in microseconds.
uint64_t micros();

// Sets the frequency in hertz to simulate at.
void sim_sethertz(double hertz);
// Gets the frequency in hertz to simulate at.
double sim_gethertz();

// Handles a single char of term input.
void handle_term_input(char c);

void exithandler();

#endif //MAIN_H
