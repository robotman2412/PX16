

// the test
	.db entry, entry, entry


entry:
	MOV ST, 0xffff
	SUB ST, [0xffff]
	
	// Indexing?
	MOV R0, hello
	MOV R1, 0x6c
	MOV R2, 14
	MOV.JSR PC, memchr
	MOV R1, R0
	SUB R1, hello
	
	DEC PC


print:
	MOV [ST], R1
	LEA PC, [PC~.check]
.loop:
	MOV R1, [R0]
	MOV [0xfff6], R1
	INC R0
.check:
	CMP1 [R0]
	LEA.UGE PC, [PC~.loop]
	MOV R1, [ST]
	MOV PC, [ST]


.section ".rodata"

hello:
	.db "Hello, World!", 0
hello1:
	.db "Hello, World!", 0
hello2:
	.db "hELLO, worlD!", 0

.section ".bss"

buf:
	.zero 1024
buf1:
	.zero 1024

.section ".text"

	// Copy N (R2) bytes of SRC (R1) to DST (R0).
	// Performs a reverse-iteration copy.
memcpy:
	DEC R2				// Initial length check.
	MOV.CC PC, .skip	// R3 not modified when skipped.
.join:					// Label specifically for memmove, to skip length check.
	MOV [ST], R3
	
.loop:
	MOV R3, [R1+R2]		// Transfer memory.
	MOV [R0+R2], R3
	DEC R2				// Next and check combined.
	MOV.CS PC, .loop
	
	MOV R3, [ST]		// Return.
.skip:
	MOV PC, [ST]



	// Copy N (R2) bytes of SRC (R1) to DST (R0).
	// Gaurantees correct copy for overlapping data.
memmove:
	CMP1 R2					// Initial length check.
	
	MOV.UGE PC, .dircheck
	MOV PC, [ST]			// Skipped.
	
.dircheck:
	CMP R0, R1				// Iteration direction check.
	MOV.ULT PC, memcpy.join	// Reverse iteration tail call.
	MOV [ST], R3
	
	// Forward iteration setup.
	ADD R0, R2				// Add R2+1 to R0 because we're about to turn it into -R2 - 1.
	INC R0
	ADD R1, R2				// Same for R1.
	INC R1
	XOR R2, 0xffff			// Convert R2 to a negative number to allow for one instruction increment and check.
	
.loop:
	MOV R3, [R1+R2]			// Transfer memory.
	MOV [R0+R2], R3
	INC R2					// Next and check combined.
	MOV.CC PC, .loop
	
	MOV R3, [ST]			// Return.
	MOV PC, [ST]



	// Set N (R2) bytes of S (R0) to C (R1).
memset:
	DEC R2				// Initial length check.
	MOV.CC PC, .skip
	
.loop:
	MOV [R0+R2], R1		// Fill memory.
	DEC R2				// Next and check combined.
	MOV.CS PC, .loop
	
.skip:					// Return.
	MOV PC, [ST]



	// Compare N (R2) bytes of A (R0) to B (R1).
memcmp:
	CMP1 R2					// Initial length check.
	MOV.ULT PC, .skip		// R3 not modified when skipped.
	MOV [ST], R3
	
	ADD R0, R2				// Add R2 to R0 because we're about to turn it into -R2.
	ADD R1, R2				// Same for R1.
	XOR R2, 0xffff			// Convert R2 to a negative number to allow for one instruction increment and check.
	INC R2
	
.loop:
	MOV R3, [R0+R2]			// Compare memory.
	SUB R3, [R1+R2]
	MOV.NE PC, .exit
	INC R2					// Next and check combined.
	MOV.CC PC, .loop
	
.exit:
	MOV R0, R3
	MOV R3, [ST]			// Return.
	MOV PC, [ST]
	
.skip:
	XOR R0, R0			// Return zero.
	MOV PC, [ST]



	// Search N (R2) bytes of S (R0) for C (R1).
	// FIXME
