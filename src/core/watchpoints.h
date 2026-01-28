/**
 * @file watchpoints.h
 * @brief Hardware watchpoints using x86-64 debug registers
 */

#ifndef WATCHPOINTS_H
#define WATCHPOINTS_H

#include <stdint.h>
#include <sys/types.h>

/* Watchpoint types */
typedef enum {
    WP_WRITE,       /* Break on write only */
    WP_READ,        /* Break on read only */
    WP_READ_WRITE   /* Break on read or write */
} wp_type_t;

/* Watchpoint state */
typedef struct {
    uint64_t address;
    wp_type_t type;
    int size;           /* 1, 2, 4, or 8 bytes */
    int enabled;
    int reg_index;      /* Which debug register (0-3) */
} watchpoint_t;

/* Global watchpoint state */
typedef struct {
    watchpoint_t watchpoints[4];  /* x86-64 has 4 debug registers */
    int count;
    pid_t pid;
} wp_state_t;

/* Initialize watchpoint state */
int wp_init(wp_state_t *state, pid_t pid);

/* Cleanup watchpoint state */
void wp_cleanup(wp_state_t *state);

/* Add a watchpoint */
int wp_add(wp_state_t *state, uint64_t addr, wp_type_t type, int size);

/* Remove a watchpoint by index */
int wp_remove(wp_state_t *state, int index);

/* Enable/disable a watchpoint */
int wp_enable(wp_state_t *state, int index, int enable);

/* List all watchpoints */
void wp_list(wp_state_t *state);

/* Check if a watchpoint was hit */
int wp_check_hit(wp_state_t *state);

/* Debug function to print DR7 status */
void wp_debug_status(wp_state_t *state);

#endif /* WATCHPOINTS_H */
