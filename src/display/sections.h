/**
 * @file sections.h
 * @brief Modular output sections (backtrace, registers, memory)
 */

#ifndef SECTIONS_H
#define SECTIONS_H

#include <stdint.h>
#include <sys/types.h>

/* Include debugger.h for the actual types */
#include "../core/debugger.h"

/* Section expansion state */
typedef enum {
    EXPAND_NONE = 0,      /* Show summary only */
    EXPAND_NORMAL,        /* Show details */
    EXPAND_FULL           /* Show everything */
} expand_level_t;

/* Section type for filtering */
typedef enum {
    SECTION_BACKTRACE = (1 << 0),
    SECTION_REGISTERS = (1 << 1),
    SECTION_MEMORY    = (1 << 2),
    SECTION_PROCESS   = (1 << 3),
    SECTION_SIGNAL    = (1 << 4),
    SECTION_ALL       = 0xFF
} section_type_t;

/* === BACKTRACE SECTION === */

typedef struct {
    int frame_count;
    expand_level_t expand;
    int show_addresses;
    int show_locals;
} backtrace_config_t;

void backtrace_print(const debugger_state_t *state, const backtrace_config_t *config);
void backtrace_set_expand(expand_level_t level);
void backtrace_toggle_expand(void);
expand_level_t backtrace_get_expand(void);

/* === REGISTERS SECTION === */

typedef struct {
    expand_level_t expand;
    int groups;           /* 1 = compact (3 per line), 2 = detailed */
    char *filter_regs;    /* NULL = all, or comma-separated list */
} registers_config_t;

void registers_print(const registers_config_t *config);
void registers_set_expand(expand_level_t level);
void registers_toggle_expand(void);
expand_level_t registers_get_expand(void);
void registers_set_filter(const char *regs);

/* === MEMORY SECTION === */

typedef struct {
    expand_level_t expand;
    void *address;
    size_t length;
    int show_changes_only;  /* Only show modified bytes */
} memory_config_t;

void memory_print(const memory_config_t *config);
void memory_set_expand(expand_level_t level);
void memory_toggle_expand(void);
expand_level_t memory_get_expand(void);

/* === PROCESS INFO SECTION === */

void process_info_print(const debugger_state_t *state);

/* === SIGNAL INFO SECTION === */

void signal_info_print(const debugger_state_t *state);

/* === BREAKPOINT INFO SECTION === */

void breakpoint_hit_print(uint64_t addr, const char *func_name);

/* === GLOBAL EXPANSION === */

void sections_set_global_expand(expand_level_t level);
expand_level_t sections_get_global_expand(void);

/* Toggle specific section expansion */
void sections_toggle_expand(section_type_t section);
void sections_toggle_all(void);

/* Section visibility */
void sections_show(section_type_t sections);
void sections_hide(section_type_t sections);
int sections_is_visible(section_type_t section);

/* === FORMATTING HELPERS === */

/* Print a section header with expand/collapse indicator */
void section_print_header(const char *name, expand_level_t level, const char *detail);

/* Print a separator line */
void section_print_separator(int length);

#endif /* SECTIONS_H */
