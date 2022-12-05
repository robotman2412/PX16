
// IRQ handler.
	.db isr_irq
// NMI handler
	.db isr_nmi
// Entrypoint
	.db entry

// LEA test.
	.equ lea_test, 0x0004

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
	// region
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
	
	// Test LEA.
	MOV [0xfff6], 0x4c
	
	LEA R0, [lea_test]		// Static LEA test.
	CMP R0, lea_test
	MOV.NE PC, fail
	
	LEA R1, [PC~lea_test]	// PIE LEA test.
	CMP R1, lea_test
	MOV.NE PC, fail
	
	// Test pop.
	MOV [0xfff6], 0x50
	
	MOV R0, [ST]			// Pop test (return address from jsr0).
	CMP R0, .jsr0_ret
	MOV.NE PC, fail
	
	// Basic tests success.
	MOV [0xfff6], 0x2e
	MOV [0xfff6], 0x0a
	
	LEA R0, [msg]
	MOV.JSR PC, print
	
	MOV.JSR PC, test_add
	MOV.JSR PC, test_sub
	MOV.JSR PC, test_and
	MOV.JSR PC, test_or
	MOV.JSR PC, test_xor
	
	// After main code finishes, halt.
halt:
	OR  [0xfffe], 0x1060
	MOV PC, halt

fail:
	MOV [0xfff6], 0x21
	MOV PC, halt
	// endregion


check_print:
	// region
	MOV [ST], R0
	MOV [ST], R1
	MOV [ST], R2
	MOV [ST], R3
	MOV [ST], PF
	
	.equ .sr0, 3
	.equ .sr1, 2
	.equ .sr2, 1
	
	MOV R0, [cur]
	MOV.JSR PC, print
	
	MOV [0xfff6], 0x20	// Print ' '
	
	MOV R0, [ST+.sr0]
	MOV.JSR PC, print_hex
	
	MOV [0xfff6], 0x2c	// Print ','
	MOV [0xfff6], 0x20	// Print ' '
	
	MOV R0, [ST+.sr1]
	MOV.JSR PC, print_hex
	
	MOV [0xfff6], 0x20	// Print ' '
	MOV [0xfff6], 0x3d	// Print '='
	MOV [0xfff6], 0x20	// Print ' '
	
	MOV R0, [ST+.sr2]
	MOV.JSR PC, print_hex
	
	MOV PF, [ST]
	MOV R3, [ST]
	MOV R2, [ST]
	MOV R1, [ST]
	MOV R0, [ST]
	MOV PC, [ST]
	// endregion

check_cs_ne:
	// region
	MOV [ST], R2
	MOV [ST], R3
	
	MOV.EQ PC, .fail
	MOV.CC PC, .fail
	MOV.JSR PC, check
	
.fail:
	MOV R0, r_fail
	MOV.JSR PC, print
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV PC, [ST]
	// endregion

check_cs:
	// region
	MOV [ST], R2
	MOV [ST], R3
	
	MOV.CC PC, .fail
	MOV.JSR PC, check
	
.fail:
	MOV R0, r_fail
	MOV.JSR PC, print
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV PC, [ST]
	// endregion

check_ne:
	// region
	MOV [ST], R2
	MOV [ST], R3
	
	MOV.EQ PC, .fail
	MOV.JSR PC, check
	
.fail:
	MOV R0, r_fail
	MOV.JSR PC, print
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV PC, [ST]
	// endregion

check:
	// region
	MOV [ST], R2
	MOV [ST], R3
	
	CMP R0, R1
	MOV.EQ PC, .ok
	
	MOV R0, r_fail
	MOV.JSR PC, print
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV PC, [ST]
	
.ok:
	MOV R0, r_ok
	MOV.JSR PC, print
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV PC, [ST]
	// endregion



test_add:
	// region
	MOV [ST], R0
	MOV [ST], R1
	MOV [ST], R2
	MOV [ST], R3
	MOV [cur], t_add
	
	MOV R0, 0x81c0
	ADD R0, 0x8f3f
	MOV R1, 0x10ff
	MOV.JSR PC, check_cs_ne
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV R1, [ST]
	MOV R0, [ST]
	MOV PC, [ST]
	// endregion



test_sub:
	// region
	MOV [ST], R0
	MOV [ST], R1
	MOV [ST], R2
	MOV [ST], R3
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV R1, [ST]
	MOV R0, [ST]
	MOV PC, [ST]
	// endregion



test_and:
	// region
	MOV [ST], R0
	MOV [ST], R1
	MOV [ST], R2
	MOV [ST], R3
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV R1, [ST]
	MOV R0, [ST]
	MOV PC, [ST]
	// endregion



test_or:
	// region
	MOV [ST], R0
	MOV [ST], R1
	MOV [ST], R2
	MOV [ST], R3
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV R1, [ST]
	MOV R0, [ST]
	MOV PC, [ST]
	// endregion



test_xor:
	// region
	MOV [ST], R0
	MOV [ST], R1
	MOV [ST], R2
	MOV [ST], R3
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV R1, [ST]
	MOV R0, [ST]
	MOV PC, [ST]
	// endregion



print_hex:
	// region
	MOV [ST], R1
	MOV [ST], R2
	MOV [ST], R3
	
	MOV.JSR PC, .char
	MOV.JSR PC, .char
	MOV.JSR PC, .char
	MOV.JSR PC, .char
	
	MOV R3, [ST]
	MOV R2, [ST]
	MOV R1, [ST]
	MOV PC, [ST]
	
.char:
	XOR R1, R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	
	MOV R2, [hexstr+R1]
	MOV [0xfff6, R2]
	MOV PC, [ST]
	// endregion

print:
	// region
	MOV [ST], R1
	LEA PC, [PC~.check]
	
.loop:
	MOV [0xfff6], R1
	INC R0
.check:
	MOV R1, [R0]
	CMP1 R1
	LEA.UGE PC, [PC~.loop]
	
	MOV R1, [ST]
	MOV PC, [ST]
	// endregion



	.section ".data"
cur:
	.zero 1



	.section ".rodata"
hexstr:
	.db "0123456789abcdef"
msg:
	.db "Basic tests sucess.\n", 0
r_ok:
	.db "ok", 0
r_fail:
	.db "fail", 0
t_lea:
	.db "LEA", 0
t_add:
	.db "ADD", 0
t_sub:
	.db "SUB", 0
t_and:
	.db "AND", 0
t_or:
	.db "OR", 0
t_xor:
	.db "XOR", 0
