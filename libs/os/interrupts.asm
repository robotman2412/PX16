
	.equ timerctl, 0xfffe
	.equ timer0, 0xfffa
	.equ limit0, 0xfff8
	.equ intsense, 0xfff4
	
	// .global _px_timer_reached

	// Interrupt service routine.
	// Handles timers with highest priority.
	// void _px_isr();
	// .global _px_isr
_px_isr:
	// #region
	MOV [ST], R0
	
	// Check for timer interrupt.
	MOV R0, [intsense]
	AND R0, 0x0202
	LEA.EQ PC, [PC~.not_timer]
	
	// Handle timers.
	LEA.JSR PC, [PC~_px_timer_reached]
.not_timer:
	
	// Return from interrupt.
	MOV R0, [ST]
	MOV PF, [ST]
	MOV PC, [ST]
	// #endregion
