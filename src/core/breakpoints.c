/**
 * @file breakpoints.c
 * @brief Breakpoint management with INT3
 */

#include "breakpoints.h"
#include "../display/output.h"
#include "../display/sections.h"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/* INT3 opcode */
#define INT3 0xCC

int breakpoints_init(breakpoint_state_t *state, int capacity) {
    state->bps = calloc(capacity, sizeof(breakpoint_t));
    if (!state->bps) {
        return -1;
    }

    state->count = 0;
    state->capacity = capacity;
    return 0;
}

void breakpoints_cleanup(breakpoint_state_t *state, pid_t pid) {
    /* Restore all breakpoints */
    for (int i = 0; i < state->count; i++) {
        breakpoint_t *bp = &state->bps[i];
        if (bp->enabled) {
            /* Restore original byte */
            errno = 0;
            long data = ptrace(PTRACE_PEEKDATA, pid, (void *)bp->actual_address, NULL);
            if (errno == 0) {
                long restored = (data & ~0xFF) | bp->original_byte;
                ptrace(PTRACE_POKEDATA, pid, (void *)bp->actual_address, (void *)restored);
            }
        }
    }

    free(state->bps);
    state->bps = NULL;
    state->count = 0;
    state->capacity = 0;
}

static int set_breakpoint_int3(pid_t pid, breakpoint_t *bp) {
    /* Read current instruction */
    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, pid, (void *)bp->actual_address, NULL);
    if (errno != 0) {
        output_error("Failed to read memory at 0x%lx: %s", bp->actual_address, strerror(errno));
        return -1;
    }

    /* Save original byte */
    bp->original_byte = data & 0xFF;

    /* Check if already INT3 (breakpoint already set) */
    if (bp->original_byte == INT3) {
        output_error("Breakpoint already set at 0x%lx", bp->actual_address);
        return -1;
    }

    /* Write INT3 */
    long patched = (data & ~0xFF) | INT3;
    if (ptrace(PTRACE_POKEDATA, pid, (void *)bp->actual_address, (void *)patched) < 0) {
        output_error("Failed to write INT3 at 0x%lx: %s", bp->actual_address, strerror(errno));
        return -1;
    }

    return 0;
}

static int restore_breakpoint(pid_t pid, breakpoint_t *bp) {
    /* Restore original byte */
    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, pid, (void *)bp->actual_address, NULL);
    if (errno != 0) {
        return -1;
    }

    long restored = (data & ~0xFF) | bp->original_byte;
    if (ptrace(PTRACE_POKEDATA, pid, (void *)bp->actual_address, (void *)restored) < 0) {
        return -1;
    }

    return 0;
}

int breakpoints_add_addr(breakpoint_state_t *state, void *addr, pid_t pid) {
    if (state->count >= state->capacity) {
        output_error("Breakpoint list full (max %d)", state->capacity);
        return -1;
    }

    breakpoint_t *bp = &state->bps[state->count];

    bp->type = BP_ADDRESS;
    bp->location.address = addr;
    bp->actual_address = (uint64_t)addr;
    bp->enabled = 0;
    bp->hit_count = 0;

    /* Set INT3 */
    if (set_breakpoint_int3(pid, bp) < 0) {
        return -1;
    }
    bp->enabled = 1;

    output_normal(CAT_BREAKPOINT, "Breakpoint #%d set at 0x%lx\n",
                  state->count, (uint64_t)addr);

    state->count++;
    return state->count - 1;  /* Return breakpoint index */
}

int breakpoints_add_func(breakpoint_state_t *state, const char *func_name,
                         pid_t pid, uint64_t offset) {
    if (state->count >= state->capacity) {
        output_error("Breakpoint list full (max %d)", state->capacity);
        return -1;
    }

    /* For now, use offset directly (DWARF resolution later) */
    breakpoint_t *bp = &state->bps[state->count];

    bp->type = BP_FUNCTION;
    strncpy(bp->location.func_name, func_name, sizeof(bp->location.func_name) - 1);
    bp->location.func_name[sizeof(bp->location.func_name) - 1] = '\0';
    bp->actual_address = offset;
    bp->enabled = 0;
    bp->hit_count = 0;

    /* Set INT3 */
    if (set_breakpoint_int3(pid, bp) < 0) {
        return -1;
    }
    bp->enabled = 1;

    output_normal(CAT_BREAKPOINT, "Breakpoint #%d set at %s (0x%lx)\n",
                  state->count, func_name, offset);

    state->count++;
    return state->count - 1;  /* Return breakpoint index */
}

