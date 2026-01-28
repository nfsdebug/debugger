/**
 * @file main.c
 * @brief Refactored debugger with true interactive mode
 *
 * Interactive mode: Target is paused while waiting for commands
 * Uses PTRACE_SYSCALL to stop at each system call
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/personality.h>
#include <string.h>

/* libunwind for backtrace */
#ifdef HAVE_LIBUNWIND
#include <libunwind-ptrace.h>
#include <libunwind.h>
#endif

/* Display modules */
#include "display/output.h"
#include "display/theme.h"
#include "display/sections.h"

/* CLI modules */
#include "cli/parser.h"
#include "cli/config.h"

/* Core modules */
#include "core/breakpoints.h"
#include "core/symbols.h"
/* DWARF disabled due to libdwarf abort issue */
/* #include "core/dwarf.h" */

/* Global state */
static pid_t g_child_pid = -1;
static int g_running = 1;
static breakpoint_state_t g_breakpoints;
static symbol_table_t g_symbols;
/* DWARF disabled due to libdwarf abort issue */
/* static dwarf_state_t g_dwarf; */
static uint64_t g_base_address = 0;  /* Runtime base address */
static int g_at_breakpoint = 0;
static int g_current_bp_index = -1;

#ifdef HAVE_LIBUNWIND
static unw_addr_space_t g_as;
static struct UPT_info *g_ui;
#endif

/* === TARGET SPAWN === */

/* Get runtime base address from /proc/PID/maps */
static uint64_t get_base_address(pid_t pid) {
    char path[128];
    snprintf(path, sizeof(path), "/proc/%d/maps", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        return 0;
    }

    char line[512];
    uint64_t base = 0;

    while (fgets(line, sizeof(line), fp)) {
        /* Look for the executable segment with r-x permissions */
        unsigned long start, end;
        char perms[5];
        unsigned long offset;
        char dev[32];
        unsigned long inode;
        char pathname[256];

        if (sscanf(line, "%lx-%lx %4s %lx %s %lu %s", &start, &end, perms, &offset, dev, &inode, pathname) == 7) {
            /* Check if this is the executable segment */
            if (strstr(perms, "r-x") && pathname[0] == '/') {
                /* For PIE, ELF vaddr 0 maps to: start - offset */
                base = start - offset;
                break;
            }
        }
    }

    fclose(fp);
    return base;
}

static pid_t spawn_target(const char *program) {
    pid_t pid = fork();

    if (pid == 0) {
        /* Child process */
        personality(ADDR_NO_RANDOMIZE);

        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
            perror("ptrace TRACEME");
            exit(1);
        }

        execvp(program, (char *const[]){(char *)program, NULL});
        perror("execvp");
        exit(1);
    } else if (pid < 0) {
        perror("fork");
        return -1;
    }

    return pid;
}

/* === LIBUNWIND BACKTRACE === */

#ifdef HAVE_LIBUNWIND
static void print_backtrace(void) {
    unw_cursor_t c;
    unw_word_t ip, sp, off;
    int ret;
    int frame_count = 0;

    ret = unw_init_remote(&c, g_as, g_ui);
    if (ret < 0) {
        output_error("Failed to initialize unwinder");
        return;
    }

    section_print_header("BACKTRACE", sections_get_global_expand(), NULL);

    do {
        if ((ret = unw_get_reg(&c, UNW_REG_IP, &ip)) < 0 ||
            (ret = unw_get_reg(&c, UNW_REG_SP, &sp)) < 0) {
            break;
        }

        char buf[512];
        buf[0] = '\0';
        unw_get_proc_name(&c, buf, sizeof(buf), (unw_word_t *)&off);

        if (off) {
            size_t len = strlen(buf);
            if (len < sizeof(buf) - 32) {
                snprintf(buf + len, sizeof(buf) - len, "+0x%lx", (unsigned long)off);
            }
        }

        output_normal(CAT_BACKTRACE, "  #%2d  0x%016lx  %s\n",
                      frame_count++, (unsigned long)ip, buf);

        ret = unw_step(&c);

        if (frame_count > 100) {
            output_error("Backtrace limit reached");
            break;
        }

    } while (ret > 0);
}
#else
static void print_backtrace(void) {
    output_error("Backtrace not available (libunwind-ptrace not installed)");
    output_error("Install: sudo apt install libunwind-dev");
}
#endif

