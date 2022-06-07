
	// IRQ vector.
	.db 0
	// NMI vector.
	.db 0
	// Entry vector.
	.db entry

	// Code entrypoint.
entry:
	MOV R0, 5
	ADD R0, 0xffff
	// Discount halt instruction.
.loop:
	MOV PC, .loop
