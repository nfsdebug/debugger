/**
 * @file breakpoints.h
 * @brief Breakpoint management
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

/* Cleanup breakpoint state */
void breakpoints_cleanup(breakpoint_state_t *state);

/* Add breakpoint by function name */
int breakpoints_add_func(breakpoint_state_t *state, const char *func_name,
                         uint64_t offset, uint64_t *actual_addr);

/* Add breakpoint by address */
int breakpoints_add_addr(breakpoint_state_t *state, void *addr);

/* Remove breakpoint */
int breakpoints_remove(breakpoint_state_t *state, int index);

/* Find breakpoint by address */
int breakpoints_find_by_addr(breakpoint_state_t *state, uint64_t addr);

/* Enable/disable breakpoint */
int breakpoints_enable(breakpoint_state_t *state, int index, int enable);

/* Hit breakpoint (call when breakpoint is hit) */
int breakpoints_hit(breakpoint_state_t *state, int index);

/* List all breakpoints */
void breakpoints_list(breakpoint_state_t *state);

#endif /* BREAKPOINTS_H */
