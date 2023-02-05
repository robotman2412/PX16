
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ==== Types ==== //

// ...

// ==== Globals ==== //

// ...

// ==== Initialisation ==== //

// Interrupt service routine.
// Handles timers with highest priority.
void _px_isr();

#ifdef __cplusplus
}
#endif
