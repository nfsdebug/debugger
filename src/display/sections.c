/**
 * @file sections.c
 * @brief Modular output sections (backtrace, registers, memory)
 */

#include "sections.h"
#include "output.h"
#include "theme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Section visibility mask */
static section_type_t g_visible_sections = SECTION_ALL;

/* Global expand level */
static expand_level_t g_global_expand = EXPAND_NONE;

/* === BACKTRACE === */

static backtrace_config_t g_backtrace_config = {
    .expand = EXPAND_NONE,
    .frame_count = 0,
    .show_addresses = 1,
    .show_locals = 0
};

void backtrace_print(const debugger_state_t *state, const backtrace_config_t *config) {
    if (!(g_visible_sections & SECTION_BACKTRACE)) {
        return;
    }

    const backtrace_config_t *cfg = config ? config : &g_backtrace_config;

    /* Only print in NORMAL or VERBOSE mode */
    if (output_get_level() < OUTPUT_NORMAL && cfg->expand == EXPAND_NONE) {
        return;
    }

    /* Section header */
    section_print_header("BACKTRACE", cfg->expand,
                        cfg->expand == EXPAND_FULL ? "FULL" : NULL);

    /* TODO: Implement backtrace with libunwind */
    /* For now, placeholder data */
    if (cfg->expand == EXPAND_FULL) {
        output_verbose(CAT_BACKTRACE, "  #0 funcdetest2+0xcc  [test.c:58:11]\n");
        output_verbose(CAT_BACKTRACE, "  #1 funcdetest+0x21   [test.c:65:5]\n");
        output_verbose(CAT_BACKTRACE, "  #2 main+0x24         [test.c:80:5]\n");
    } else {
        output_normal(CAT_BACKTRACE, "  #0 funcdetest2+0xcc  test.c:58\n");
        output_normal(CAT_BACKTRACE, "  #1 funcdetest+0x21   test.c:65\n");
        output_normal(CAT_BACKTRACE, "  #2 main+0x24         test.c:80\n");
    }

    (void)state; /* TODO: Use state for real backtrace */
}

void backtrace_set_expand(expand_level_t level) {
    g_backtrace_config.expand = level;
}

void backtrace_toggle_expand(void) {
    /* Cycle through: NONE -> NORMAL -> FULL -> NONE */
    g_backtrace_config.expand = (g_backtrace_config.expand + 1) % 3;
}

expand_level_t backtrace_get_expand(void) {
    return g_backtrace_config.expand;
}

/* === REGISTERS === */

static registers_config_t g_registers_config = {
    .expand = EXPAND_NONE,
    .groups = 1,
    .filter_regs = NULL
};

void registers_print(const registers_config_t *config) {
    if (!(g_visible_sections & SECTION_REGISTERS)) {
        return;
    }

    const registers_config_t *cfg = config ? config : &g_registers_config;

    /* Section header */
    section_print_header("REGISTERS", cfg->expand, NULL);

    /* TODO: Implement real register reading */
    /* For now, placeholder data */
    if (cfg->expand == EXPAND_FULL) {
        /* Detailed view: all registers on separate lines */
        output_verbose(CAT_REGISTERS, "  rax: 0x0000000000000000  rbx: 0x00007fffffffdde8\n");
        output_verbose(CAT_REGISTERS, "  rcx: 0x0000000000000000  rdx: 0x0000000000000000\n");
        output_verbose(CAT_REGISTERS, "  rsp: 0x00007fffffffdde0  rbp: 0x00007fffffffddf0\n");
        output_verbose(CAT_REGISTERS, "  rsi: 0x0000000000000000  rdi: 0x0000000000000000\n");
        output_verbose(CAT_REGISTERS, "  r8:  0x0000000000000000  r9:  0x0000000000000000\n");
        output_verbose(CAT_REGISTERS, "  r10: 0x0000000000000000  r11: 0x0000000000000000\n");
        output_verbose(CAT_REGISTERS, "  r12: 0x0000000000000000  r13: 0x0000000000000000\n");
        output_verbose(CAT_REGISTERS, "  r14: 0x0000000000000000  r15: 0x0000000000000000\n");
        output_verbose(CAT_REGISTERS, "  rip: 0x0000000000401000  rflags: 0x0000000000000206\n");
    } else {
        /* Compact view: summary only */
        output_normal(CAT_REGISTERS, "  rax: 0x0000000000000000  rbx: 0x00007fffffffdde8\n");
        output_normal(CAT_REGISTERS, "  rsp: 0x00007fffffffdde0  rbp: 0x00007fffffffddf0\n");
        output_normal(CAT_REGISTERS, "  rip: 0x0000000000401000\n");
    }

    /* If filter is set, only show filtered registers */
    if (cfg->filter_regs) {
        output_normal(CAT_REGISTERS, "  (Filter: %s)\n", cfg->filter_regs);
    }
}

void registers_set_expand(expand_level_t level) {
    g_registers_config.expand = level;
}

void registers_toggle_expand(void) {
    g_registers_config.expand = (g_registers_config.expand + 1) % 3;
}

