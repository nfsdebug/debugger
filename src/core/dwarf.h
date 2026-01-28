/**
 * @file dwarf.h
 * @brief DWARF debug information parsing
 */

#ifndef DWARF_H
#define DWARF_H

#include <stdint.h>

/* Source location */
typedef struct {
    char file[256];
    int line;
    uint64_t address;
} source_location_t;

/* Function info */
typedef struct {
    char name[256];
    uint64_t low_pc;
    uint64_t high_pc;
} function_info_t;

/* DWARF state */
typedef struct {
    int initialized;
    char *program_path;
    source_location_t *lines;
    int num_lines;
    function_info_t *functions;
    int num_functions;
} dwarf_state_t;

/* Load DWARF info from program */
int dwarf_load(dwarf_state_t *state, const char *program_path);

/* Free DWARF state */
void dwarf_free(dwarf_state_t *state);

/* Find address for source location (file:line) */
uint64_t dwarf_line_to_addr(dwarf_state_t *state, const char *file, int line);

/* Find source location for address */
const source_location_t* dwarf_addr_to_line(dwarf_state_t *state, uint64_t addr);

/* Get function containing address */
const function_info_t* dwarf_addr_to_function(dwarf_state_t *state, uint64_t addr);

/* List source code */
void dwarf_list_source(dwarf_state_t *state, const char *file, int start_line, int count);

#endif
