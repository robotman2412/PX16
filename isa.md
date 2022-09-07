![Logo image](logo_small.png)

# Pixie 16 instruction set

Pixie 16 is a RISC 16-bit CPU with a 16-bit address space.
It addresses 65536 times 16-bit words (bytes unsupported).

Also referred to by "PX16", it is a von neumann architecture, which means that instructions and data use the same bus.
Pixie 16 reads memory during instruction loading time, and the actual execution of opcodes themselves takes only one of the on average four cycles it takes to run an instruction.

Instructions take between 2 (minimum) and 5 (maximum) cycles to run,
and instructions with constant parameter have the parameter words immediately follow the instruction word.

PX16 does not necessarily need to deal with endianness, but is specified to be little-endian (lowest word comes first in memory) when applicable. Therfore, PX16 binaries stored on disk should also be little-endian when they use their two bytes to encode a 16-bit word.

The character set is ASCII and text outside that range should be UTF-16 little-endian unicode.

# Registers
Pixie  has 4 general and 3 special registers:
| name | code | description
| :--- | :--- | :----------
| R0   | 00   | General-purpose register.
| R1   | 01   | General-purpose register.
| R2   | 02   | General-purpose register.
| R3   | 03   | General-purpose register.
| ST   | 04   | Stack pointer.
| PF   | 05   | Flags register.
| PC   | 06   | Program counter.

# Encoding
The instruction is comprised of 5 bitfields filling up a word:
| name | range | bitmask               | description
| :--- | :---- | :-------------------- | :----------
| `y`  | 15    | `y... .... .... ....` | Addressing selector.
| `x`  | 14-12 | `.xxx .... .... ....` | Addressing mode.
| `b`  | 11-9  | `.... bbb. .... ....` | Second "B" operand.
| `a`  | 8-6   | `.... ...a aa.. ....` | First "A" operand.
| `o`  | 5-0   | `.... .... ..oo oooo` | Opcode.
- `y` selects which operand `x` will apply to:
  - `y=0` means `x` applies to A,
  - `y=1` means `x` applies to B.
