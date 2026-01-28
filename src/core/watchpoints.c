/**
 * @file watchpoints.c
 * @brief Hardware watchpoints using x86-64 debug registers
 *
 * x86-64 Debug Registers:
 * - DR0-DR3: Watchpoint addresses (4 watchpoints max)
 * - DR4-DR5: Reserved
 * - DR6: Debug status (which breakpoint triggered)
 * - DR7: Debug control (enable/disable, type, size)
 */

#include "watchpoints.h"
#include "../display/output.h"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>  /* For offsetof */

/* Offsets for debug registers in PTRACE_PEEKUSER/POKEUSER on x86_64 */
#ifndef DR_OFFSET
#define DR_OFFSET(reg) (offsetof(struct user, u_debugreg[reg]))
#endif

/* DR7 control register bits */
#define DR7_LOCAL_ENABLE_SHIFT  0   /* Bits 0, 2, 4, 6 for local enable */
#define DR7_GLOBAL_ENABLE_SHIFT 1   /* Bits 1, 3, 5, 7 for global enable */
#define DR7_RW_SHIFT            16  /* Bits 16-17, 20-21, 24-25, 28-29 for R/W */
#define DR7_LEN_SHIFT           18  /* Bits 18-19, 22-23, 26-27, 30-31 for length */

/* R/W values for DR7 */
#define DR7_RW_EXECUTE    0x00  /* Break on instruction execution */
#define DR7_RW_WRITE      0x01  /* Break on data write */
#define DR7_RW_IO         0x02  /* Break on I/O read/write */
#define DR7_RW_READ_WRITE 0x03  /* Break on data read/write */

/* Length values for DR7 */
#define DR7_LEN_1_BYTE    0x00
#define DR7_LEN_2_BYTES   0x01
#define DR7_LEN_4_BYTES   0x02
#define DR7_LEN_8_BYTES   0x03

/* Read a debug register */
static unsigned long read_dr(pid_t pid, int reg_num) {
    errno = 0;
    unsigned long value = ptrace(PTRACE_PEEKUSER, pid, (void *)(long)DR_OFFSET(reg_num), NULL);
    if (errno != 0) {
        output_error("Failed to read DR%d: %s", reg_num, strerror(errno));
        return 0;
    }
    return value;
}

/* Write a debug register */
static int write_dr(pid_t pid, int reg_num, unsigned long value) {
    errno = 0;
    if (ptrace(PTRACE_POKEUSER, pid, (void *)(long)DR_OFFSET(reg_num), (void *)value) < 0) {
        output_error("Failed to write DR%d: %s", reg_num, strerror(errno));
        return -1;
    }
    return 0;
}

/* Get length value for DR7 */
static int dr7_len_value(int size) {
    switch (size) {
        case 1: return DR7_LEN_1_BYTE;
        case 2: return DR7_LEN_2_BYTES;
        case 4: return DR7_LEN_4_BYTES;
        case 8: return DR7_LEN_8_BYTES;
        default: return DR7_LEN_1_BYTE;
    }
}

/* Get R/W value for DR7 */
static int dr7_rw_value(wp_type_t type) {
    /* x86_64 hardware only supports: 0=execute, 1=write, 2=I/O */
    /* Read-only watchpoints are emulated by removing write permissions */
    switch (type) {
        case WP_WRITE:       return DR7_RW_WRITE;
        case WP_READ:        return DR7_RW_WRITE;  /* Use write for all data watchpoints */
        case WP_READ_WRITE:  return DR7_RW_WRITE;
        default:             return DR7_RW_WRITE;
    }
}

/* Find a free debug register */
static int find_free_reg(wp_state_t *state) {
    for (int i = 0; i < 4; i++) {
        if (!state->watchpoints[i].enabled) {
            return i;
        }
    }
    return -1;
}

/* Update DR7 register based on current watchpoints */
static int update_dr7(wp_state_t *state) {
    unsigned long dr7 = 0;

    for (int i = 0; i < 4; i++) {
        if (state->watchpoints[i].enabled) {
            /* Enable locally (bit 0, 2, 4, 6) */
            dr7 |= (1UL << (DR7_LOCAL_ENABLE_SHIFT + i * 2));

            /* Set R/W (bits 16-17, 20-21, 24-25, 28-29) */
            int rw = dr7_rw_value(state->watchpoints[i].type);
            dr7 |= ((unsigned long)rw << (DR7_RW_SHIFT + i * 4));

            /* Set length (bits 18-19, 22-23, 26-27, 30-31) */
            int len = dr7_len_value(state->watchpoints[i].size);
            dr7 |= ((unsigned long)len << (DR7_LEN_SHIFT + i * 4));
        }
    }

    return write_dr(state->pid, 7, dr7);
}

int wp_init(wp_state_t *state, pid_t pid) {
    memset(state, 0, sizeof(*state));
    state->pid = pid;
    state->count = 0;
    return 0;
}

