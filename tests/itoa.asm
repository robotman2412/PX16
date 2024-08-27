
	.section ".text"
	.equ uart_tx, 0xfff6

// IRQ handler.
	.db isr_irq
// NMI handler
	.db isr_nmi
// Entrypoint
	.db entry

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
	// Main code.
	MOV R0, 123
	LEA R1, [PC~temp]
	LEA.JSR PC, [PC~itod]
	
	// The printing.
	LEA.JSR PC, [PC~print]
	
	// After main code finishes, halt.
.halt:
	OR  [0xfffe], 0x1060
	MOV PC, .halt



	// Prints word in R0 as a hex string.
printhex:
	// region
	MOV [ST], R1
	MOV [ST], R2
	MOV [ST], R3
	
	// R2 helps with PIC.
	LEA R2, [PC~hexstr]
	
	// Extract four bits.
	XOR R1, R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	// Put hex char.
	MOV R1, [R2+R1]
	MOV [uart_tx], R1
	
	// Extract four bits.
	XOR R1, R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	// Put hex char.
	MOV R1, [R2+R1]
	MOV [uart_tx], R1
	
	// Extract four bits.
	XOR R1, R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	// Put hex char.
	MOV R1, [R2+R1]
	MOV [uart_tx], R1
	
	// Extract four bits.
	XOR R1, R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	SHL R0
	SHLC R1
	// Put hex char.
	MOV R1, [R2+R1]
	MOV [uart_tx], R1
	
	// Return.
	MOV R3, [ST]
	MOV R2, [ST]
	MOV R1, [ST]
	// endregion
	MOV PC, [ST]


	// Prints c-string pointer in R0.
print:
	// region
	MOV [ST], R1
	
	// HaHa, yEOs.
	CMP1 R0
	LEA.ULT PC, [PC~.exit]
.loop:
	MOV R1, [R0]
	MOV [uart_tx], R1
	INC R0
	CMP1 [R0]
	LEA.UGE PC, [PC~.loop]
	
.skip:
	MOV R1, [ST]
	// endregion
	MOV PC, [ST]


	// Turns word in R0 into base 10 ascii number string.
	// Stores up to 6 bytes to the string pointer in R1.
	// Always adds a null terminator.
itod:
	// region
	MOV [ST], R2
	MOV [ST], R3
	
	// Save string pointer for later.
	MOV [ST], R1
	// Use R1 as cycle counter.
	MOV R1, 15
	// Use decimal from stack.
	MOV [ST], R0
	// Use R2 and R3 as double dabble output.
	XOR R2, R2
	XOR R3, R3
	
	// Double dabble loop.
.dd:
	// Perform an iteration of dabbles doubling.
	MOV R0, R2
	LEA.JSR PC, [PC~double_dabble_bitwise1]
	MOV R2, R0
	MOV R0, R3
	LEA.JSR PC, [PC~double_dabble_bitwise1]
	MOV R3, R0
	// Shift in some more bits.
	SHL [ST+0]
	SHLC R2
	SHLC R3
	// Loop count check.
	DEC R1
	LEA.UGE PC, [PC~.dd]
	
	
	// Use R0 as output count.
	// R0 is already 0.
	
	// Skip leading zeroes loop.
.slz:
	// Test the top digit against zero.
	MOV R1, R3
	AND R1, 0xf000
	// If not, branch out.
	LEA.NE PC, [PC~.slz_ex]
	// If so, truncate the top digit.
	SHL R2
	SHLC R3
	SHL R2
	SHLC R3
	SHL R2
	SHLC R3
	SHL R2
	SHLC R3
	// Increment output count to correct for less digits left.
	INC R0
	CMP R0, 7
	LEA.ULT PC, [PC~.slz]
	
	
	// Prepare for outputting characters.
.slz_ex:
	// Restore string pointer to R1.
	MOV R1, [ST+1]
	// Relocate output count into stack.
	MOV [ST+0], R0
	// Use R0 as output scratch.
	
	
	// Output characters loop.
