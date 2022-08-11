
	// IRQ vector.
	.db 0
	// NMI vector.
	.db 0
	// Entry vector.
	.db entry

	// Code entrypoint.
entry:
	MOV ST, 0xffff
	SUB ST, [0xffff]
	
	MOV R0, msg0
	MOV.JSR PC, print
	
	// Discount halt instruction.
	DEC PC

msg0:
	.db "Starting the pie test.\n", 0

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

