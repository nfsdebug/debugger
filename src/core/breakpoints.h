/**
 * @file breakpoints.h
 * @brief Breakpoint management with INT3
 */

#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include <stdint.h>
#include <sys/types.h>

/* Breakpoint types */
typedef enum {
    BP_NONE = 0,
    BP_FUNCTION,      /* Breakpoint by function name */
    BP_ADDRESS,       /* Breakpoint by address */
    BP_CONDITIONAL     /* Not implemented yet */
} bp_type_t;

/* Single breakpoint */
typedef struct {
    bp_type_t type;
    union {
        char func_name[128];    /* For BP_FUNCTION */
        void *address;           /* For BP_ADDRESS */
    } location;
    uint64_t actual_address;     /* Resolved address (with offset) */
    uint8_t original_byte;       /* Original instruction byte */
    int enabled;
    int hit_count;
} breakpoint_t;

/* Breakpoint state */
typedef struct {
    breakpoint_t *bps;
    int count;
    int capacity;
} breakpoint_state_t;

/* Initialize breakpoint state */
int breakpoints_init(breakpoint_state_t *state, int capacity);

/* Cleanup breakpoint state (restores all breakpoints) */
void breakpoints_cleanup(breakpoint_state_t *state, pid_t pid);

/* Add breakpoint by function name */
int breakpoints_add_func(breakpoint_state_t *state, const char *func_name,
                         pid_t pid, uint64_t offset);

/* Add breakpoint by address */
int breakpoints_add_addr(breakpoint_state_t *state, void *addr, pid_t pid);

/* Remove breakpoint */
int breakpoints_remove(breakpoint_state_t *state, int index, pid_t pid);

/* Find breakpoint by address */
int breakpoints_find_by_addr(const breakpoint_state_t *state, uint64_t addr);

/* Enable/disable breakpoint */
int breakpoints_enable(breakpoint_state_t *state, int index, pid_t pid, int enable);

/* Hit breakpoint (call when breakpoint is hit) */
int breakpoints_hit(breakpoint_state_t *state, int index);

/* List all breakpoints */
void breakpoints_list(const breakpoint_state_t *state);

/* Check if we hit a breakpoint and return the breakpoint index */
int breakpoints_check_hit(const breakpoint_state_t *state, pid_t pid, uint64_t rip);

/* Step past a breakpoint (restore instruction, single step, re-set INT3) */
int breakpoints_step_past(breakpoint_state_t *state, pid_t pid, int bp_index);

/* Prepare to step past a breakpoint (restore instruction, adjust RIP) - NO WAIT */
int breakpoints_step_past_prepare(breakpoint_state_t *state, pid_t pid, int bp_index);

/* Re-set breakpoint after stepping past (called after single step completes) */
int breakpoints_step_past_finish(breakpoint_state_t *state, pid_t pid, int bp_index);

#endif /* BREAKPOINTS_H */
