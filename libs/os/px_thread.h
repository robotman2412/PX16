
#ifndef PX_THREAD_H
#define PX_THREAD_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ==== Types ==== //

// Context local to a thread.
typedef struct {
	// Stack pointer of the thread.
	size_t   stackptr;
	// Name of the thread, if any.
	char    *name;
	// Priority of the thread, 0 is default.
	int      priority;
	// Total CPU time allocated to the thread.
	uint64_t time;
} px_thd_t;

// ==== Globals ==== //

// The currently active thread.
extern px_thd_t *_px_current_thread;

// ==== Context switching ==== //

// Disables interrupts.
static inline void px_disable_irq() {
	asm volatile(
		"AND PF, 0xfffd"
	);
}
// Enables interrupts.
static inline void px_enable_irq() {
	asm volatile(
		"OR PF, 0x0002"
	);
}

// Switches contexts to the given context.
void px_thread_wake(px_thd_t *thread);
// Low-level context switch.
void _px_thd_wake_int(size_t set_reg_st, size_t *read_reg_st);
// Forces a context switch to happen.
void px_yield();

#endif //PX_THREAD_H