expand_level_t registers_get_expand(void) {
    return g_registers_config.expand;
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

static memory_config_t g_memory_config = {
    .expand = EXPAND_NONE,
    .address = NULL,
    .length = 0,
    .show_changes_only = 0
};

static expand_level_t g_memory_expand = EXPAND_NONE;

void memory_print(const memory_config_t *config) {
    if (!(g_visible_sections & SECTION_MEMORY)) {
        return;
    }

    const memory_config_t *cfg = config ? config : &g_memory_config;

    if (!cfg->address) {
        return;
    }

    /* Section header */
    char detail[64];
    snprintf(detail, sizeof(detail), "@ %p", cfg->address);
    section_print_header("MEMORY", g_memory_expand, detail);

    /* TODO: Implement real memory reading */
    /* For now, placeholder data */
    unsigned char *addr = (unsigned char *)cfg->address;
    size_t len = cfg->length > 0 ? cfg->length : 64;

    for (size_t i = 0; i < len; i += 16) {
        output_verbose(CAT_MEMORY, "  %p: ", addr + i);

        /* Hex bytes */
        for (size_t j = 0; j < 16 && i + j < len; j++) {
            output_verbose(CAT_MEMORY, "%02x ", 0); /* Placeholder */
        }

        /* ASCII */
        output_verbose(CAT_MEMORY, " |");
        for (size_t j = 0; j < 16 && i + j < len; j++) {
            output_verbose(CAT_MEMORY, "."); /* Placeholder */
        }
        output_verbose(CAT_MEMORY, "|\n");
    }
}

void memory_set_expand(expand_level_t level) {
    g_memory_expand = level;
}

void memory_toggle_expand(void) {
    g_memory_expand = (g_memory_expand + 1) % 3;
}

expand_level_t memory_get_expand(void) {
    return g_memory_expand;
}

/* === PROCESS INFO === */

void process_info_print(const debugger_state_t *state) {
    if (!state) return;
    if (!(g_visible_sections & SECTION_PROCESS)) {
        return;
    }

    /* Always show process info in NORMAL mode */
    output_process_info("%s  PID:%d  Offset:0x%lx",
                       state->proc.path, state->proc.pid, state->proc.offset);

    /* Additional info in VERBOSE mode */
    if (output_get_level() >= OUTPUT_VERBOSE) {
        output_verbose(CAT_PROCESS, "  GID:%d  Base:0x%lx\n",
                      state->proc.gid, state->proc.base);
    }
}

/* === SIGNAL INFO === */

void signal_info_print(const debugger_state_t *state) {
    if (!state || state->signal.signo == 0) {
        return;
    }
    if (!(g_visible_sections & SECTION_SIGNAL)) {
        return;
    }

    const char *color = theme_color(COLOR_RED);
    const char *bold = theme_style(STYLE_BOLD);
    const char *reset = theme_reset();

    if (theme_using_colors()) {
        printf("%s%s[SIG%s]%s\n",
               color, bold,
               state->signal.name ? state->signal.name : "UNKNOWN",
               reset);
    } else {
        printf("[SIG%s]\n",
               state->signal.name ? state->signal.name : "UNKNOWN");
    }
}

/* === BREAKPOINT INFO === */

void breakpoint_hit_print(uint64_t addr, const char *func_name) {
    if (!(g_visible_sections & SECTION_BACKTRACE)) {
        return;
    }

    const char *color = theme_color(COLOR_MAGENTA);
    const char *reset = theme_reset();

    if (theme_using_colors()) {
        printf("%s[BREAKPOINT]%s Hit at ", color, reset);
    } else {
        printf("[BREAKPOINT] Hit at ");
    }

    if (func_name) {
        printf("%s()\n", func_name);
    } else {
        printf("0x%lx\n", addr);
    }

    output_stats_inc_breakpoint();
}

/* === GLOBAL EXPANSION === */

void sections_set_global_expand(expand_level_t level) {
    g_global_expand = level;
    backtrace_set_expand(level);
    registers_set_expand(level);
    memory_set_expand(level);
}

expand_level_t sections_get_global_expand(void) {
    return g_global_expand;
}

/* Toggle specific section expansion */
void sections_toggle_expand(section_type_t section) {
    if (section & SECTION_BACKTRACE) {
        backtrace_toggle_expand();
    }
    if (section & SECTION_REGISTERS) {
        registers_toggle_expand();
    }
    if (section & SECTION_MEMORY) {
        memory_toggle_expand();
    }
}

void sections_toggle_all(void) {
    backtrace_toggle_expand();
    registers_toggle_expand();
    memory_toggle_expand();
}

/* Section visibility */
void sections_show(section_type_t sections) {
    g_visible_sections |= sections;
}

void sections_hide(section_type_t sections) {
    g_visible_sections &= ~sections;
}

int sections_is_visible(section_type_t section) {
    return (g_visible_sections & section) != 0;
}

/* === FORMATTING HELPERS === */

static const char *expand_indicator(expand_level_t level) {
    switch (level) {
        case EXPAND_NONE:  return "-";
        case EXPAND_NORMAL: return "+";
        case EXPAND_FULL:   return "*";
        default:           return "?";
    }
}

void section_print_header(const char *name, expand_level_t level, const char *detail) {
    const char *color = theme_color(theme_color_for_category(CAT_BACKTRACE));
    const char *reset = theme_reset();
    const char *indicator = expand_indicator(level);

    if (theme_using_colors()) {
        if (detail) {
            printf("%s[%s %s]%s ", color, indicator, name, reset);
            printf("%s\n", detail);
        } else {
            printf("%s[%s %s]%s\n", color, indicator, name, reset);
        }
    } else {
        if (detail) {
            printf("[%s %s] %s\n", indicator, name, detail);
        } else {
            printf("[%s %s]\n", indicator, name);
        }
    }
}

void section_print_separator(int length) {
    if (theme_using_colors()) {
        const char *color = theme_color(COLOR_BRIGHT_BLACK);
        printf("%s", color);
    }

    for (int i = 0; i < length; i++) {
        printf("-");
    }
    printf("\n");

    if (theme_using_colors()) {
        printf("%s", theme_reset());
    }
}
