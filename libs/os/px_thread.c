
#include <px_thread.h>

// The currently active thread.
px_thd_t *_px_current_thread;

// Switches contexts to the given context.
void px_thread_wake(px_thd_t *thread) {
	int flags;
	// Save PF just in case.
	asm (
		"MOV %, PF"
		: "=rm" (flags)
	);
	// Disable interrupts because we are entering a critical region.
	px_disable_irq();
	
	// Have the real low-level things get done by CRITICAL ASSEMBLY.
	// We can't risk unpredictable output from the compiler.
	_px_thd_wake_int(thread->stackptr, &_px_current_thread->stackptr);
	
	// Code beyond this point happens only when the current thread regains control.
	// Because of the threading implementation, execution is linear.
	
	// Restore that again, which also re-enables interrupts.
	asm (
		"MOV PF, %"
		: "=rm" (flags)
	);
}

// Forces a context switch to happen.
void px_yield() {
	
}
