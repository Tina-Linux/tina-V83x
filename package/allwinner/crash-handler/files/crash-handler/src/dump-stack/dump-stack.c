/*
 * Copyright (c) 2018 Allwinnertech Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE 1

#include "dump-stack.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libelf.h>
#include <linux/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>

#include <elfutils/version.h>
#include <elfutils/libdwfl.h>

#define BUF_SIZE (BUFSIZ)
#define HEXA 16
#define PERM_LEN 5
#define ADDR_LEN 16
#define STR_ANONY "[anony]"
#define STR_ANONY_LEN 8

FILE *outputfile = NULL;
static FILE *errfile = NULL;
static FILE *bufferfile = NULL;

enum {
	OPT_PID,
	OPT_TID,
	OPT_SIGNUM,
	OPT_OUTPUTFILE,
	OPT_ERRFILE
};

const struct option opts[] = {
	{ "pid", required_argument, 0, OPT_PID },
	{ "tid", required_argument, 0, OPT_TID },
	{ "sig", required_argument, 0, OPT_SIGNUM },
	{ "output", required_argument, 0, OPT_OUTPUTFILE },
	{ "erroutput", required_argument, 0, OPT_ERRFILE },
	{ 0, 0, 0, 0 }
};

struct addr_node {
	uintptr_t startaddr;
	uintptr_t endaddr;
	char perm[PERM_LEN];
	char *fpath;
	struct addr_node *next;
};

static struct addr_node *get_addr_list_from_maps(int fd);
static void free_all_nodes(struct addr_node *start);
static char *fgets_fd(char *str, int len, int fd);

extern char *__cxa_demangle(const char *mangled_name, char *output_buffer,
		size_t *length, int *status);

static int __module_callback(Dwfl_Module *module, void **userdata,
		const char *name, Dwarf_Addr address, void *arg)
{
	return DWARF_CB_OK;
}

static void __find_symbol_in_elf(ProcInfo *proc_info, Dwarf_Addr mapping_start)
{
	Elf *elf;
	int fd;
	const char *elf_name = proc_info->module_name;
	Dwarf_Addr address = proc_info->addr;

	fd = open(elf_name, O_RDONLY);
	if (-1 == fd)
		return;

	elf = elf_begin(fd, ELF_C_READ_MMAP, NULL);

	if (NULL == elf) {
		close(fd);
		return;
	}

	Elf_Scn *scn = NULL;
	int found = 0;

	while ((scn = elf_nextscn(elf, scn)) != NULL && !found) {
		GElf_Shdr shdr_mem;
		GElf_Shdr *shdr = gelf_getshdr(scn, &shdr_mem);
		if (shdr != NULL && (shdr->sh_type == SHT_SYMTAB || shdr->sh_type == SHT_DYNSYM)) {
			Elf_Data *sdata = elf_getdata(scn, NULL);
			unsigned int nsyms = sdata->d_size / (gelf_getclass(elf) == ELFCLASS32 ?
					sizeof(Elf32_Sym) :
					sizeof(Elf64_Sym));
			unsigned int cnt;
			uintptr_t address_offset = address;
			if (shdr->sh_type == SHT_DYNSYM)
				address_offset -= mapping_start;
			for (cnt = 0; cnt < nsyms; ++cnt) {
				GElf_Sym sym_mem;
				Elf32_Word xndx;
				GElf_Sym *sym = gelf_getsymshndx(sdata, NULL, cnt, &sym_mem, &xndx);
				if (sym != NULL && sym->st_shndx != SHN_UNDEF) {
					if (sym->st_value <= address_offset && address_offset < sym->st_value + sym->st_size) {
						free(proc_info->name);
						proc_info->name = strdup(elf_strptr(elf, shdr->sh_link, sym->st_name));
						proc_info->offset = address_offset - sym->st_value;
						found = 1;
						break;
					}
				}
			}
		}
	}

	elf_end(elf);
	close(fd);
}

static int __attachable(pid_t pid, pid_t tid)
{
	char buf[40];
	FILE *f;
	char status;

	snprintf(buf, sizeof(buf), "/proc/%d/task/%d/stat", pid, tid);

	f = fopen(buf, "r");
	if (NULL == f)
		return -1;

	if (fscanf(f, "%*d %*s %c", &status) != 1) {
		fclose(f);
		return -1;
	}

	fclose(f);

	return status != 'D';
}

static void __print_proc_file(pid_t pid, pid_t tid, const char *name)
{
	char buf[1024];
	FILE *f;
	int r;

	snprintf(buf, sizeof(buf), "/proc/%d/task/%d/%s", pid, tid, name);

	fprintf(outputfile, "%s:\n", buf);

	f = fopen(buf, "r");
	if (NULL == f)
	{
		fprintf(errfile, "Failed to open %s: %m\n", buf);
		return;
	}

	while ((r = fread(buf, 1, sizeof(buf), f)) > 0)
	{
		fwrite(buf, r, 1, outputfile);
	}

	fclose(f);

	fprintf(outputfile, "\n");
}

static void __print_not_attachable_process_info(pid_t pid, pid_t tid)
{
	fprintf(outputfile, "ERROR: can't attach to process %d, thread %d - thread is in uninterruptible sleep state\n", pid, tid);
	fprintf(outputfile, "Giving some /proc info instead:\n\n");
	__print_proc_file(pid, tid, "wchan");
	fprintf(outputfile, "\n");
	__print_proc_file(pid, tid, "syscall");
	__print_proc_file(pid, tid, "stack");
}

static Dwfl *__open_dwfl_with_pid(pid_t pid, pid_t tid)
{
	int status;
	pid_t stopped_pid;

	status = __attachable(pid, tid);
	if (-1 == status)
	{
		fprintf(errfile, "failed to read /proc/%d/task/%d/stat: %m\n", pid, tid);
		return NULL;
	}

	if (!status)
	{
		__print_not_attachable_process_info(pid, tid);
		return NULL;
	}

	if (ptrace(PTRACE_SEIZE, tid, NULL, PTRACE_O_TRACEEXIT) != 0) {
		fprintf(errfile, "PTRACE_SEIZE failed on TID %d: %m\n", tid);
		return NULL;
	}

	ptrace(PTRACE_INTERRUPT, tid, 0, 0);

	stopped_pid = waitpid(tid, &status, __WALL);
	if (stopped_pid == -1 || stopped_pid != tid || !WIFSTOPPED(status)) {
		fprintf(errfile, "waitpid failed: %m, stopped_pid=%d, status=%d\n", stopped_pid, status);
		return NULL;
	}

	static const Dwfl_Callbacks proc_callbacks = {
		.find_elf = dwfl_linux_proc_find_elf,
		.find_debuginfo = dwfl_standard_find_debuginfo,
		.section_address = NULL,
		.debuginfo_path = NULL
	};

	Dwfl *dwfl = dwfl_begin(&proc_callbacks);
	if (dwfl == NULL) {
		fprintf(errfile, "process %d : Can't start dwfl (%s)\n", tid, dwfl_errmsg(-1));
		return NULL;
	}

	if (dwfl_linux_proc_report(dwfl, tid) < 0) {
		fprintf(errfile, "process %d : dwfl report failed (%s)\n", tid, dwfl_errmsg(-1));
		dwfl_end(dwfl);
		return NULL;
	}

#if _ELFUTILS_PREREQ(0,158)
	if (dwfl_linux_proc_attach(dwfl, tid, true) < 0) {
		fprintf(errfile, "process %d : dwfl attach failed (%s)\n", tid, dwfl_errmsg(-1));
		dwfl_end(dwfl);
		return NULL;
	}
#endif
	return dwfl;
}

static int __get_registers_ptrace(pid_t pid)
{
	struct iovec data;
	data.iov_base = _dump_stack_get_memory_for_ptrace_registers(&data.iov_len);

	if (NULL == data.iov_base) {
		fprintf(errfile, "Cannot get memory for registers for ptrace (not implemented for this architecture\n");
		return -1;
	}

	if (ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &data) != 0) {
		fprintf(errfile, "PTRACE_GETREGSET failed on PID %d: %m\n", pid);
		return -1;
	}

	_dump_stack_set_ptrace_registers(data.iov_base);

	return 0;
}

static void __dump_stack_print_signal(int signo)
{
	const char* const signal_table[] = {
		[SIGHUP]="SIGHUP", [SIGINT]="SIGINT", [SIGQUIT]="SIGQUIT",
		[SIGILL]="SIGILL", [SIGTRAP]="SIGTRAP", [SIGABRT]="SIGABRT",
		/* [SIGIOT]="SIGIOT", */ [SIGBUS]="SIGBUS", [SIGFPE]="SIGFPE",
		[SIGKILL]="SIGKILL", [SIGUSR1]="SIGUSR1", [SIGSEGV]="SIGSEGV",
		[SIGUSR2]="SIGUSR2", [SIGPIPE]="SIGPIPE", [SIGALRM]="SIGALRM",
		[SIGTERM]="SIGTERM", [SIGSTKFLT]="SIGSTKFLT", [SIGCHLD]="SIGCHLD",
		[SIGCONT]="SIGCONT", [SIGSTOP]="SIGSTOP", [SIGTSTP]="SIGTSTP",
		[SIGTTIN]="SIGTTIN", [SIGTTOU]="SIGTTOU", [SIGURG]="SIGURG",
		[SIGXCPU]="SIGXCPU", [SIGXFSZ]="SIGXFSZ", [SIGVTALRM]="SIGVTALRM",
		[SIGPROF]="SIGPROF", [SIGWINCH]="SIGWINCH", [SIGIO]="SIGIO",
		[SIGPWR]="SIGPWR", [SIGSYS]="SIGSYS", /* [SIGUNUSED]="SIGUNUSED", */
	};

	if (SIGHUP > signo || signo > SIGSYS) {
		fprintf(errfile, "Invalid signal number: %d\n", signo);
		return;
	}

	printf("Signal: %d\n"
	       "      (%s)\n",
	       signo,
	       signal_table[signo]);
}

