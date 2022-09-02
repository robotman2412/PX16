![Logo image](logo_small.png)

# Pixie 16
A 16-bit simple-as-possible\* RISC CPU.
Has conditional `MOV` instructions (which is also how branching happens),
And a few math instructions.

Has 4 general registers: `R0`, `R1`, `R2`, `R3`
And 3 special registers: `ST` (stack register), `PF` (flags register), `PC` (program counter)

64K x 16bit memory.


### Note:
RISC is not equal to RISC-V.

PX16 cannot address bytes.

# TODO
Stuf i do be workin on:
- Emulator
- Finalise MMIO Hardware
- C Compiler
