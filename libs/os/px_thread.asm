
	// .global _px_current_thread



	// Switches contexts to the given context.
	// Sets _px_current_thread after switch.
	// void px_thread_wake(px_thd_t *thread);
	// .global px_thread_wake
px_thread_wake:
	MOV [ST], R1
	// Preserve PF.
	MOV [ST], PF
	// Disable interrupts.
	AND PF, 0xfffd
	
	// Store ST to the current thread info.
	LEA R1, [PC~_px_current_thread]
	MOV [R1], ST
	// Load ST from new thread info.
	MOV ST, [R0]
	// Update running thread.
	MOV [PC~_px_current_thread], R0
	
	MOV R1, [ST]
	// Restore PF and return.
	MOV PF, [ST]
	MOV PC, [ST]
