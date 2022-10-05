
	// Copy N (R2) bytes of SRC (R1) to DST (R0)
memcpy:
	DEC R2				// Initial length check.
	MOV.CC PC, .skip
	MOV [ST], R3		// R3 not modified when skipped.
	
.loop:
	MOV R3, [R1+R2]		// Transfer memory.
	MOV [R0+R2], R3
	DEC R2				// Next and check combined.
	MOV.CS PC, .loop
	
	MOV R3, [ST]		// Return.
.skip:
	MOV PC, [ST]



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
strchr:
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