static void __resolve_symbols_from_dwfl(ProcInfo *proc_info, Dwfl *dwfl)
{
	uintptr_t address = proc_info->addr;
	Dwfl_Module *module = dwfl_addrmodule(dwfl, address);
	if (module) {

		Dwarf_Addr mapping_start = 0;
		const char *fname = 0;
		const char *module_name = dwfl_module_info(module, NULL, &mapping_start, NULL, NULL, NULL, &fname, NULL);

		proc_info->module_offset = address - mapping_start;

		if (!proc_info->module_name) {
			if (fname)
				proc_info->module_name = strdup(fname);
			else if (module_name)
				proc_info->module_name = strdup(module_name);
		}

		const char *symbol = dwfl_module_addrname(module, address);
		if (symbol) {
			free(proc_info->name);
			proc_info->name = strdup(symbol);
		}
		else if (proc_info->module_name != NULL) {
			__find_symbol_in_elf(proc_info, mapping_start);
		}
	}
}

static int is_symbol_demanglable(const char *symbol)
{
	return symbol != 0 && (strlen(symbol) >= 2) &&
		symbol[0] == '_' && symbol[1] == 'Z';
}

static void __demangle_symbols(ProcInfo *proc_info)
{
	int status = -1;
	char *dem_buffer = NULL;
	char *demangled_symbol = __cxa_demangle(proc_info->name, dem_buffer, NULL, &status);
	if (status == 0) {
		free(proc_info->name);
		proc_info->name = demangled_symbol;
	}
}