/* === COMMAND EXECUTION === */

static int cmd_continue(void) {
    /* Continue execution - target will run until breakpoint or signal */
    int status;

    /* If we're at a breakpoint, step past it first */
    if (g_at_breakpoint && g_current_bp_index >= 0) {
        /* Prepare to step past: restore instruction and adjust RIP */
        breakpoints_step_past_prepare(&g_breakpoints, g_child_pid, g_current_bp_index);

        /* Single step to execute the original instruction */
        ptrace(PTRACE_SINGLESTEP, g_child_pid, 0, 0);
        waitpid(g_child_pid, &status, 0);

        /* Check if exited during single step */
        if (WIFEXITED(status)) {
            output_normal(CAT_PROCESS, "Process exited with code %d\n", WEXITSTATUS(status));
            g_running = 0;
            return 0;
        }

        /* Re-set the INT3 breakpoint */
        breakpoints_step_past_finish(&g_breakpoints, g_child_pid, g_current_bp_index);

        g_at_breakpoint = 0;
        g_current_bp_index = -1;
    }

    /* Use PTRACE_CONT to run until breakpoint/signal (not syscall) */
    ptrace(PTRACE_CONT, g_child_pid, 0, 0);
    waitpid(g_child_pid, &status, 0);

    /* Check if exited */
    if (WIFEXITED(status)) {
        output_normal(CAT_PROCESS, "Process exited with code %d\n", WEXITSTATUS(status));
        g_running = 0;
        return 0;
    }

    /* Check for signal */
    if (WIFSTOPPED(status)) {
        int sig = WSTOPSIG(status);

        if (sig == SIGTRAP) {
            /* Check if we hit a breakpoint */
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, g_child_pid, NULL, &regs) < 0) {
                output_error("Failed to read registers after SIGTRAP");
            } else {
                int bp_idx = breakpoints_check_hit(&g_breakpoints, g_child_pid, regs.rip);
                if (bp_idx >= 0) {
                    /* Hit a breakpoint */
                    g_at_breakpoint = 1;
                    g_current_bp_index = bp_idx;
                    int hit_count = breakpoints_hit(&g_breakpoints, bp_idx);
                    output_normal(CAT_BREAKPOINT, "\n");
                    section_print_header("BREAKPOINT HIT", sections_get_global_expand(), NULL);
                    output_normal(CAT_BREAKPOINT, "  Breakpoint #%d hit at 0x%016lx (count: %d)\n\n",
                                  bp_idx, regs.rip - 1, hit_count);
                    return 1;
                }
            }
        } else if (sig == SIGSEGV || sig == SIGILL || sig == SIGFPE) {
            /* Crash signal */
            output_signal(sig, strsignal(sig));
            print_backtrace();
            g_running = 0;
            return 0;
        } else if (sig != SIGSTOP && sig != SIGTRAP) {
            /* Other signals */
            output_signal(sig, strsignal(sig));
        }
    }

    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        output_signal(sig, strsignal(sig));
        g_running = 0;
        return 0;
    }

    output_stats_inc_step();
    return 1;
}

static int cmd_single_step(void) {
    int status;

    /* If we're at a breakpoint, step past it first */
    if (g_at_breakpoint && g_current_bp_index >= 0) {
        /* Prepare to step past: restore instruction and adjust RIP */
        breakpoints_step_past_prepare(&g_breakpoints, g_child_pid, g_current_bp_index);

        /* Single step to execute the original instruction */
        ptrace(PTRACE_SINGLESTEP, g_child_pid, 0, 0);
        waitpid(g_child_pid, &status, 0);

        /* Check if exited during single step */
        if (WIFEXITED(status)) {
            output_normal(CAT_PROCESS, "Process exited with code %d\n", WEXITSTATUS(status));
            g_running = 0;
            return 0;
        }

        /* Re-set the INT3 breakpoint */
        breakpoints_step_past_finish(&g_breakpoints, g_child_pid, g_current_bp_index);

        g_at_breakpoint = 0;
        g_current_bp_index = -1;
        return 1;
    }

    ptrace(PTRACE_SINGLESTEP, g_child_pid, 0, 0);
    waitpid(g_child_pid, &status, 0);

    /* Check if exited */
    if (WIFEXITED(status)) {
        output_normal(CAT_PROCESS, "Process exited with code %d\n", WEXITSTATUS(status));
        g_running = 0;
        return 0;
    }

    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        output_signal(sig, strsignal(sig));

        if (sig == SIGSEGV || sig == SIGILL || sig == SIGFPE) {
            /* Show backtrace on crash */
            print_backtrace();
        }

        g_running = 0;
        return 0;
    }

    output_stats_inc_step();
    return 1;
}

