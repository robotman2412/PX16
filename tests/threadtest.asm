
	// IRQ vector.
	.db isr_ignore
	// NMI vector.
	.db isr_switch
	// Entry vector.
	.db entry

// region tasks


	// Dummy ISR that ignores everything.
	.section ".text"
isr_ignore:
	MOV PF, [ST]
	MOV PC, [ST]


	// ISR that invokes context switching.
isr_switch:
	// Store context (registers).
	MOV R0, [ST]
	MOV R1, [ST]
	MOV R2, [ST]
	MOV R3, [ST]
	// Debug trap.
	MOV [0xfffe], 0x1d00
	// Invoke the (very dumb) scheduler to change the stack register.
	MOV.JSR PC, scheduler
.schedret:
	// Start, ack. irq, zero timer, enable NMI.
	MOV [0xfffe], 0x0d03
	// Notify.
	MOV [0xfff6], 0x53
	MOV [0xfff6], 0x77
	MOV [0xfff6], 0x0a
	
	// Load other context.
	MOV R3, [ST]
	MOV R2, [ST]
	MOV R1, [ST]
	MOV R0, [ST]
	// Return from interrupt.
	MOV PF, [ST]
	MOV PC, [ST]


	// A very simple scheduler that toggles between two threads.
scheduler:
	// Simple swap.
	MOV R0, ST
	MOV ST, [otherthread]
	MOV [otherthread], R0
	// Return.
	MOV PC, [ST]


// endregion tasks


// region entry

entry:
	// Disable timer.
	MOV [0xfffe], 0
	XOR R0, R0
	
	// Set time to 0.
	MOV [0xfffa], R0
	MOV [0xfffb], R0
	// Set limit.
	MOV [0xfff8], 1000
	MOV [0xfff9], R0
	
	// Create dummy stack for thread A.
	MOV ST, 0xe000
	MOV [ST], thread_a
	MOV [ST], 0x0001
	MOV [ST], R0
	MOV [ST], R0
	MOV [ST], R0
	MOV [ST], R0
	MOV [ST], isr_switch.schedret
	MOV [otherthread], ST
	
	// Start, ack. irq, zero timer, enable NMI.
	MOV [0xfffe], 0x0d03
	
	// Enable NMI.
	MOV ST, 0xd000
	MOV PF, 0x0001
	
	// We are now thread B, go to that loop.
	MOV PC, thread_b

// endregion entry


// region main

	// Print a message.
print:
	MOV PC, .check
	MOV [ST], R1
.loop:
	MOV R1, [R0]
	MOV [0xfff6], R1
	INC R0
.check:
	CMP1 [R0]
	MOV R1, [ST]
	MOV.UGE PC, .loop


	// Idle for 4+5*R0 cycles.
idle:
	// DEC PC
	DEC R0
	MOV.CS PC, idle
	MOV PC, [ST]


	// Thread A loop.
thread_a:
	// Set R3 to B to indicate thread.
	MOV R3, 0xB0B0
.loop:
	// Print message.
	MOV R0, msg_a
	MOV.JSR PC, print
	// Idle for 100.
	MOV R0, 100
	MOV.JSR PC, idle
	// Loop back.
	MOV PC, .loop


	// Thread B loop.
thread_b:
	// Set R3 to A to indicate thread.
	MOV R3, 0x0A0A
.loop:
	// Print message.
	MOV R0, msg_b
	MOV.JSR PC, print
	// Idle for 200.
	MOV R0, 200
	MOV.JSR PC, idle
	// Loop back.
	MOV PC, .loop

// endregion main


// region data

// Read-only in section .rodata, after .text:
	.section ".rodata"

// Message for thread A.
msg_a:
	.db "Thread A message!\n", 0

// Message for thread B.
msg_b:
	.db "Thread B message!\n", 0

// Variables in section .bss, at offset 0x8000:
	.section ".bss"

// The not-currently-running thread.
otherthread:
	.zero 1

// endregion data
