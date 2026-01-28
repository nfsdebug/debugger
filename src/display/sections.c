/**
 * @file sections.c
 * @brief Modular output sections (backtrace, registers, memory)
 */

#include "sections.h"
#include <stdio.h>
#include <string.h>

/* Forward declaration - avoid circular dependency */
struct debugger_state;

/* Global expand level */
static expand_level_t g_global_expand = EXPAND_NONE;

/* === BACKTRACE === */

static backtrace_config_t g_backtrace_config = {
    .expand = EXPAND_NONE,
    .show_addresses = 1,
    .show_locals = 0
};

void backtrace_print(const debugger_state_t *state, const backtrace_config_t *config) {
    const backtrace_config_t *cfg = config ? config : &g_backtrace_config;

    if (cfg->expand == EXPAND_NONE) {
        output_normal(CAT_BACKTRACE, "[BACKTRACE]\n");
    } else {
        output_normal(CAT_BACKTRACE, "[BACKTRACE - EXPANDED]\n");
    }

    /* TODO: Implement backtrace with libunwind */
    /* For now, just a placeholder */

    output_normal(CAT_BACKTRACE, "  #0 funcdetest2+0xcc  test.c:58\n");
    output_normal(CAT_BACKTRACE, "  #1 funcdetest+0x21   test.c:65\n");
    output_normal(CAT_BACKTRACE, "  #2 main+0x24         test.c:80\n");
}

void backtrace_set_expand(expand_level_t level) {
    g_backtrace_config.expand = level;
}

/* === REGISTERS === */

static registers_config_t g_registers_config = {
    .expand = EXPAND_NONE,
    .groups = 1,
    .filter_regs = NULL
};

void registers_print(const registers_config_t *config) {
    /* TODO: Implement register printing */
    output_normal(CAT_REGISTERS, "[REGISTERS]\n");
    output_normal(CAT_REGISTERS, "  rax: 0x0000000000000000  rbx: 0x00007fffffffdde8\n");
}

void registers_set_expand(expand_level_t level) {
    g_registers_config.expand = level;
}

void registers_set_filter(const char *regs) {
    if (g_registers_config.filter_regs) {
        free(g_registers_config.filter_regs);
    }

    if (regs) {
        g_registers_config.filter_regs = strdup(regs);
    } else {
        g_registers_config.filter_regs = NULL;
    }
}

/* === MEMORY === */

void memory_print(const memory_config_t *config) {
    /* TODO: Implement memory printing */
    if (config && config->address) {
        output_normal(CAT_MEMORY, "[MEMORY @ %p]\n", config->address);
    }
}

/* === PROCESS INFO === */

void process_info_print(const debugger_state_t *state) {
    if (!state) return;

    output_normal(CAT_PROCESS, "[PROCESS] %s  PID:%d  Offset:0x%lx\n",
                  state->proc.path, state->proc.pid, state->proc.offset);
}

/* === GLOBAL EXPANSION === */

void sections_set_global_expand(expand_level_t level) {
    g_global_expand = level;
    backtrace_set_expand(level);
    registers_set_expand(level);
}

expand_level_t sections_get_global_expand(void) {
    return g_global_expand;
}