static int cmd_register_dump(void) {
    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, g_child_pid, NULL, &regs) < 0) {
        output_error("Failed to read registers");
        return -1;
    }

    output_normal(CAT_REGISTERS, "Dumping registers:\n");
    output_normal(CAT_REGISTERS, "  rax = 0x%016llx  rbx = 0x%016llx\n", regs.rax, regs.rbx);
    output_normal(CAT_REGISTERS, "  rcx = 0x%016llx  rdx = 0x%016llx\n", regs.rcx, regs.rdx);
    output_normal(CAT_REGISTERS, "  rdi = 0x%016llx  rsi = 0x%016llx\n", regs.rdi, regs.rsi);
    output_normal(CAT_REGISTERS, "  rbp = 0x%016llx  rsp = 0x%016llx\n", regs.rbp, regs.rsp);
    output_normal(CAT_REGISTERS, "  rip = 0x%016llx\n\n", regs.rip);

    return 0;
}

static int cmd_register_read(const char *reg_name) {
    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, g_child_pid, NULL, &regs) < 0) {
        output_error("Failed to read registers");
        return -1;
    }

    uint64_t value = 0;
    const char *name = NULL;

    if (strcmp(reg_name, "rax") == 0) { value = regs.rax; name = "rax"; }
    else if (strcmp(reg_name, "rbx") == 0) { value = regs.rbx; name = "rbx"; }
    else if (strcmp(reg_name, "rcx") == 0) { value = regs.rcx; name = "rcx"; }
    else if (strcmp(reg_name, "rdx") == 0) { value = regs.rdx; name = "rdx"; }
    else if (strcmp(reg_name, "rdi") == 0) { value = regs.rdi; name = "rdi"; }
    else if (strcmp(reg_name, "rsi") == 0) { value = regs.rsi; name = "rsi"; }
    else if (strcmp(reg_name, "rbp") == 0) { value = regs.rbp; name = "rbp"; }
    else if (strcmp(reg_name, "rsp") == 0) { value = regs.rsp; name = "rsp"; }
    else if (strcmp(reg_name, "r8") == 0)  { value = regs.r8;  name = "r8"; }
    else if (strcmp(reg_name, "r9") == 0)  { value = regs.r9;  name = "r9"; }
    else if (strcmp(reg_name, "r10") == 0) { value = regs.r10; name = "r10"; }
    else if (strcmp(reg_name, "r11") == 0) { value = regs.r11; name = "r11"; }
    else if (strcmp(reg_name, "r12") == 0) { value = regs.r12; name = "r12"; }
    else if (strcmp(reg_name, "r13") == 0) { value = regs.r13; name = "r13"; }
    else if (strcmp(reg_name, "r14") == 0) { value = regs.r14; name = "r14"; }
    else if (strcmp(reg_name, "r15") == 0) { value = regs.r15; name = "r15"; }
    else if (strcmp(reg_name, "rip") == 0) { value = regs.rip; name = "rip"; }
    else {
        output_error("Unknown register: %s", reg_name);
        return -1;
    }

    output_normal(CAT_REGISTERS, "  %s = 0x%016llx\n", name, value);
    return 0;
}

static int cmd_memory_read(uint64_t addr) {
    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, g_child_pid, (void *)addr, NULL);

    if (errno != 0) {
        output_error("Failed to read memory at 0x%lx: %s", addr, strerror(errno));
        return -1;
    }

    output_normal(CAT_MEMORY, "  [0x%lx] = 0x%016lx\n", addr, (uint64_t)data);
    output_stats_inc_memory_read();
    return 0;
}

