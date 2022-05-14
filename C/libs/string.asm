
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