static void __resolve_symbols(ProcInfo *proc_info, Dwfl *dwfl)
{
	__resolve_symbols_from_dwfl(proc_info, dwfl);

	if (is_symbol_demanglable(proc_info->name))
		__demangle_symbols(proc_info);
}

static void __print_proc_info(int iter, ProcInfo *proc_info)
{
	if (sizeof(proc_info->addr) > 4) {
		TLOGE("  %2zu: %s + 0x%x (0x%016llx) [%s] + 0x%x", iter, proc_info->name, proc_info->offset, (long long)proc_info->addr, proc_info->module_name, proc_info->module_offset);
	} else {
		TLOGE("  %2zu: %s + 0x%x (0x%08x) [%s] + 0x%x", iter, proc_info->name, proc_info->offset, (int32_t)proc_info->addr, proc_info->module_name, proc_info->module_offset);
	}
}

static void __print_callstack(Callstack *callstack, pid_t pid)
{
	TLOGE("  Callstack Information");
	TLOGE("  PID: %d", pid);
	TLOGE("  Call Stack Count: %zu", callstack->elems);

	size_t it;
	for (it = 0; it != callstack->elems; ++it) {
		__print_proc_info(it, &callstack->proc[it]);
	}
	TLOGE("  End of Callstack Information.\n");
}

