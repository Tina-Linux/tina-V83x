#include "dump-stack.h"
#include <sys/user.h>
#include <string.h>

static struct user_regs_struct g_registers;

struct Regs {
	Dwarf_Addr x29;
	Dwarf_Addr x30;
	Dwarf_Addr pc;
	Dwarf_Addr sp;
};

#define REG_SP  32
#define REG_PC  33
#define REG_X29 29
#define REG_X30 30

typedef struct Regs Regs;
static Regs g_regs;

void *_dump_stack_get_memory_for_ptrace_registers(size_t *size)
{
	if (NULL != size)
		*size = sizeof(g_registers);
	return &g_registers;
}

void _dump_stack_set_ptrace_registers(void *regbuf)
{
	struct user_regs_struct *regs = regbuf;
	memcpy(_get_place_for_register_value("sp", 0), &regs->sp, sizeof(regs->sp));
	memcpy(_get_place_for_register_value("pc", 0), &regs->pc, sizeof(regs->pc));
	memcpy(_get_place_for_register_value("x29", 0), &regs->regs[29], sizeof(regs->regs[29]));
	memcpy(_get_place_for_register_value("x30", 0), &regs->regs[30], sizeof(regs->regs[30]));
}

void *_get_place_for_register_value(const char *regname, int regnum)
{
	if (strcmp(regname, "pc") == 0 || REG_PC == regnum)
		return &g_regs.pc;
	else if (strcmp(regname, "sp") == 0 || REG_SP == regnum)
		return &g_regs.sp;
	else if (strcmp(regname, "x29") == 0 || REG_X29 == regnum)
		return &g_regs.x29;
	else if (strcmp(regname, "x30") == 0 || REG_X30 == regnum)
		return &g_regs.x30;
	return NULL;
}

#define PSR_MODE32_BIT  0x00000010
#define PSR_MODE_MASK   0x0000000f

void _dump_stack_print_regs(FILE* outputfile)
{
	int i;

	TLOGE("\nRegister Information.\n");
	for (i = 0; i <= 30; i=i+2) {
		TLOGE("x%-2d  = 0x%016llx, x%-2d  = 0x%016llx, x%-2d  = 0x%016llx", i, g_registers.regs[i], i+1, g_registers.regs[i+1], i+2, g_registers.regs[i+2]);
		if (i != 30 && 0 == i % 3)
			fprintf(outputfile, ", ");
		else
			fprintf(outputfile, "\n");
	}
	TLOGE("xr   = 0x%016llx, ip0  = 0x%016llx\nip1  = 0x%016llx, pr   = 0x%016llx\n", g_registers.regs[8], g_registers.regs[16], g_registers.regs[17], g_registers.regs[18]);
	TLOGE("fp   = 0x%016llx, lr   = 0x%016llx\npc   = 0x%016llx, sp   = 0x%016llx\n", g_registers.regs[29], g_registers.regs[30], g_registers.pc, g_registers.sp);
}
