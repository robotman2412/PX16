
	// IRQ vector.
	.db irqv
	// NMI vector.
	.db nmiv
	// Entry vector.
	.db entry

	// Code entrypoint.
entry:
	SUB ST, [0xffff]
    // Enable IRQ and NMI.
    OR  PF, 0x0003
.loop:
    MOV R0, 0x1337
    MOV PC, .loop

    // IRQ Handler.
irqv:
    MOV R0, 0xcafe
    MOV PF, [ST]
    MOV PC, [ST]

    // NMI Handler.
nmiv:
    MOV R0, 0xbeef
    MOV PF, [ST]
    MOV PC, [ST]
