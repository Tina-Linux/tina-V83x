#include "dump-stack.h"
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#define REGS_REGULAR_NUM 13
#define REG_FP 11
#define REG_IP 12
#define REG_SP 13
#define REG_LR 14
#define REG_PC 15
#define REG_SPSR 16

struct Regs {
	Dwarf_Addr regs[REGS_REGULAR_NUM];
	Dwarf_Addr sp;
	Dwarf_Addr lr;
	Dwarf_Addr pc;
	Dwarf_Addr spsr;
};

typedef struct Regs Regs;
static Regs g_regs;

static struct user_regs g_ptrace_registers;

void *_dump_stack_get_memory_for_ptrace_registers(size_t *size)
{
	if (NULL != size)
		*size = sizeof(g_ptrace_registers);
	return &g_ptrace_registers;
}

void *_get_place_for_register_value(const char *regname, int regnum)
{
	if (strcmp(regname, "pc") == 0 || REG_PC == regnum)
		return &g_regs.pc;
	else if (strcmp(regname, "sp") == 0 || REG_SP == regnum)
		return &g_regs.sp;
	else if (strcmp(regname, "lr") == 0 || REG_LR == regnum)
		return &g_regs.lr;
	else if (strcmp(regname, "spsr") == 0 || REG_SPSR == regnum)
		return &g_regs.spsr;
	else if (regnum < REGS_REGULAR_NUM)
		return &g_regs.regs[regnum];
	return NULL;
}

void _dump_stack_set_ptrace_registers(void *regbuf)
{
	struct user_regs *registers = regbuf;
	int i;
	for (i = 0; i < sizeof(registers->uregs)/sizeof(registers->uregs[0]); i++) {
		void *regmem = _get_place_for_register_value("", i);
		if (NULL != regmem)
			memcpy(regmem, &registers->uregs[i], sizeof(registers->uregs[i]));
	}
}

void _dump_stack_print_regs(FILE* outputfile)
{
	TLOGE("\nRegister Information\n");
	TLOGE("r0   = 0x%08lx, r1   = 0x%08lx r2   = 0x%08lx, r3   = 0x%08lx",
	       g_ptrace_registers.uregs[0], g_ptrace_registers.uregs[1],
	       g_ptrace_registers.uregs[2], g_ptrace_registers.uregs[3]);
	TLOGE("r4   = 0x%08lx, r5   = 0x%08lx r6   = 0x%08lx, r7   = 0x%08lx",
	       g_ptrace_registers.uregs[4], g_ptrace_registers.uregs[5],
	       g_ptrace_registers.uregs[6], g_ptrace_registers.uregs[7]);
	TLOGE("r8   = 0x%08lx, r9   = 0x%08lx r10  = 0x%08lx, fp   = 0x%08lx",
	       g_ptrace_registers.uregs[8], g_ptrace_registers.uregs[9],
	       g_ptrace_registers.uregs[10], g_ptrace_registers.uregs[REG_FP]);
	TLOGE("ip   = 0x%08lx, sp   = 0x%08lx lr   = 0x%08lx, pc   = 0x%08lx",
	       g_ptrace_registers.uregs[REG_IP], g_ptrace_registers.uregs[REG_SP],
	       g_ptrace_registers.uregs[REG_LR], g_ptrace_registers.uregs[REG_PC]);
	TLOGE("cpsr = 0x%08lx", g_ptrace_registers.uregs[REG_SPSR]);
}
