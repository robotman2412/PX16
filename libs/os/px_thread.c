
#include "px_thread.h"

// The currently active thread.
// Avoid renaming the field; used by `px_thread_wake` in ASM.
px_thd_t *_px_current_thread;

// Forces a context switch to happen.
void px_yield() {
	
}

// Forces a context switch to happen; ISR safe gauranteed.
void px_yield_from_isr() {
	
}
