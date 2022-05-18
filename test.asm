
	// IRQ vector.
	.db 0
	// NMI vector.
	.db 0
	// Entry vector.
	.db entry

	// Code entrypoint.
entry:
	; Discount halt instruction.
	DEC PC
