
// IRQ handler.
	.db isr_irq
// NMI handler
	.db isr_nmi
// Entrypoint
	.db entry

	.equ f_zero,  0x2000
	.equ f_scout, 0x8000
	.equ f_ucout, 0x4000
	.equ f_gt,    0xc000
	.equ f_lt,    0x0000

	// An interrupt handler.
isr_irq:
isr_nmi:
	// Handle interrupts.
	// Acknowledge interrupts and return.
	OR  [0xfffe], 0x0c00
	MOV PF, [ST]
	MOV PC, [ST]

	// The entrypoint of the program.
entry:
	// Initialise stack.
	MOV ST, 0xffff
	SUB ST, [0xffff]
	
	// Test branch conditions.
	MOV [0xfff6], 0x42
	
	MOV PC, .jump0
	MOV [0xfff6], 0x21
	
.jump0:
	MOV PF, 0
	MOV.EQ PC, fail		// Zero clear test.
	MOV.CS PC, fail		// Carry out clear test.
	
	MOV PF, f_zero
	MOV.NE PC, fail		// Zero set test.
	MOV PF, f_ucout
	MOV.CC PC, fail		// Carry out set test.
	
	MOV PF, f_gt
	MOV.ULT PC, fail	// Greater than test
	MOV.ULE PC, fail
	MOV.SLT PC, fail
	MOV.SLE PC, fail
	
	MOV PF, f_lt
	MOV.UGT PC, fail	// Less than test.
	MOV.UGE PC, fail
	MOV.SGT PC, fail
	MOV.SGE PC, fail
	
	// Test CMP.
	MOV [0xfff6], 0x43
	
	MOV R0, 123
	CMP R0, 123
	MOV.NE PC, fail
	CMP R0, 124
	MOV.UGE PC, fail
	
	// Test JSR.
	MOV [0xfff6], 0x4a
	MOV R0, ST
	MOV.JSR PC, .jsr0
.jsr0_ret:
	MOV PC, fail
.jsr0:
	CMP R0, ST
	MOV.EQ PC, fail
	
	// Test pop.
	
	
	// After main code finishes, halt.
halt:
	OR  [0xfffe], 0x1060
	MOV PC, halt

fail:
	MOV [0xfff6], 0x21
	MOV PC, halt
