
#ifndef PX16_H
#define PX16_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ======== Definitions ======== */

// Unsigned carry out flag mask.
#define FLAG_UCOUT 0x4000
// Signed carry out flag mask.
#define FLAG_SCOUT 0x8000
// Zero flag mask.
#define FLAG_ZERO  0x2000
// Interrupt in progress flag.
#define FLAG_IPROG 0x0008
// Interrupt in progress flag.
#define FLAG_ISW   0x0004
// IRQ enable flag.
#define FLAG_IRQ   0x0002
// NMI enable flag.
#define FLAG_NMI   0x0001

// Unsigned less than.
#define COND_ULT  000
// Unsigned greater than.
#define COND_UGT  001
// Signed less than.
#define COND_SLT  002
// Signed greater than.
#define COND_SGT  003
// Equal.
#define COND_EQ   004
// Unsigned carry set.
#define COND_CS   005
// Always true.
#define COND_TRUE 006
// Unsigned greater than or equal.
#define COND_UGE  010
// Unsigned less than or equal.
#define COND_ULE  011
// Signed greater than or equal.
#define COND_SGE  012
// Signed less than or equal.
#define COND_SLE  013
// Not equal.
#define COND_NE   014
// Unsigned carry not set.
#define COND_CC   015
// Reserved for JSR instruction.
#define COND_JSR  016
// Reserved for carry extend instruction.
#define COND_CX   017

// ADD instructions.
#define OP_ADD 000
// SUB instruction.
#define OP_SUB 001
// CMP instructions.
#define OP_CMP 002
// AND instructions.
#define OP_AND 003
// OR instructions.
#define OP_OR  004
// XOR instructions.
#define OP_XOR 005

// INC instructions.
#define OP_INC 020
// DEC instruction.
#define OP_DEC 021
// CMP1 instructions.
#define OP_CMP1 022
// SHL instructions.
#define OP_SHL 026
// SHR instructions.
#define OP_SHR 027

// Unconditional MOV instructions.
#define OP_MOV 046
// Unconditional LEA instructions.
#define OP_LEA 066

// Offset for carry continue.
#define OFFS_CC 010
// Offset for MATH1 instructions.
#define OFFS_MATH1 020
// Offset for MOV instructions.
#define OFFS_MOV 040
// Offset for LEA instructions.
#define OFFS_LEA 060

/* ======== Types ======== */

typedef uint32_t lword;
typedef uint16_t word;

typedef enum {
	MEM_TYPE_RAM,
	MEM_TYPE_ROM,
	MEM_TYPE_MMIO,
	MEM_TYPE_VRAM,
} memtype;

typedef enum {
	PX_REG_R0,
	PX_REG_R1,
	PX_REG_R2,
	PX_REG_R3,
	PX_REG_ST,
	PX_REG_PF,
	PX_REG_PC,
	PX_REG_IMM,
} reg;

typedef enum {
	PX_ADDR_R0,
	PX_ADDR_R1,
	PX_ADDR_R2,
	PX_ADDR_R3,
	PX_ADDR_ST,
	PX_ADDR_MEM,
	PX_ADDR_PC,
	PX_ADDR_IMM,
} addrmode;

struct s_memmap;
struct s_core;
union  s_core_cu;
struct s_instr;

typedef struct s_memmap memmap;
typedef struct s_core core;
typedef union  s_core_cu core_cu;
typedef struct s_instr instr;

struct s_memmap {
	// Whether an IRQ is being asserted.
	bool irq;
	// Whether an NMI is being asserted.
	bool nmi;
	
	// Context to memory callbacks.
	void *mem_ctx;
	// Read callback.
	// May not do read cycle triggers if notouchy is true.
	word (*mem_read)(core *cpu, memmap *mem, word address, bool notouchy, void *ctx);
	// Write callback.
	// May always do write cycle triggers.
	void (*mem_write)(core *cpu, memmap *mem, word address, word data, void *ctx);
	// Get type of memory at address.
	memtype (*mem_gettype)(core *cpu, memmap *mem, word address, void *ctx);
	