int breakpoints_remove(breakpoint_state_t *state, int index, pid_t pid) {
    if (index < 0 || index >= state->count) {
        output_error("Invalid breakpoint index: %d", index);
        return -1;
    }

    breakpoint_t *bp = &state->bps[index];

    /* Restore original byte if enabled */
    if (bp->enabled) {
        restore_breakpoint(pid, bp);
    }

    output_normal(CAT_BREAKPOINT, "Breakpoint #%d removed\n", index);

    /* Shift remaining breakpoints */
    for (int i = index; i < state->count - 1; i++) {
        state->bps[i] = state->bps[i + 1];
    }
    state->count--;

    return 0;
}

int breakpoints_find_by_addr(breakpoint_state_t *state, uint64_t addr) {
    for (int i = 0; i < state->count; i++) {
        if (state->bps[i].actual_address == addr) {
            return i;
        }
    }
    return -1;
}

int breakpoints_enable(breakpoint_state_t *state, int index, pid_t pid, int enable) {
    if (index < 0 || index >= state->count) {
        output_error("Invalid breakpoint index: %d", index);
        return -1;
    }

    breakpoint_t *bp = &state->bps[index];

    if (enable && !bp->enabled) {
        /* Enable: write INT3 */
        if (set_breakpoint_int3(pid, bp) < 0) {
            return -1;
        }
        bp->enabled = 1;
        output_normal(CAT_BREAKPOINT, "Breakpoint #%d enabled\n", index);
    } else if (!enable && bp->enabled) {
        /* Disable: restore original byte */
        if (restore_breakpoint(pid, bp) < 0) {
            return -1;
        }
        bp->enabled = 0;
        output_normal(CAT_BREAKPOINT, "Breakpoint #%d disabled\n", index);
    }

    return 0;
}

int breakpoints_hit(breakpoint_state_t *state, int index) {
    if (index < 0 || index >= state->count) {
        return -1;
    }

    state->bps[index].hit_count++;
    return state->bps[index].hit_count;
}

void breakpoints_list(breakpoint_state_t *state) {
    if (state->count == 0) {
        output_normal(CAT_BREAKPOINT, "No breakpoints set\n");
        return;
    }

    section_print_header("BREAKPOINTS", sections_get_global_expand(), NULL);
    output_normal(CAT_BREAKPOINT, "  Total: %d breakpoint(s)\n\n", state->count);

    for (int i = 0; i < state->count; i++) {
        breakpoint_t *bp = &state->bps[i];
        const char *status = bp->enabled ? "[+]" : "[_]";

        if (bp->type == BP_FUNCTION) {
            output_normal(CAT_BREAKPOINT, "  %s #%2d  %s @ 0x%016lx  hits: %d\n",
                          status, i, bp->location.func_name, bp->actual_address, bp->hit_count);
        } else {
            output_normal(CAT_BREAKPOINT, "  %s #%2d  0x%016lx  hits: %d\n",
                          status, i, bp->actual_address, bp->hit_count);
        }
    }
}

/* Check if we hit a breakpoint and return the breakpoint index */
int breakpoints_check_hit(breakpoint_state_t *state, pid_t pid, uint64_t rip) {
    (void)pid; /* Unused parameter - kept for API compatibility */
    /* When INT3 is hit, RIP points to the instruction AFTER the INT3 */
    /* We need to check RIP-1 */
    uint64_t bp_addr = rip - 1;

    for (int i = 0; i < state->count; i++) {
        if (state->bps[i].enabled && state->bps[i].actual_address == bp_addr) {
            return i;
        }
    }

    return -1;
}

/* Prepare to step past a breakpoint (restore instruction, adjust RIP) - NO WAIT */
int breakpoints_step_past_prepare(breakpoint_state_t *state, pid_t pid, int bp_index) {
    if (bp_index < 0 || bp_index >= state->count) {
        return -1;
    }

    breakpoint_t *bp = &state->bps[bp_index];

    /* 1. Restore original instruction */
    if (restore_breakpoint(pid, bp) < 0) {
        output_error("Failed to restore instruction at breakpoint");
        return -1;
    }

    /* 2. Adjust RIP back to breakpoint address */
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
        output_error("Failed to get registers");
        return -1;
    }

    regs.rip = bp->actual_address;
    if (ptrace(PTRACE_SETREGS, pid, NULL, &regs) < 0) {
        output_error("Failed to set RIP");
        return -1;
    }

    return 0;
}

/* Re-set breakpoint after stepping past (called after single step completes) */
int breakpoints_step_past_finish(breakpoint_state_t *state, pid_t pid, int bp_index) {
    if (bp_index < 0 || bp_index >= state->count) {
        return -1;
    }

    breakpoint_t *bp = &state->bps[bp_index];

    /* Re-set INT3 */
    if (set_breakpoint_int3(pid, bp) < 0) {
        output_error("Failed to re-set breakpoint");
        return -1;
    }

    return 0;
}
