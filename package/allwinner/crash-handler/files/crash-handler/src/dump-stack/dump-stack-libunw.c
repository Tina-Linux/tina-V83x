#include "dump-stack.h"
#include <libunwind-ptrace.h>

#define MAXPROCNAMELEN 512

extern FILE *outputfile;
void _create_dump_stack(Dwfl *dwfl, pid_t pid, Callstack *callstack)
{
	unw_addr_space_t as = 0;
	void *ui = 0;
	do {
		callstack->elems = 0;

		as = unw_create_addr_space(&_UPT_accessors, 0);
		if (!as) {
			break;
		}

		ui = _UPT_create(pid);
		if (!ui) {
			break;
		}

		unw_cursor_t cursor;
		int ret = unw_init_remote(&cursor, as, ui);
		if (ret < 0) {
			fprintf(outputfile, "unw_init_remote() failed: ret=%d code=", ret);
			if (ret == UNW_EINVAL) {
				fprintf(outputfile, "UNW_EINVAL\n");
			} else if (ret == UNW_EUNSPEC) {
				fprintf(outputfile, "UNW_EUNSPEC\n");
			} else if (ret == UNW_EBADREG) {
				fprintf(outputfile, "UNW_EBADREG\n");
			} else {
				fprintf(outputfile, "UNKNOWN\n");
			}
			return;
		}

		char proc_name[MAXPROCNAMELEN];
		for (; callstack->elems < sizeof(callstack->proc)/sizeof(callstack->proc[0]); ) {

			unw_word_t ip;
			if (unw_get_reg(&cursor, UNW_REG_IP, &ip) < 0) {
				break;
			}
			callstack->proc[callstack->elems].addr = ip;

			proc_name[0] = '\0';
			unw_word_t off;
			if (unw_get_proc_name(&cursor, proc_name, sizeof(proc_name), &off) == 0)
				callstack->proc[callstack->elems].offset = off;

			++callstack->elems;

			if (unw_step(&cursor) <= 0) {
				break;
			}
		}
	} while (0);

	if (ui)
		_UPT_destroy(ui);
	if (as)
		unw_destroy_addr_space(as);
}
