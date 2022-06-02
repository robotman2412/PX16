
	// Low-level context switch.
	// TODO: Enforce interrupts disabled.
_px_thd_wake_int:
	// Save other regs.
	MOV [ST], R2
	MOV [ST], R3
	
	// Exchange stack pointers.
	MOV [R1], ST
	MOV ST, R0
	
	// Restore other regs.
	MOV R3, [ST]
	MOV R2, [ST]
	
	// Return.
	MOV PC, [ST]
