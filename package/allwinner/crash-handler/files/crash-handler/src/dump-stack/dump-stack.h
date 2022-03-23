#ifndef CRASH_STACK_H
#define CRASH_STACK_H

#include <stdint.h>
#include <elfutils/libdwfl.h>
#include <tina_log.h>

#ifdef TAG
#undef TAG
#define TAG "Tina-Crash-Handler"
#endif

struct ProcInfo {
	uintptr_t addr;
	int offset;
	char *name;
	char *module_name;
	int module_offset;
};
typedef struct ProcInfo ProcInfo;

#define MAX_CALLSTACK_LEN 1000

struct Callstack {
	ProcInfo proc[MAX_CALLSTACK_LEN];
	size_t elems;
};
typedef struct Callstack Callstack;

void *_get_place_for_register_value(const char *regname, int regnum);
void _create_dump_stack(Dwfl *dwfl, pid_t pid, Callstack *callstack);
void *_dump_stack_get_memory_for_ptrace_registers(size_t *size);
void _dump_stack_set_ptrace_registers(void *regbuf);
void _dump_stack_print_regs(FILE* outputfile);

#endif
