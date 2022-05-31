
// ===== Signed multiplication ===== //

	// Multiplies R0 by R1, result in R0.
	// Signed multiply.
__px16_mul_i:
	MOV [ST], R2
	// Determines sign later.
	XOR R2, R2
	// Check whether R0 is negative.
	CMP R0, 0
	MOV.SGE PC, .skipinva
	// Make R0 positive.
	XOR R0, 0xffff
	INC R0
	XOR R2, 1
.skipinva:
	// Check whether R1 is negative.
	CMP R1, 0
	MOV.SGE PC, .skipinvb
	// Make R1 positive.
	XOR R1, 0xffff
	INC R1
	XOR R2, 1
.skipinvb:
	// Preserve R2.
	MOV [ST], R2
	// Call the unsigned multiply.
	MOV.JSR PC, __px16_mul_u
	// Restore R2.
	MOV R2, [ST]
	// Check the sign bit.
	CMP1 R2
	MOV.ULT PC, .exit
	// And now we fix the sign again.
	XOR R0, 0xffff
	INC R0
.exit:
	MOV R2, [ST]
	MOV PC, [ST]



	// Multiplies R0 R1 by R2 R3, result in R0 R1.
	// Signed multiply.
__px16_mul_li:
	// Determines sign later.
	MOV [ST], 0
	// Check whether A is negative.
	// Checking R0 is unnecessary.
	CMP R1, 0
	MOV.SGE PC, .skipinva
	// Make A positive.
	XOR R0, 0xffff
	XOR R1, 0xffff
	INC R0
	INCC R1
	XOR [ST+1], 1
.skipinva:
	// Check whether B is negative.
	CMP R2, 0
	MOV.SGE PC, .skipinvb
	// Make B positive.
	XOR R2, 0xffff
	XOR R3, 0xffff
	INC R2
	INCC R3
	XOR [ST+1], 1
.skipinvb:
	// Call the unsigned multiply.
	MOV.JSR PC, __px16_mul_lu
	// Check the sign bit.
	MOV R2, [ST]
	CMP2 R2
	MOV.ULT PC, .exit
	// Fix the sign.
	XOR R0, 0xffff
	XOR R1, 0xffff
	INC R0
	INCC R1
.exit:
	MOV PC, [ST]



	// Multiplies [ST+5]...[ST+8] by [ST+1]...[ST+4], result in R0...R3.
	// Signed multiply.
__px16_mul_lli:
	// Determines sign later.
	XOR R0, R0
	// Check whether A is negative.
	CMP [ST+8], 0
	MOV.SGE PC, .skipinva
	// Make A positive.
	XOR [ST+5], 0xffff
	XOR [ST+6], 0xffff
	XOR [ST+7], 0xffff
	XOR [ST+8], 0xffff
	INC [ST+5]
	INCC [ST+6]
	INCC [ST+7]
	INCC [ST+8]
	XOR R0, 1
.skipinva:
	// Check whether B is negative.
	CMP [ST+4], 0
	MOV.SGE PC, .skipinva
	// Make B positive.
	XOR [ST+1], 0xffff
	XOR [ST+2], 0xffff
	XOR [ST+3], 0xffff
	XOR [ST+4], 0xffff
	INC [ST+1]
	INCC [ST+2]
	INCC [ST+3]
	INCC [ST+4]
	XOR R0, 1
.skipinvb:
	MOV [ST], R0
	// It turns out we need to allocate more stack space...
	// So there's this memory copy.
	MOV R0, [ST+9]
	MOV [ST], R0
	MOV R0, [ST+9]
	MOV [ST], R0
	MOV R0, [ST+9]
	MOV [ST], R0
	MOV R0, [ST+9]
	MOV [ST], R0
	MOV R0, [ST+9]
	MOV [ST], R0
	MOV R0, [ST+9]
	MOV [ST], R0
	MOV R0, [ST+9]
	MOV [ST], R0
	MOV R0, [ST+9]
	MOV [ST], R0
	// Call the unsigned multiply.
	MOV.JSR PC, __px16_mul_llu
	// Discard extra stack things.
	ADD ST, 8
	// Check the sign bit.
	CMP1 [ST+1]
	MOV.ULT PC, .exit
	// Fix the sign.
	XOR R0, 0xffff
	XOR R1, 0xffff
	XOR R2, 0xffff
	XOR R3, 0xffff
	INC R0
	INCC R1
	INCC R2
	INCC R3
.exit:
	// Remove the sign bit from the stack.
	INC ST
	MOV PC, [ST]



// ==== Unsigned multiplication ==== //

	// Multiplies R0 by R1, result in R0.
	// Unsigned multiply.
__px16_mul_u:
	XOR R2, R2
.loop:
	CMP1 R0
	MOV.ULT PC, .exit
	SHR R0
	MOV.CC  PC, .skipadd
	ADD R2, R1
.skipadd:
	SHL R1
	MOV     PC, .loop
.exit:
	MOV R0, R2
	MOV PC, [ST]



	// Multiplies R0 R1 by R2 R3, result in R0 R1.
	// Unsigned multiply.
__px16_mul_lu:
	// Allocate some result.
	MOV [ST], 0
	MOV [ST], 0
.loop:
	// If A == 0 then stop.
	CMP1 R0
	CMP1C R1
	MOV.ULT PC, .exit
	// CONSUME a bit of A.
	SHR R1
	SHRC R0
	// Add... ?
	MOV.CC  PC, .skipadd
	ADD [ST+1], R2
	ADDC [ST+2], R3
.skipadd:
	// Shift left B.
	SHL R2
	SHLC R3
	MOV PC, .loop
.exit:
	// Copy back result.
	MOV R0, [ST]
	MOV R1, [ST]
	MOV PC, [ST]



	// Multiplies [ST+5]...[ST+8] by [ST+1]...[ST+4], result in R0...R3.
	// Unsigned multiply.
__px16_mul_llu:
	// Clear result.
	XOR R0, R0
	XOR R1, R1
	XOR R2, R2
	XOR R3, R3
.loop:
	// If multiplier == 0 then stop.
	CMP1 [ST+5]
	CMP1C [ST+6]
	CMP1C [ST+7]
	CMP1C [ST+8]
	MOV.ULT PC, .exit
	// Consume a bit of A.
	SHR [ST+4]
	SHRC [ST+3]
	SHRC [ST+2]
	SHRC [ST+1]
	// Add to result?
	MOV.CC PC, .skipadd
	ADD R0, [ST+1]
	ADDC R1, [ST+2]
	ADDC R2, [ST+3]
	ADDC R3, [ST+4]
.skipadd:
	// Shift left B.
	SHL [ST+1]
	SHLC [ST+2]
	SHLC [ST+3]
	SHLC [ST+4]
	MOV PC, .loop
.exit:
	// No need to copy back result.
	MOV PC, [ST]
