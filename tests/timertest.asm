
// IRQ vector
	.db irqhandler
// NMI vector
	.db irqhandler
// Entry vector.
	.db entry

irqhandler:
	// Acknowledge timer.
	MOV [0xfffe], 0x0404
	
	// Print message.
	MOV R0, timermsg
	MOV.JSR PC, print
	
	// Return from interrupt.
	MOV PF, [ST]
	MOV PC, [ST]

entry:
	// Initialise stack.
	MOV ST, 0xffff
	SUB ST, [0xffff]
	
	// Read button state.
	MOV R0, [0xfffd]
	MOV [0x8000], R0
	
	// Test whether timer is still enabled.
	MOV R0, [0xfffe]
	AND R0, 0x0001
	MOV.EQ PC, .skip
	// Report findings.
	MOV R0, wakemsg
	MOV.JSR PC, print
.skip:
	
	// Print message.
	MOV R0, startmsg
	MOV.JSR PC, print
	
	// Enable interrupts.
	// OR  PF, 0x0002
	
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

print:
	MOV [ST], R1
	MOV PC, .check
.loop:
	MOV R1, [R0]
	MOV [0xfff6], R1
	INC R0
.check:
	CMP1 [R0]
	MOV.UGE PC, .loop
	MOV R1, [ST]
	MOV PC, [ST]

startmsg:
	.db "Starting timer test.\n", 0
wakemsg:
	.db "Woke from sleep.\n", 0
timermsg:
	.db "Timer fired!\n", 0
