
// IRQ vector
	.db entry
// NMI vector
	.db entry
// Entry vector.
	.db entry

entry:
	// Initialise stack.
	MOV ST, 0xffff
	SUB ST, [0xffff]
	
	// Say we are alive.
	LEA R0, [PC~msg0]
	MOV.JSR PC, print
	
	// Decide were to put the pie code.
	LEA R0, [PC~piestart]
	LEA R1, [PC~pieend]
	MOV R2, 0x8000
	
	// Copy the pie of testing.
	LEA PC, [PC~.check]
.loop:
	MOV R3, [R0]
	MOV [R2], R3
	INC R0
	INC R2
.check:
	CMP R0, R1
	LEA.ULT PC, [PC~.loop]
	
	// Rer?
	MOV R0, msg1
	MOV.JSR PC, print
	
	// Test whether it copied correctly.
	MOV.JSR PC, 0x8000
	
halt:
	MOV [0xfffe], 0x1000
	LEA PC, [PC~halt]

msg0:
	.db "Starting the pie test.\n", 0
msg1:
	.db "Memory copied.\n", 0

print:
	MOV [ST], R1
	MOV PC, .check
.loop:
	MOV R1, [R0]
	MOV [0xfff6], R1
	INC R0
.check:
	CMP1 [R0]
	MOV.UGE PC, .loop
	MOV R1, [ST]
	MOV PC, [ST]

piestart:
	LEA R0, [PC~piemsg]
	MOV.JSR PC, print
piemsg:
	.db "This is data in the pie test!\n", 0
pieend:
