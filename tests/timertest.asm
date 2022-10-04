
// IRQ vector
	.db irqhandler
// NMI vector
	.db irqhandler
// Entry vector.
	.db entry

irqhandler:
	// Acknowledge timer.
	MOV [0xfffe], 0x0404
	
	// Report IRQ handled.
	OR  [0xffc0], 0x4000
	
	// Return from interrupt.
	MOV PF, [ST]
	MOV PC, [ST]

entry:
	// Initialise stack.
	MOV ST, 0xffff
	SUB ST, [0xffff]
	
	// Test whether timer is still enabled.
	MOV R0, [0xfffe]
	AND R0, 0x0001
	MOV.EQ PC, .skip
	// Report findings.
	OR  [0xffc0], 0x2000
.skip:
	
	// Print message.
	OR  [0xffc0], 0x8000
	
	// Enable interrupts.
	OR  PF, 0x0002
	
	// Initialise timer.
	// Set time to 0.
	MOV [0xfffa], 0
	MOV [0xfffb], 0
	// Set limit.
	MOV [0xfff8], 10000
	MOV [0xfff9], 0
	// Copy value to counter and start timer.
	OR  [0xfffe], 0x0505
	
	// Go to deep sleep.
	OR  [0xfffe], 0x4000
	
	// Idle forever.
	DEC PC