void callstack_constructor(Callstack *callstack)
{
	size_t it;
	callstack->elems = 0;
	for (it = 0; it < (int)sizeof(callstack->proc)/sizeof(callstack->proc[0]); ++it) {
		callstack->proc[it].offset = -1;
		callstack->proc[it].name = 0;
		callstack->proc[it].module_name = 0;
	}
}

void callstack_destructor(Callstack *callstack)
{
	size_t it;
	for (it = 0; it < callstack->elems; ++it) {
		free(callstack->proc[it].name);
		free(callstack->proc[it].module_name);
	}
}

static void __dump_stack_print_exe(FILE* outputfile, pid_t pid)
{
	int fd, ret;
	char file_path[PATH_MAX];
	char cmd_path[PATH_MAX];

	snprintf(cmd_path, PATH_MAX, "/proc/%d/cmdline", pid);
	if ((fd = open(cmd_path, O_RDONLY)) < 0)
		return;

	if ((ret = read(fd, file_path, sizeof(file_path) - 1)) <= 0) {
		close(fd);
		return;
	}
	file_path[ret] = '\0';

	fprintf(outputfile, "Executable File Path: %s\n", file_path);
	close(fd);
}

static void __dump_stack_print_threads(FILE* outputfile, pid_t pid, pid_t tid)
{
	int threadnum=1;
	DIR *dir;
	struct dirent entry;
	struct dirent *dentry=NULL;
	char task_path[PATH_MAX];
	struct stat sb;


	snprintf(task_path, PATH_MAX, "/proc/%d/task", pid);
	if (stat(task_path, &sb) == -1) {
		return;
	}

	threadnum = sb.st_nlink - 2;

	if (threadnum > 1) {
		fprintf(outputfile, "\nThreads Information\n");
		fprintf(outputfile,
			"Threads: %d\nPID = %d TID = %d\n",
			threadnum, pid, tid);
		/* print thread */
		dir = opendir(task_path);
		if (!dir) {
			fprintf(errfile, "[dump-stack] cannot open %s\n", task_path);
		} else {
			while (readdir_r(dir, &entry, &dentry) == 0 && dentry) {
				if (strcmp(dentry->d_name, ".") == 0 ||
				    strcmp(dentry->d_name, "..") == 0)
					continue;
				fprintf(outputfile, "%s ", dentry->d_name);
			}
			closedir(dir);
			fprintf(outputfile, "\n");
		}
	}

}

static void __dump_stack_print_maps(FILE* outputfile, pid_t pid)
{
	char file_path[PATH_MAX];
	struct addr_node *head = NULL;
	struct addr_node *t_node;
	int fd;

	snprintf(file_path, PATH_MAX, "/proc/%d/maps", pid);

	if ((fd = open(file_path, O_RDONLY)) < 0) {
		fprintf(errfile, "[dump-stack] cannot open %s\n", file_path);
        } else {
                head = get_addr_list_from_maps(fd);
                close(fd);
        }
	if (head == NULL) {
		return;
	}

	t_node = head;
	fprintf(outputfile, "\nMaps Information\n");
	while (t_node) {
		if (!strncmp(STR_ANONY, t_node->fpath, STR_ANONY_LEN)) {
			t_node = t_node->next;
		} else {
			fprintf(outputfile, "%16lx %16lx %s %s\n",
				(unsigned long)t_node->startaddr,
				(unsigned long)t_node->endaddr,
				t_node->perm, t_node->fpath);
			t_node = t_node->next;
		}
	}
	fprintf(outputfile, "End of Maps Information\n");
	free_all_nodes(head);
}
#if 0
static struct addr_node *get_addr_list_from_maps(int fd)
{
        int fpath_len, result;
        uintptr_t saddr;
        uintptr_t eaddr;
        char perm[PERM_LEN];
        char path[PATH_MAX];
        char addr[ADDR_LEN * 2 + 2];
        char linebuf[BUF_SIZE];
        struct addr_node *head = NULL;
        struct addr_node *tail = NULL;
        struct addr_node *t_node = NULL;