- `x` describes the addressing mode to use ([see: Addressing](#addressing))
- `b` and `a` describe which register to use for their respective operands.
- `o` describes which opcode to run ([see: Opcodes](#opcodes))

# Opcodes
Pixie 16 has 4 major categories of instructions, filling `o` field of the instruction:
| type  | start | end | description
| :---- | :---- | :-- | :----------
| MATH2 | 000   | 017 | Math with two operands.
| MATH1 | 020   | 037 | Math with one operand.
| MOV   | 040   | 057 | Data move, with optional condition.
| LEA   | 060   | 077 | Load effective address, more restrictive.

## MATH2 type opcodes
MATH2 opcodes take 2 parameters and modify the A parameter with the result.
`CMP` is an exception to this rule.

The code is for the `o` field with carry in = default.
The CC code is with carry in = carry out flag.
| name | code | code (CC) | description
| :--- | :--- | :-------- | :----------
| ADD  | 000  | 010       | Adds B to A.
| SUB  | 001  | 011       | Subtracts B from A.
| CMP  | 002  | 012       | Compares A to B for use in conditional MOV or LEA.
| AND  | 003  | 013       | Bitwise ANDs A by B.
| OR   | 004  | 014       | Bitwise ORs A with B.
| XOR  | 005  | 015       | Bitwise XORs A by B.

## MATH1 type opcodes
MATH1 opcodes take 1 parameter and ignore the B parameter, which is substituted with 1.
Again, `CMP1` does not modify the A parameter.

| name | code | code (CC) | description
| :--- | :--- | :-------- | :----------
| INC  | 020  | 030       | Adds 1 to A.
| DEC  | 021  | 031       | Subtracts 1 from A.
| CMP1 | 022  | 032       | Compares A against 1.
| SHL  | 026  | 036       | Bitwise shifts A left one position. (A = A * 2)
| SHR  | 027  | 037       | Bitwise shifts A right one position. (A = A / 2)

## MOV type opcodes
MOV opcodes mostly do the same thing: copy B into A.
However, MOV opcodes also have a condition to satisfy, which enables their behaviour (except push/pop which always happens).

| name    | code | description
| :------ | :--- | :----------
| MOV.ULT | 040  | Condition: A < B (unsigned).
| MOV.UGT | 041  | Condition: A > B (unsigned).
| MOV.SLT | 042  | Condition: A < B (signed).
| MOV.SGT | 043  | Condition: A > B (signed).
| MOV.EQ  | 044  | Condition: A = B (zero flag = 1).
| MOV.CS  | 045  | Condition: carry out flag = 1 (unsigned).
| MOV     | 046  | Unconditional.
| MOV.UGE | 050  | Condition: A >= B (unsigned).
| MOV.ULE | 051  | Condition: A <= B (unsigned).
| MOV.SGE | 052  | Condition: A >= B (signed).
| MOV.SLE | 053  | Condition: A <= B (signed).
| MOV.NE  | 054  | Condition: A != B (zero flag = 0).
| MOV.CC  | 055  | Condition: carry out flag = 0 (unsigned).
| MOV.JSR | 056  | Special: jump to subroutine (push PC to stack before performing MOV).
| MOV.CX  | 057  | Special: carry extend (A is filled with highest bit of B).

## LEA type opcodes
LEA opcodes are identical to MOV, with the exception that `LEA.CX` doesn't exist.
LEA opcodes load the effective address of B into A, instead of the value stored there.
This means that LEA always has a _register_ for A and always has a _memory operand_ for B.

# Adressing
PX16 opcodes aren't exceptionally smart on their own, and programs will heavily rely on more complex addressing features to compensate.

All 7 registers can be used in all instructions, but an extra `imm` register appears here.
These have code 07, and are written as any one of: `label`, `label+0123`, `0123`, etc.

The `imm` registers are initialised to the specified constant at instruction load time, and there is one `imm` for operand A, and another for operand B. Storing to the `imm` registers results in undefined behaviour, which makes `cmp` and `cmp1` the only valid instructions with `imm` for operand A.

The registers are then combined with one of the following addressing modes, which is stored in the `x` field:
| type    | syntax       | code | description
| :------ | :----------- | :--- | :----------
| Memory  | `[... + R0]` | 00   | Address is value of `R0` + the affected operand.
| Memory  | `[... + R1]` | 01   | Address is value of `R1` + the affected operand.
| Memory  | `[... + R2]` | 02   | Address is value of `R2` + the affected operand.
| Memory  | `[... + R3]` | 03   | Address is value of `R3` + the affected operand.
| Memory  | `[... + ST]` | 04   | Address is value of `ST` + the affected operand.
| Memory  | `[...]`      | 05   | Address is value of the affected operand.
| Memory  | `[... + PC]` | 06   | Address is value of `PC` + the affected operand.
| Register | `...`       | 07   | No change to affected operand.

# The stack
When combined, one of the addressing modes has a special effect:
The stack push/pop operation.
Represented by `[ST]`, this pushes to the stack when the A parameter, or pops from it when the B parameter.
This mode doesn't make sense to use in `MATH1` or as A parameter to `MATH2`.

The stack grown down in the address space in a pre-decrement way.
This means that the top value in the stack is at `[ST+0x0000]`.

# Implementation of functions
PX16 functions follow simple calling conventions
- When the arguments total up to 4 words, registers are used for passing the arguments.
- Otherwise, the stack is used, where the last parameter is pushed first.
- On return, the same rules apply to the return value.
- Any registers not used to pass args or return should be preserved by the callee.

Here is a typical function based on the following C:
```c
int multiplyByTwo(int value) {
    return value << 1;
}
```
And assembly:
```
multiplyByTwo:
    // Preserve other registers.
    MOV [ST], R3
    MOV [ST], R2
    MOV [ST], R1
    // value << 1
    SHL R0
    // Restore other registers.
    mov R1, [ST]
    mov R2, [ST]
    mov R3, [ST]
    // return
    MOV PC, [ST]
```

# Interrupt handling
When an interrupt occurs, the PC followed by PF are pushed to the stack.
Then, [0x0000] (for NMI type) or [0x0001] (for IRQ type) is loaded into the PC.

At this point, it is up to the interrupt handler to do it's things, after which it returns with the following:
```
    // Restore flags, which reenables interrupts after the return.
    MOV PF, [ST]
    // Return by popping PC off the stack.
    MOV PC, [ST]
```

# Initial startup
After reset signal clears, Pixie 16 takes 1-2 cycles to load the entrypoint address from [0x0002].
At this point, all registers are zero, including PF, which means that Pixie 16 starts by jumping to the entrypoint with interrupts disabled.

Pixie 16 (and any occasional additional hardware) are designed  such that 0 is the "safe state", which means that the hardware or CPU are in a defined state when registers are all zero.
