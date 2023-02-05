
	.equ timerctl, 0xfffe
	.equ timer0, 0xfffa
	.equ limit0, 0xfff8
	.equ intsense, 0xfff4

	// .global _px_timer_msb
	// .global _px_timer_type
	// .global _px_timer_thread_init



	// Set timer limit.
	// void _px_timer_limit(uint32_t limit, timer_type_t type);
	// .global _px_timer_limit
_px_timer_limit:
	// #region
	// Disable interrupts.
	MOV [ST], PF
	AND PF, 0xfffc
	
	// Disable timer0 interrupts.
	MOV [timerctl], 0x0001
	// Set limit0.
	MOV [limit0+0], R0
	MOV [limit0+1], R1
	// Re-enable timer0 interrupts.
	MOV [timerctl], 0x0003
	
	// Update _px_timer_type.
	MOV [_px_timer_type], R2
	
	// Restore PF and return.
	MOV PF, [ST]
	MOV PC, [ST]
	// #endregion



	// Initialise timer system, start counting from 0.
	// void px_timer_init_0();
	// .global px_timer_init_0
px_timer_init_0:
	// #region
	MOV [ST], R0
	// Disable interrupts.
	MOV [ST], PF
	AND PF, 0xfffc
	
	// Create timer callbacks thread.
	LEA.JSR PC, [PC~_px_timer_thread_init]
	
	// Clear timer0.
	XOR R0, R0
	MOV [timer0+0], R0
	MOV [timer0+1], R0
	// Clear limit0.
	MOV [limit0+0], R0
	MOV [limit0+1], R0
	// Clear timer MSB.
	MOV [PC~_px_timer_msb+0], R0
	MOV [PC~_px_timer_msb+1], R0
	
	// Ack timer interrupt, write and enable timer0.
	MOV [timerctl], 0x0501
	// Enable timer interrupts later to avoid triggering overflow early.
	MOV [timerctl], 0x0003
	
	MOV R0, [ST]
	// Restore PF and return.
	MOV PF, [ST]
	MOV PC, [ST]
	// #endregion



	// Read timer microseconds (32-bit; prone to overflow every 71 minutes).
	// uint32_t px_time32();
	// .global px_time32
px_time32:
	// #region
	// Time adjustment to account for time taken running this function.
	.equ .adj, 0
	// Disable interrupts.
	MOV [ST], PF
	AND PF, 0xfffc
	
	// Tell timer0 to latch.
	OR  [timerctl], 0x0200
	
	// Read timer0
	MOV R0, [timer0]
	MOV R1, [timer0+1]
	// Add time adjustment.
	ADD R0, .adj
	INCC R1
	
	// Restore PF and return.
	MOV PF, [ST]
	MOV PC, [ST]
	// #endregion



	// Read timer microseconds (64-bit).
	// uint64_t px_time64();
	// .global px_time64
px_time64:
	// #region
	// Time adjustments to account for time taken running this function.
	.equ .adj0, 0
	.equ .adj1, 0
	.equ .adj2, 0
	
	// Disable interrupts.
	MOV [ST], PF
	AND PF, 0xfffc
	
	// Tell timer0 to latch.
	OR  [timerctl], 0x0200
	
	// Read timer0
	MOV R0, [timer0+0]
	MOV R1, [timer0+1]
	
	// Check for timer interrupt.
	MOV R2, [intsense]
	AND R2, 0x0202
	LEA.EQ PC, [PC~.normal] // 40
	// Check timer limit against 0.
	CMP1 [limit0+0]
	CMP1C [limit0+1]
	LEA.NE PC, [PC~.adjusted] // 34
	
	// When limit is 0, that's the overflow detect.
	// Concatenate timer MSB.
	MOV R2, [PC~_px_timer_msb+0]
	MOV R3, [PC~_px_timer_msb+1]
	// Add more time adjustment + 1<<32
	ADD R0, .adj0
	INCC R1
	ADDC R2, 1 // This is 1<<32, to correct for overflow.
	INCC R3
	// Restore PF and return.
	MOV PF, [ST]
	MOV PC, [ST]
	
.adjusted:
	// Only part of the check succeeded.
	// Concatenate timer MSB.
	MOV R2, [PC~_px_timer_msb+0]
	MOV R3, [PC~_px_timer_msb+1]
	// Use different time adjustment.
	ADD R0, .adj1
	INCC R1
	INCC R2
	INCC R3
	// Restore PF and return.
	MOV PF, [ST]
	MOV PC, [ST]
	
.normal:
	// Concatenate timer MSB.
	MOV R2, [PC~_px_timer_msb+0]
	MOV R3, [PC~_px_timer_msb+1]
	// Add normal time adjustment.
	ADD R0, .adj0
	INCC R1
	INCC R2
	INCC R3
	
	// Restore PF and return.
	MOV PF, [ST]
	MOV PC, [ST]
	// #endregion