        while (fgets_fd(linebuf, BUF_SIZE, fd) != NULL) {
                memset(path, 0, PATH_MAX);
                result = sscanf(linebuf, "%s %s %*s %*s %*s %s ", addr, perm, path);
                if (result < 0)
                        continue;
                perm[PERM_LEN - 1] = 0;
                /* rwxp */
                if ((perm[2] == 'x' && path[0] == '/') ||
		    (perm[1] == 'w' && path[0] != '/'))
		{
			char* addr2 = strchr(addr, '-');
			*(addr2++) = '\0';
			/* add addr node to list */
			saddr = strtoul(addr, NULL, HEXA);
			/* ffff0000-ffff1000 */
			eaddr = strtoul(addr2, NULL, HEXA);
			/* make node and attach to the list */
			t_node = (struct addr_node *)mmap(0, sizeof(struct addr_node),
							  PROT_READ | PROT_WRITE,
							  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			if (t_node == NULL) {
				fprintf(errfile, "error : mmap\n");
				return NULL;
			}
			memcpy(t_node->perm, perm, PERM_LEN);
			t_node->startaddr = saddr;
			t_node->endaddr = eaddr;
			t_node->fpath = NULL;
			fpath_len = strlen(path);
			if (fpath_len > 0) {
				t_node->fpath = (char *)mmap(0, fpath_len + 1,
							     PROT_READ | PROT_WRITE,
							     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
				memset(t_node->fpath, 0, fpath_len + 1);
				memcpy(t_node->fpath, path, fpath_len);
			} else {
				t_node->fpath = (char *)mmap(0, STR_ANONY_LEN,
							     PROT_READ | PROT_WRITE,
							     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
				memset(t_node->fpath, 0, STR_ANONY_LEN);
				memcpy(t_node->fpath, STR_ANONY, STR_ANONY_LEN);
			}
			t_node->next = NULL;
			if (head == NULL) {
				head = t_node;
				tail = t_node;
			} else {
				tail->next = t_node;
				tail = t_node;
			}
		}
	}
	return head;
}
#endif

static void free_all_nodes(struct addr_node *start)
{
	struct addr_node *t_node, *n_node;
	int fpath_len;

	if (start == NULL)
		return;
	t_node = start;
	n_node = t_node->next;
	while (t_node) {
		if (t_node->fpath != NULL) {
			fpath_len = strlen(t_node->fpath);
			munmap(t_node->fpath, fpath_len + 1);
		}
		munmap(t_node, sizeof(struct addr_node));
		if (n_node == NULL)
			break;
		t_node = n_node;
		n_node = n_node->next;
	}
}

static char *fgets_fd(char *str, int len, int fd)
{
        char ch;
        register char *cs;
        int num = 0;

        cs = str;
        while (--len > 0 && (num = read(fd, &ch, 1) > 0)) {
                if ((*cs++ = ch) == '\n')
                        break;
        }
        *cs = '\0';
        return (num == 0 && cs == str) ? NULL : str;
}
#if 0
static void __dump_stack_print_meminfo(FILE* outputfile, pid_t pid)
{
	char infoname[BUF_SIZE];
	char memsize[BUF_SIZE];
	char linebuf[BUF_SIZE];
	char file_path[PATH_MAX];
	int fd;

	fprintf(outputfile, "\nMemory information\n");

        if ((fd = open("/proc/meminfo", O_RDONLY)) < 0) {
                fprintf(errfile, "[dump-stack] cannot open /proc/meminfo\n");
        } else {
                while (fgets_fd(linebuf, BUF_SIZE, fd) != NULL) {
                        sscanf(linebuf, "%s %s %*s", infoname, memsize);
                        if (strcmp("MemTotal:", infoname) == 0) {
                                fprintf(outputfile, "%s %8s KB\n", infoname, memsize);
                        } else if (strcmp("MemFree:", infoname) == 0) {
                                fprintf(outputfile, "%s  %8s KB\n", infoname, memsize);
                        } else if (strcmp("Buffers:", infoname) == 0) {
                                fprintf(outputfile, "%s  %8s KB\n", infoname, memsize);
                        } else if (strcmp("Cached:", infoname) == 0) {
                                fprintf(outputfile, "%s   %8s KB\n", infoname, memsize);
                                break;
                        }
                }
                close(fd);
        }

	snprintf(file_path, PATH_MAX, "/proc/%d/status", pid);
        if ((fd = open(file_path, O_RDONLY)) < 0) {
                fprintf(errfile, "[dump-stack] cannot open %s\n", file_path);
        } else {
                while (fgets_fd(linebuf, BUF_SIZE, fd) != NULL) {
                        sscanf(linebuf, "%s %s %*s", infoname, memsize);
                        if (strcmp("VmPeak:", infoname) == 0) {
                                fprintf(outputfile, "%s   %8s KB\n", infoname,
                                                memsize);
                        } else if (strcmp("VmSize:", infoname) == 0) {
                                fprintf(outputfile, "%s   %8s KB\n", infoname,
                                                memsize);
                        } else if (strcmp("VmLck:", infoname) == 0) {
                                fprintf(outputfile, "%s    %8s KB\n", infoname,
                                                memsize);
                        } else if (strcmp("VmPin:", infoname) == 0) {
                                fprintf(outputfile, "%s    %8s KB\n", infoname,
                                                memsize);
                        } else if (strcmp("VmHWM:", infoname) == 0) {
                                fprintf(outputfile, "%s    %8s KB\n",
                                                infoname, memsize);
                        } else if (strcmp("VmRSS:", infoname) == 0) {
                                fprintf(outputfile, "%s    %8s KB\n",
                                                infoname, memsize);
                        } else if (strcmp("VmData:", infoname) == 0) {
                                fprintf(outputfile, "%s   %8s KB\n",
                                                infoname, memsize);
                        } else if (strcmp("VmStk:", infoname) == 0) {
                                fprintf(outputfile, "%s    %8s KB\n",
                                                infoname, memsize);
                        } else if (strcmp("VmExe:", infoname) == 0) {
                                fprintf(outputfile, "%s    %8s KB\n",
                                                infoname, memsize);
                        } else if (strcmp("VmLib:", infoname) == 0) {
                                fprintf(outputfile, "%s    %8s KB\n",
                                                infoname, memsize);
                        } else if (strcmp("VmPTE:", infoname) == 0) {
                                fprintf(outputfile, "%s    %8s KB\n",
                                                infoname, memsize);
                        } else if (strcmp("VmSwap:", infoname) == 0) {
                                fprintf(outputfile, "%s   %8s KB\n",
                                                infoname, memsize);
                                break;
                        }
                }
                close(fd);
        }
}
#endif

static void __print_buffer_info(FILE* bufferfile, FILE *outputfile)
{
	int cnt;
	char buf[1024];

	if (fseek(bufferfile, 0, SEEK_SET) < 0) {
		fprintf(errfile, "Failed to fseek\n");
		return;
	}
	while ((cnt = fread(buf, sizeof(char), sizeof(buf), bufferfile)) != 0) {
		if (cnt != fwrite(buf, sizeof(char), cnt, outputfile))
			break;
	}
}

static int check_thread_wchan(int pid, int tid)
{
	int fd, cnt;
	char path[PATH_MAX], buf[100];

	snprintf(path, PATH_MAX, "/proc/%d/task/%d/wchan", pid, tid);
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		fprintf(errfile, "[dump-stack] cannot open %s\n", path);
		return -errno;
	}
	cnt = read(fd, buf, sizeof(buf));
	if (cnt == -1 || cnt == sizeof(buf)) {
		fprintf(errfile, "[dump-stack] read %s error\n", path);
		close(fd);
		return -errno;
	}
	buf[cnt] = 0;
	close(fd);

	if (strncmp("do_coredump", buf, sizeof(buf)) == 0)
		return tid;
	else
		return 0;
}

static int find_crash_tid(int pid)
{
	int threadnum = 1;
	int crash_tid = -1;
	DIR *dir;
	struct dirent entry;
	struct dirent *dentry = NULL;
	char task_path[PATH_MAX];
	struct stat sb;

	snprintf(task_path, PATH_MAX, "/proc/%d/task", pid);
	if (stat(task_path, &sb) == -1) {
		return -1;
	}

	threadnum = sb.st_nlink - 2;

	if (threadnum > 1) {
		dir = opendir(task_path);
		if (!dir) {
			fprintf(errfile, "[dump-stack] cannot open %s\n", task_path);
			return -1;
		} else {
			while (readdir_r(dir, &entry, &dentry) == 0 && dentry) {
				if (strcmp(dentry->d_name, ".") == 0 ||
				    strcmp(dentry->d_name, "..") == 0)
					continue;
				crash_tid = check_thread_wchan(pid,
						atoi(dentry->d_name));
				if (crash_tid > 0)
					break;
			}
			closedir(dir);
			return crash_tid;
		}
	}
	return -1;
}

int main(int argc, char **argv)
{
	int c;
	int signo = 0;
	pid_t pid = 0;
	pid_t tid = 0;
	FILE *fp;

	char bufferfile_path[20] = "/tmp/crash.XXXXXX";

       if (argc == 2 && !strcmp(argv[1], "--install")) {
                char procfile[15] = "/proc/self/exe";
                char actualpath[PATH_MAX];
                char *ptr;

                fp = fopen("/proc/sys/kernel/core_pattern", "w");
                if (!fp) {
                        perror("Cannot open core_pattern for install.\n");
                        exit(1);
                }

                memset(actualpath, 0, PATH_MAX);

                if (readlink(procfile, actualpath, PATH_MAX) != -1) {
                        fprintf(fp, "|%s %%p %%s %%i %%g\n", actualpath);
                } else {
                        fprintf(stderr, "Couldn't find real path for %s\n",
                                argv[0]);
                }
                fclose(fp);

                fp = fopen("/proc/sys/kernel/core_pipe_limit", "w");
                if (!fp) {
                        perror("Cannot open core_pipe_limit for install.\n");
                        exit(1);
                }
                fprintf(fp, "10\n");
                fclose(fp);

                printf("Installation done\n");
                return 0;
        }

        pid = atoi(argv[1]);
        signo = atoi(argv[2]);
	tid = atoi(argv[3]);

	//outputfile = outputfile = fopen("/tmp/aaaaa", "w");

	if (NULL == errfile) errfile = stderr;
	if (NULL == outputfile) outputfile = stdout;

	if (tid == 0) {
		if ((tid = find_crash_tid(pid)) < 0)
			tid = pid;
	}

	if (mkstemp(bufferfile_path) < 0) {
		fprintf(errfile, "Failed to create buffer file.\n");
		return errno;
	}
	if ((bufferfile = fopen(bufferfile_path, "w+")) == NULL) {
		fprintf(errfile, "Failed to open buffer file.\n");
		return errno;
	}
	unlink(bufferfile_path);

	//argc -= optind;

	elf_version(EV_CURRENT);

	/* First, prepare dwfl and modules */
	Dwfl *dwfl = NULL;

	/* Executable File Path */
	__dump_stack_print_exe(outputfile, pid);

	/* Signal information */
	__dump_stack_print_signal(signo);

	/* Memory information */
	//__dump_stack_print_meminfo(bufferfile, pid);

	/* Threads */
	__dump_stack_print_threads(bufferfile, pid, tid);

	/* Maps information */
	__dump_stack_print_maps(bufferfile, pid);

	if (pid > 1)
		dwfl = __open_dwfl_with_pid(pid, tid);
	else {
		fprintf(errfile,
				"Usage: %s [--output file] [--erroutput file] [--pid <pid> [--tid <tid>]]\n",
				argv[0]);
		return 1;
	}

	if (NULL == dwfl)
		return 1111;

	dwfl_getmodules(dwfl, __module_callback, NULL, 0);

	if (-1 == __get_registers_ptrace(tid))
		return 3333;

	/* Unwind call stack */
	Callstack callstack;
	callstack_constructor(&callstack);

	_create_dump_stack(dwfl, tid, &callstack);
	size_t it;
	for (it = 0; it != callstack.elems; ++it)
		__resolve_symbols(&callstack.proc[it], dwfl);

	/* Print registers */
	_dump_stack_print_regs(outputfile);

	/* Print pre-ptrace info */
	//__print_buffer_info(bufferfile, outputfile);

	/* Print the results */
	__print_callstack(&callstack, tid);

	/* Clean up */
	callstack_destructor(&callstack);
	dwfl_report_end(dwfl, NULL, NULL);
	dwfl_end(dwfl);

	return 0;
}