static int cmd_memory_write(uint64_t addr, uint64_t value) {
    errno = 0;
    long orig = ptrace(PTRACE_PEEKDATA, g_child_pid, (void *)addr, NULL);

    if (errno != 0) {
        output_error("Failed to read memory at 0x%lx", addr);
        return -1;
    }

    long new_data = (orig & ~0xFFFFFFFFULL) | (value & 0xFFFFFFFFULL);
    if (ptrace(PTRACE_POKEDATA, g_child_pid, (void *)addr, new_data) < 0) {
        output_error("Failed to write memory at 0x%lx", addr);
        return -1;
    }

    output_normal(CAT_MEMORY, "  [0x%lx] = 0x%016lx (was: 0x%016lx)\n",
                      addr, value, (uint64_t)orig);
    output_stats_inc_memory_write();
    return 0;
}

static int cmd_backtrace(void) {
    print_backtrace();
    return 0;
}

/* === BREAKPOINT COMMANDS === */

static int cmd_breakpoint_set_addr(uint64_t addr) {
    /* If address is small (< 0x10000), treat it as a virtual address that needs base adjustment */
    uint64_t runtime_addr = addr;
    if (addr < 0x10000) {
        runtime_addr = addr + g_base_address;
        output_normal(CAT_BREAKPOINT, "Virtual address 0x%lx -> runtime 0x%lx (base: 0x%lx)\n",
                      addr, runtime_addr, g_base_address);
    }
    int idx = breakpoints_add_addr(&g_breakpoints, (void *)runtime_addr, g_child_pid);
    if (idx >= 0) {
        output_stats_inc_breakpoint();
    }
    return idx;
}

static int cmd_breakpoint_set_current(void) {
    /* Set breakpoint at current RIP */
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, g_child_pid, NULL, &regs) < 0) {
        output_error("Failed to read registers");
        return -1;
    }

    output_normal(CAT_BREAKPOINT, "Setting breakpoint at current RIP: 0x%lx\n", regs.rip);
    return cmd_breakpoint_set_addr(regs.rip);
}

static int cmd_breakpoint_set_func(const char *func_spec) {
    /* Check for "function:line" format - DWARF disabled, show error */
    char *colon = strchr(func_spec, ':');
    if (colon) {
        output_error("Line number breakpoints (func:line) require DWARF support (currently disabled)");
        output_error("Use address breakpoints instead: disassemble target to find address");
        return -1;
    }

    /* Just function name - use symbol table */
    uint64_t vaddr = symbols_find_address(&g_symbols, func_spec);
    if (vaddr == 0) {
        output_error("Function not found: %s", func_spec);
        output_normal(CAT_PROCESS, "Tip: Use 'info functions' to list available functions\n");
        return -1;
    }

    /* Adjust to runtime address */
    uint64_t addr = vaddr + g_base_address;
    output_normal(CAT_BREAKPOINT, "Function '%s' resolved to 0x%lx (vaddr: 0x%lx + base: 0x%lx)\n",
                  func_spec, addr, vaddr, g_base_address);
    return cmd_breakpoint_set_addr(addr);
}

static int cmd_list_source(const char *file, int line, int count) {
    output_error("Source listing requires DWARF support (currently disabled)");
    return -1;
}

static int cmd_breakpoint_list(void) {
    breakpoints_list(&g_breakpoints);
    return 0;
}

static int cmd_breakpoint_enable(int index, int enable) {
    return breakpoints_enable(&g_breakpoints, index, g_child_pid, enable);
}

static int cmd_breakpoint_delete(int index) {
    return breakpoints_remove(&g_breakpoints, index, g_child_pid);
}

static int cmd_info_functions(void) {
    symbols_list_functions(&g_symbols);
    return 0;
}

/* === STEP OVER === */