.out:
	// Grab a nibble of characters.
	XOR R0, R0
	SHL R2
	SHLC R3
	SHLC R0
	SHL R2
	SHLC R3
	SHLC R0
	SHL R2
	SHLC R3
	SHLC R0
	SHL R2
	SHLC R3
	SHLC R0
	// Add 0x30 to this to turn it into ascii.
	OR R0, 0x30
	// Store the new ascii into the output.
	MOV [R1], R0
	INC R1
	// Check output limit.
	INC [ST+0]
	CMP [ST+0], 8
	LEA.ULT PC, [PC~.out]
	
	
	// Append null terminator.
	MOV [R1], 0
	// Restore string pointer for return value.
	INC ST
	MOV R0, [ST]
	// Return.
	MOV R3, [ST]
	MOV R2, [ST]
	// endregion
	MOV PC, [ST]


	// Performs a single iteration of double dabble addition.
	// Takes input and returns in R0.
	// Takes 40 cycles.
double_dabble_bitwise0:
	// region
	MOV [ST], R2	// 3
	MOV [ST], R3
	
	// OR bits 0 and 1 into bit 0 of R2.
	MOV R2, R0		// 2
	MOV R3, R0		// 2
	SHR R3			// 2
	OR  R2, R3		// 2
	
	// AND bit 2 from R0 and bit 0 of R2 into R2.
	// Value in R3 reused
	SHR R3			// 2
	AND R2, R3		// 2
	
	// 8 and 9 detector.
	// Grabs bit 3 into bit 0 of R3.
	// Value in R3 reused.
	SHR R3			// 2
	
	// OR bit 0 from R1 into bit 0 of R2.
	OR  R2, R3		// 2
	AND R2, 0x1111	// 3
	
	// Multiply R2 by 3 and add to R0 trick.
	ADD R0, R2		// 2
	SHL R2			// 2
	ADD R0, R2		// 2
	
	// Return.
	MOV R3, [ST]	// 3
	MOV R2, [ST]
	// endregion
	MOV PC, [ST]


	// Performs a single iteration of double dabble addition.
	// Takes input and returns in R0.
	// Takes 39 cycles.
double_dabble_bitwise1:
	// region
	MOV [ST], R1	// 3
	MOV [ST], R2
	
	// OR bits 0 and 1 into bit 1 of R1.
	LEA R1, [R0+R0]	// 3
	OR  R1, R0		// 2
	
	// AND bit 2 from R0 with bit 1 of R1.
	MOV R2, R0		// 2
	SHR R2			// 2
	AND R1, R2		// 2
	
	// OR bit 3 into bit 1 of R1.
	SHR R2			// 2
	OR  R1, R2		// 2
	AND R1, 0x2222	// 3
	
	// Add to R0.
	ADD R0, R1		// 2
	SHR R1			// 2
	ADD R0, R1		// 2
	
	// Return.
	MOV R2, [ST]	// 3
	MOV R1, [ST]
	// endregion
	MOV PC, [ST]


	// Performs a single iteration of double dabble addition.
	// Takes input and returns in R0.
	// Takes 53 to 65 cycles.
double_dabble_simple:
	// region
	MOV [ST], R1
	
	// Addition 1/4.
	MOV R1, R0				// 2
	AND R1, 0x000f			// 3
	CMP R1, 4				// 3
	LEA.ULE PC, [PC~.skip0]	// 3
	ADD R0, 0x0003			// 3?
.skip0:
	
	// Addition 2/4.
	MOV R1, R0
	AND R1, 0x00f0
	CMP R1, 4
	LEA.ULE PC, [PC~.skip1]
	ADD R0, 0x0030
.skip1:
	
	// Addition 3/4.
	MOV R1, R0
	AND R1, 0x0f0
	CMP R1, 4
	LEA.ULE PC, [PC~.skip2]
	ADD R0, 0x030
.skip2:
	
	// Addition 4/4.
	MOV R1, R0
	AND R1, 0xf000
	CMP R1, 4
	LEA.ULE PC, [PC~.skip3]
	ADD R0, 0x3000
.skip3:
	
	// Return.
	MOV R1, [ST]
	// endregion
	MOV PC, [ST]



	.section ".rodata"
hexstr:
	.db "0123456789abcdef"

	.section ".bss"
temp:
	.zero 8