	// Context to tick callbacks.
	void *tick_ctx;
	// Before core tick (i.e. before rising edge).
	void (*pre_tick)(core *cpu, memmap *mem, void *ctx);
	// After core tick (i.e. after rising edge).
	void (*post_tick)(core *cpu, memmap *mem, void *ctx, lword cycles);
	
	// The reset line activation.
	void (*reset)(core *cpu, memmap *mem, void *ctx);
};

union s_core_cu {
	struct {
		// The first two cycles of startup.
		bool boot_0, boot_1;
		// Loading the instruction word.
		bool load_0;
		// Loading into register IMM0.
		bool load_1;
		// Loading into register IMM1.
		bool load_2;
		// Pre-decrementing ST for push operations.
		bool push;
		// Storing the PC to the stack for JSR type instructions.
		bool jsr;
		// Address calculations and LEA type instructions.
		bool addr;
		// Math type instructions.
		bool act;
		// MOV type instructions.
		bool mov;
		// Post-incrementing ST for pop operations.
		bool pop;
		// Interrupt handling cycles.
		bool intr_0, intr_1, intr_2, intr_3, intr_4;
	};
	// Array representation of state.
	bool array[16];
};

struct s_instr {
	bool     y;
	uint8_t  x;
	uint8_t  b;
	uint8_t  a;
	uint8_t  o;
};

struct s_core {
	/* ==== Busses ==== */
	// The A parameter of a computation.
	word par_bus_a;
	// The B parameter of a computation.
	// Also used to pass through a register.
	word par_bus_b;
	// The result of the ALU calculation.
	word res_bus;
	// The bus used to write to registers to interface to memory.
	word data_bus;
	// The address to use for memory access.
	word addr_bus;
	/* ==== Registers ==== */
	union {
		struct {
			// General registers.
			word R0, R1, R2, R3;
			// Special registers.
			word ST, PF, PC;
		};
		// Registers clumped into an array.
		word regfile[7];
	};
	// The registers that store constants.
	word    imm0, imm1;
	// The register that stores the memory access address.
	word    AR;
	/* ==== Control unit state ==== */
	// Helper register that makes return from interrupt possible.
	bool    intr_helper;
	// Contains the stage of execution the CPU is busy with.
	core_cu state;
	// The unpacked instruction.
	instr   current;
	/* ==== Statistics ==== */
	// The amount of instructions ran, counted by stage load0.
	uint64_t insn_count;
	// The amount of subroutines entered, counted by pattern MOV.JSR and LEA.JSR.
	uint64_t jsr_count;
	// The amount of IRQs handled, counted by IRQ handler calls.
	uint64_t irq_count;
	// The amount of NMIs handled, counted by NMI handler calls.
	uint64_t nmi_count;
};

/* ==== Them descriptions ==== */

// The number of control unit states that exist.
#define n_cu_states 16

// Describes CU stages.
extern const char *cu_state_names[n_cu_states];

/* ======== Functions ======== */

// Resets the core.
void core_reset(core *cpu);

// Simulates one full clock cycle of the core.
void core_tick(core *cpu, memmap *mem) __attribute__((hot));

// Simulates exactly cycles full clock cycles of the core.
void core_ticks(core *cpu, memmap *mem, lword cycles);

// Simulates a full instruction instead of a number of cycles.
// Returns the number of cycles the instruction takes in reality.
lword fast_tick(core *cpu, memmap *mem) __attribute__((hot));

// Simulates at least cycles full clock cycles worth of instructions.
// Uses fast_tick internally, which makes this usually take a cycle or two more.
// Returns the real number of simulated cycles.
lword fast_ticks(core *cpu, memmap *mem, lword cycles);

// Simulates as many cycles as possible until timeout is reached.
// Returns the real number of simulated cycles.
lword warp_ticks(core *cpu, memmap *mem, uint64_t timeout);

#endif //PX16_H