static int cmd_step_over(void) {
    /* Step over: execute until next line in current function */
    /* This is a simple implementation - for full step-over we need to:
     * 1. Detect if current instruction is a CALL
     * 2. If yes, set breakpoint at return address and continue
     * 3. If no, just single step
     */

    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, g_child_pid, NULL, &regs) < 0) {
        output_error("Failed to read registers");
        return -1;
    }

    /* Read current instruction */
    errno = 0;
    unsigned long instr = ptrace(PTRACE_PEEKTEXT, g_child_pid, (void *)regs.rip, NULL);
    if (errno != 0) {
        output_error("Failed to read instruction");
        return -1;
    }

    /* Check for CALL instruction (0xE8 for rel call, 0xFF /2 for indirect) */
    unsigned char opcode = instr & 0xFF;
    int is_call = (opcode == 0xE8) || ((opcode == 0xFF) && (((instr >> 8) & 0x38) == 0x10));

    if (is_call) {
        /* Set breakpoint at return address and continue */
        uint64_t ret_addr = regs.rip + 5; /* Approximate - should decode instruction */
        int bp_idx = breakpoints_add_addr(&g_breakpoints, (void *)ret_addr, g_child_pid);
        if (bp_idx < 0) {
            output_error("Failed to set breakpoint for step over");
            return -1;
        }

        output_normal(CAT_PROCESS, "Stepping over function call (bp at 0x%lx)\n", ret_addr);
        return cmd_continue();
    } else {
        /* Not a call, just single step */
        return cmd_single_step();
    }
}

/* === MAIN LOOP === */

static int run_interactive(void) {
    char line[4096];

    printf("\n");
    parser_print_usage();
    printf("\n");

    while (g_running) {
        /* Show prompt */
        const char *prompt = "dbg> ";
        printf("%s", prompt);
        fflush(stdout);

        /* Read command */
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        /* Remove newline */
        line[strcspn(line, "\n")] = 0;

        /* Skip empty lines */
        if (line[0] == '\0') continue;

        /* Parse command */
        command_t cmd;
        if (parser_parse_command(line, &cmd) != 0) {
            output_error("Failed to parse command");
            continue;
        }

        /* Execute command */
        int continue_execution = 0;

        switch (cmd.type) {
            case CMD_CONTINUE:
            case CMD_SINGLE_STEP:
            case CMD_STEP_OVER:
                if (cmd.type == CMD_STEP_OVER) {
                    cmd_step_over();
                } else if (cmd.type == CMD_SINGLE_STEP) {
                    cmd_single_step();
                } else {
                    cmd_continue();
                }
                break;

            case CMD_REGISTER_DUMP:
                cmd_register_dump();
                break;

            case CMD_BREAKPOINT_ADDR:
                if (cmd.addr_arg > 0) {
                    cmd_breakpoint_set_addr(cmd.addr_arg);
                }
                break;

            case CMD_BREAKPOINT_FUNC:
                if (cmd.string_arg) {
                    cmd_breakpoint_set_func(cmd.string_arg);
                }
                break;

            case CMD_BREAKPOINT_CURRENT:
                cmd_breakpoint_set_current();
                break;

            case CMD_BREAKPOINT_LIST:
                cmd_breakpoint_list();
                break;

            case CMD_BREAKPOINT_ENABLE:
                cmd_breakpoint_enable(cmd.int_arg, 1);
                break;

            case CMD_BREAKPOINT_DISABLE:
                cmd_breakpoint_enable(cmd.int_arg, 0);
                break;

            case CMD_BREAKPOINT_DELETE:
                cmd_breakpoint_delete(cmd.int_arg);
                break;

            case CMD_REGISTER_READ:
                if (cmd.string_arg) {
                    cmd_register_read(cmd.string_arg);
                }
                break;

            case CMD_MEMORY_READ:
                if (cmd.addr_arg > 0) {
                    cmd_memory_read(cmd.addr_arg);
                }
                break;

            case CMD_MEMORY_WRITE:
                if (cmd.addr_arg > 0) {
                    cmd_memory_write(cmd.addr_arg, cmd.value_arg);
                }
                break;

            case CMD_BACKTRACE:
                cmd_backtrace();
                break;

            case CMD_LIST_SOURCE:
                if (cmd.string_arg) {
                    int line = (cmd.int_arg > 0) ? cmd.int_arg : 1;
                    int count = (cmd.value_arg > 0) ? (int)cmd.value_arg : 10;
                    cmd_list_source(cmd.string_arg, line, count);
                }
                break;

            case CMD_INFO_FUNCTIONS:
                cmd_info_functions();
                break;

            case CMD_SET_OUTPUT:
                if (cmd.int_arg >= OUTPUT_QUIET && cmd.int_arg <= OUTPUT_DEBUG) {
                    output_set_level((output_level_t)cmd.int_arg);
                    output_normal(CAT_PROCESS, "Output level set\n");
                }
                break;

            case CMD_SET_EXPAND:
                if (cmd.int_arg >= EXPAND_NONE && cmd.int_arg <= EXPAND_FULL) {
                    sections_set_global_expand((expand_level_t)cmd.int_arg);
                    output_normal(CAT_PROCESS, "Expand level set\n");
                }
                break;

            case CMD_HELP:
                parser_print_usage();
                break;

            case CMD_QUIT:
                g_running = 0;
                continue_execution = 0;
                break;

            default:
                output_error("Unknown command");
                break;
        }

        parser_free_command(&cmd);
    }

    return 0;
}

