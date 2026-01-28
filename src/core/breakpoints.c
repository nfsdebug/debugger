/**
 * @file breakpoints.c
 * @brief Breakpoint management
 */

#include "breakpoints.h"
#include <sys/ptrace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int breakpoints_init(breakpoint_state_t *state, int capacity) {
    state->bps = calloc(capacity, sizeof(breakpoint_t));
    if (!state->bps) {
        return -1;
    }

    state->count = 0;
    state->capacity = capacity;
    return 0;
}

void breakpoints_cleanup(breakpoint_state_t *state) {
    /* TODO: Remove all breakpoints (restore original bytes) */
    free(state->bps);
    state->bps = NULL;
    state->count = 0;
    state->capacity = 0;
}

int breakpoints_add_func(breakpoint_state_t *state, const char *func_name,
                         uint64_t offset, uint64_t *actual_addr) {
    if (state->count >= state->capacity) {
        return -1;  /* Full */
    }

    /* TODO: Resolve function name to address using DWARF */
    /* For now, use offset directly */
    breakpoint_t *bp = &state->bps[state->count];

    bp->type = BP_FUNCTION;
    strncpy(bp->location.func_name, func_name, sizeof(bp->location.func_name) - 1);
    bp->actual_address = offset;

    /* Read original byte */
    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, 0, (void *)offset, NULL);
    if (errno != 0) {
        return -1;
    }
    bp->original_byte = data & 0xFF;

    /* Write INT3 (0xCC) */
    long patched = (data & ~0xFF) | 0xCC;
    if (ptrace(PTRACE_POKEDATA, 0, (void *)offset, (void *)patched) < 0) {
        return -1;
    }

    bp->enabled = 1;
    bp->hit_count = 0;

    if (actual_addr) {
        *actual_addr = offset;
    }

    state->count++;
    return 0;
}

int breakpoints_add_addr(breakpoint_state_t *state, void *addr) {
    if (state->count >= state->capacity) {
        return -1;  /* Full */
    }

    breakpoint_t *bp = &state->bps[state->count];

    bp->type = BP_ADDRESS;
    bp->location.address = addr;
    bp->actual_address = (uint64_t)addr;

    /* Read original byte */
    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, 0, addr, NULL);
    if (errno != 0) {
        return -1;
    }
    bp->original_byte = data & 0xFF;

    /* Write INT3 (0xCC) */
    long patched = (data & ~0xFF) | 0xCC;
    if (ptrace(PTRACE_POKEDATA, 0, addr, (void *)patched) < 0) {
        return -1;
    }

    bp->enabled = 1;
    bp->hit_count = 0;

    state->count++;
    return 0;
}

int breakpoints_remove(breakpoint_state_t *state, int index) {
    if (index < 0 || index >= state->count) {
        return -1;
    }

    /* TODO: Restore original byte */
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

int breakpoints_enable(breakpoint_state_t *state, int index, int enable) {
    if (index < 0 || index >= state->count) {
        return -1;
    }

    /* TODO: Write/remove INT3 */
    state->bps[index].enabled = enable;
    return 0;
}

int breakpoints_hit(breakpoint_state_t *state, int index) {
    if (index < 0 || index >= state->count) {
        return -1;
    }

    state->bps[index].hit_count++;
    return 0;
}

void breakpoints_list(breakpoint_state_t *state) {
    printf("[BREAKPOINTS - %d]\n", state->count);

    for (int i = 0; i < state->count; i++) {
        breakpoint_t *bp = &state->bps[i];

        if (bp->type == BP_FUNCTION) {
            printf("  #%d %s @ 0x%lx [%s] hits: %d\n",
                   i, bp->location.func_name, bp->actual_address,
                   bp->enabled ? "enabled" : "disabled",
                   bp->hit_count);
        } else {
            printf("  #%d 0x%lx [%s] hits: %d\n",
                   i, bp->actual_address,
                   bp->enabled ? "enabled" : "disabled",
                   bp->hit_count);
        }
    }
}
