
	; SUB     [R0], R1
	
	; Y X B A  O
	; 0 5 1 0 01
	
	; YXXX BBBA AAOO OOOO
	; 0101 0010 0000 0001
	; 0x5201

	; Integer minimum.
	; args:
	;   R0, R1: integers
	; returns:
	;   R0: the smaller of the two integers.
imin:
	; Compare R0 to R1.
	CMP		R0, R1
	; Copy R1 to R0 if R0 > R1.
	MOV.SGT	R0, R1
	; Return.
	RET


	; Find last occurance of the given character in the given string.
	; args:
	;   R0: String pointer.
	;   R1: Character to find.
	; returns:
	;   R0: Pointer to found character.
strrchr:
	; Make space for LAST ADDRESS.
	PSH		R2
	MOV		R2, 0
.loop:
	; Compare against desired bullshit.
	CMP		[R0], R1
	MOV.EQ	R2, R0
	; Compare against zero.
	CMP		[R0], 0
	BNE		.loop
	
	; Finally.
	MOV		R0, R2
	POP		R2
	RET


	; Find first occurance of the given character in the given string.
	; args:
	;   R0: String pointer.
	;   R1: Character to find.
	; returns:
	;   R0: Pointer to found character.
strchr:
.loop:
	; Compare against desired bullshit.
	CMP		[R0], R1
	BEQ		.yeems
	; Compare against zero.
	CMP		[R0], 0
	BNE		.loop
	
	; We did not find it.
	MOV		R0, 0
	
.yeems:
	; Return.
	RET


	; Find string length.
	; args:
	;   R0: String pointer.
	; returns:
	;   R0: Length.
strlen:
	; Save initial pointer for later.
	PSH		R0
	; Pre-decrement because we pre-increment in the loop.
	DEC		R0
	
.loop:
	; Increment pointer.
	INC		R0
	; Compare against zero.
	CMP		[R0], 0
	; Branch back if not zero.
	BNE		.loop
	
	; Subtract initial from R0 to get length.
	; This by means of using [ST] to pop the value saved earlier.
	SUB		R0, [ST]
	; Return.
	RET