memchr:
	CMP1 R2					// Initial length check.
	MOV.ULT PC, .skip		// R3 not modified when skipped.
	
	ADD R0, R2				// Add R2 to R0 because we're about to turn it into -R2.
	ADD R1, R2				// Same for R1.
	XOR R2, 0xffff			// Convert R2 to a negative number to allow for one instruction increment and check.
	INC R2
	
.loop:
	CMP [R0+R2], R1			// Compare memory.
	MOV.EQ PC, .match
	INC R2					// Next and check combined.
	MOV.CC PC, .loop
	
.skip:
	XOR R0, R0				// Return zero.
	MOV PC, [ST]
	
.match:
	LEA R0, [R0+R2]			// Return address of match.
	MOV PC, [ST]

// TODO: Untested past this point

	// Search N (R2) bytes of S (R0) for the last occurance of C (R1).
memrchr:
	DEC R2					// Initial length check.
	MOV.CC PC, .skip		// R3 not modified when skipped.
	
.loop:
	CMP [R0+R2], R1			// Compare memory.
	MOV.EQ PC, .match
	DEC R2					// Next and check combined.
	MOV.CS PC, .loop
	
.skip:
	XOR R0, R0				// Return zero.
	MOV PC, [ST]
	
.match:
	LEA R0, [R0+R2]			// Return address of match.
	MOV PC, [ST]



	// Copy SRC (R1) to DST (R0).
strcpy:
	MOV [ST], R2			// Simple function entry.
	SUB R0, R1
	LEA PC, [PC~.check]
	
.loop:
	MOV R2, [R0+R1]			// Memory transfer.
	MOV [R1], R2
	INC R1					// INCERMENT.
	
.check:
	CMP1 [R0+R1]			// Perform length check on SRC.
	LEA.ULT PC, [PC~.exit]
	CMP1 [R1]				// Perform length check on DST.
	LEA.UGE PC, [PC~.loop]
	
.exit:
	ADD R0, R1				// Return dest.
	MOV R2, [ST]
	MOV PC, [ST]



	// Copy no more than N (R2) words of SRC (R1) to DST (R0).
strncpy:
	MOV [ST], R3			// Simple function entry.
	CMP1 R2
	LEA.UGE PC, [PC~.prep]
	MOV R3, [ST]
	MOV PC, [ST]
	
.prep:
	MOV [ST], R2			// Out of registers; copy limit to stack.
	ADD R0, R2				// Do overflow based forward iteration.
	ADD R0, R1
	XOR R2, 0xffff
	INC R2
	// 
	
.loop:
	MOV R3, [R1+R2]			// Memory transfer.
	MOV [R0+R2], R3



	// Find string length for pointer in R0, returns length in R0.
strlen:
	MOV [ST], R1
	// Copy the pointer to R1.
	MOV R1, R0
	MOV PC, .check
.loop:
	// On to the next word.
	INC R0
.check:
	// Check a word.
	CMP1 [R0]
	MOV.UGE PC, .loop
	// Calculate length.
	SUB R0, R1
	MOV R1, [ST]
	MOV PC, [ST]



	// Find first occurrance of word R1 in string R0.
	// Returns found pointer in R0, or null if none.
strchr:
	MOV PC, .check
.loop:
	// On to the next word.
	INC R0
.check:
	// Check against target.
	CMP [R0], R1
	MOV.EQ  PC, .found
	// Check against null.
	CMP1 [R0]
	MOV.UGE PC, .loop
	// Found nothing.
	XOR R0, R0
.found:
	MOV PC, [ST]



	// Find last occurrance of word R1 in string R0.
	// Returns found pointer in R0, or null if none.
strrchr:
	MOV [ST], R2
	XOR R2, R2
	MOV PC, .check
.loop:
	// On to the next word.
	INC R0
.check:
	// Check against target.
	CMP [R0], R1
	MOV.NE  PC, .notfound
	MOV R2, R0
.notfound:
	// Check against null.
	CMP1 [R0]
	MOV.UGE PC, .loop
	// Return result.
	MOV R0, R2
	MOV R2, [ST]
	MOV PC, [ST]
