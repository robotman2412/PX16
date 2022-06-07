
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

void exithandler();

#endif //MAIN_H