/* === MAIN === */

int main(int argc, char **argv) {
    /* Initialize display systems */
    theme_init(NULL);
    output_init(NULL);
    config_init(NULL);

    /* Check arguments */
    if (argc < 2) {
        output_error("Usage: %s <program> [args...]", argv[0]);
        printf("\nAvailable commands:\n");
        printf("  continue, c      - Continue execution (stops at syscalls)\n");
        printf("  step, s          - Single step\n");
        printf("  register dump    - Dump all registers\n");
        printf("  register read X  - Read register X\n");
        printf("  memory read X    - Read memory at address X\n");
        printf("  memory write X Y - Write value Y at address X\n");
        printf("  backtrace, bt     - Show backtrace\n");
        printf("  set output L     - Set output level (quiet/normal/verbose/debug)\n");
        printf("  set expand L     - Set expansion level (none/normal/full)\n");
        printf("  help, h          - Show this help\n");
        printf("  quit, q          - Quit debugger\n");
        return 1;
    }

    /* Spawn target process */
    g_child_pid = spawn_target(argv[1]);
    if (g_child_pid < 0) {
        output_error("Failed to spawn target process");
        return 1;
    }

    /* Wait for initial stop (execve) */
    int status;
    waitpid(g_child_pid, &status, 0);

    /* Get runtime base address */
    g_base_address = get_base_address(g_child_pid);

#ifdef HAVE_LIBUNWIND
    /* Initialize libunwind */
    g_as = unw_create_addr_space(&_UPT_accessors, 0);
    g_ui = _UPT_create(g_child_pid);
    if (!g_as || !g_ui) {
        output_error("Failed to initialize libunwind");
        output_error("Install: sudo apt install libunwind-dev libunwind-8-dev");
    }
#endif

    /* Initialize breakpoints */
    if (breakpoints_init(&g_breakpoints, 32) < 0) {
        output_error("Failed to initialize breakpoints");
        return 1;
    }

    /* Initialize symbols from ELF file */
    if (symbols_init(&g_symbols, argv[1]) < 0) {
        output_error("Failed to load symbols (continuing anyway)\n");
    }

    /* Initialize DWARF from program */
    // TODO: Fix libdwarf abort() issue - temporarily disabled
    // if (dwarf_load(&g_dwarf, argv[1]) < 0) {
    //     output_error("No DWARF info - compile with -g for source-level debugging\n");
    // }

    /* Print process info */
    section_print_separator(60);
    output_normal(CAT_PROCESS, "Target: %s\n", argv[1]);
    output_normal(CAT_PROCESS, "PID: %d  GID: %d\n", g_child_pid, getgid());
    section_print_separator(60);

    printf("\nDebugger started. Target is paused.\n");
    printf("Type 'help' for commands, 'quit' to exit.\n\n");

    /* Run interactive loop */
    run_interactive();

    /* Cleanup */
    /* dwarf_free(&g_dwarf); DWARF disabled */
    symbols_cleanup(&g_symbols);
    breakpoints_cleanup(&g_breakpoints, g_child_pid);
#ifdef HAVE_LIBUNWIND
    if (g_ui) _UPT_destroy(g_ui);
    if (g_as) unw_destroy_addr_space(g_as);
#endif

    output_close_log();

    printf("\nDebugger exited.\n");
    return 0;
}