void wp_cleanup(wp_state_t *state) {
    /* Disable all watchpoints by clearing DR7 */
    if (state->pid > 0) {
        write_dr(state->pid, 7, 0);
    }
    memset(state, 0, sizeof(*state));
}

int wp_add(wp_state_t *state, uint64_t addr, wp_type_t type, int size) {
    /* Find free register */
    int reg_idx = find_free_reg(state);
    if (reg_idx < 0) {
        output_error("No available debug registers (max 4 watchpoints)");
        return -1;
    }

    /* Validate size */
    if (size != 1 && size != 2 && size != 4 && size != 8) {
        output_error("Invalid watchpoint size: %d (must be 1, 2, 4, or 8)", size);
        return -1;
    }

    /* Align address (must be aligned to size) */
    uint64_t aligned_addr = addr & ~(uint64_t)(size - 1);
    if (addr != aligned_addr) {
        output_normal(CAT_PROCESS, "Warning: Address 0x%lx aligned to 0x%lx (size=%d)\n",
                     addr, aligned_addr, size);
        addr = aligned_addr;
    }

    /* Set watchpoint address */
    if (write_dr(state->pid, reg_idx, addr) < 0) {
        output_error("Failed to set watchpoint address in DR%d", reg_idx);
        return -1;
    }

    /* Store watchpoint info */
    state->watchpoints[reg_idx].address = addr;
    state->watchpoints[reg_idx].type = type;
    state->watchpoints[reg_idx].size = size;
    state->watchpoints[reg_idx].enabled = 1;
    state->watchpoints[reg_idx].reg_index = reg_idx;
    state->count++;

    /* Update DR7 */
    if (update_dr7(state) < 0) {
        output_error("Failed to update DR7");
        return -1;
    }

    return reg_idx;
}

int wp_remove(wp_state_t *state, int index) {
    if (index < 0 || index >= 4) {
        output_error("Invalid watchpoint index: %d", index);
        return -1;
    }

    if (!state->watchpoints[index].enabled) {
        output_error("Watchpoint %d is not enabled", index);
        return -1;
    }

    /* Disable watchpoint */
    state->watchpoints[index].enabled = 0;
    state->count--;

    /* Update DR7 */
    if (update_dr7(state) < 0) {
        output_error("Failed to update DR7");
        return -1;
    }

    return 0;
}

int wp_enable(wp_state_t *state, int index, int enable) {
    if (index < 0 || index >= 4) {
        output_error("Invalid watchpoint index: %d", index);
        return -1;
    }

    if (enable) {
        if (!state->watchpoints[index].enabled) {
            state->watchpoints[index].enabled = 1;
            state->count++;
        }
    } else {
        if (state->watchpoints[index].enabled) {
            state->watchpoints[index].enabled = 0;
            state->count--;
        }
    }

    /* Update DR7 */
    return update_dr7(state);
}

void wp_list(wp_state_t *state) {
    output_normal(CAT_PROCESS, "\n  Watchpoints:\n");
    int found = 0;

    for (int i = 0; i < 4; i++) {
        if (state->watchpoints[i].enabled) {
            const char *type_str;
            switch (state->watchpoints[i].type) {
                case WP_WRITE:       type_str = "write"; break;
                case WP_READ:        type_str = "read"; break;
                case WP_READ_WRITE:  type_str = "read/write"; break;
                default:             type_str = "unknown"; break;
            }

            output_normal(CAT_PROCESS, "    [%d] 0x%016lx  %s  (size: %d)\n",
                         i, state->watchpoints[i].address, type_str,
                         state->watchpoints[i].size);
            found = 1;
        }
    }

    if (!found) {
        output_normal(CAT_PROCESS, "    No watchpoints set\n");
    }
}

int wp_check_hit(wp_state_t *state) {
    /* Read DR6 to see which breakpoint triggered */
    unsigned long dr6 = read_dr(state->pid, 6);
    if (dr6 == 0) {
        return -1;
    }

    /* Check bits 0-3 for watchpoint hits */
    for (int i = 0; i < 4; i++) {
        if (dr6 & (1UL << i)) {
            if (state->watchpoints[i].enabled) {
                /* Clear DR6 by writing it */
                write_dr(state->pid, 6, dr6);
                return i;
            }
        }
    }

    return -1;
}

/* Debug function to print DR7 status */
void wp_debug_status(wp_state_t *state) {
    unsigned long dr7 = read_dr(state->pid, 7);
    output_normal(CAT_PROCESS, "DR7 status: 0x%lx\n", dr7);
    for (int i = 0; i < 4; i++) {
        unsigned long dr = read_dr(state->pid, i);
        if (dr != 0 || state->watchpoints[i].enabled) {
            output_normal(CAT_PROCESS, "  DR%d: 0x%lx (enabled=%d)\n",
                         i, dr, state->watchpoints[i].enabled);
        }
    }
}
