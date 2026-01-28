/**
 * @file dwarf.c
 * @brief DWARF parsing using libdwarf
 */

#include "dwarf.h"
#include "../display/output.h"
#include "../display/sections.h"
#include <dwarf.h>
#include <libdwarf/libdwarf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LINES 10000
#define MAX_FUNCS 1000

int dwarf_load(dwarf_state_t *state, const char *program_path) {
    memset(state, 0, sizeof(*state));

    state->program_path = strdup(program_path);
    state->lines = calloc(MAX_LINES, sizeof(source_location_t));
    state->functions = calloc(MAX_FUNCS, sizeof(function_info_t));

    if (!state->lines || !state->functions) {
        dwarf_free(state);
        return -1;
    }

    /* Open file */
    int fd = open(program_path, O_RDONLY);
    if (fd < 0) {
        output_error("Failed to open %s", program_path);
        dwarf_free(state);
        return -1;
    }

    /* Initialize libdwarf */
    Dwarf_Debug dbg = 0;
    Dwarf_Error error = 0;

    if (dwarf_init(fd, DW_DLC_READ, 0, 0, &dbg, &error) != DW_DLV_OK) {
        output_error("No DWARF info (compile with -g)");
        close(fd);
        dwarf_free(state);
        return -1;
    }

    /* Walk CUs */
    Dwarf_Unsigned cu_header_length, next_cu_header_offset;
    Dwarf_Half version_stamp, address_size;
    Dwarf_Off abbr_offset;

    while (dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp,
                                 &abbr_offset, &address_size,
                                 &next_cu_header_offset, &error) == DW_DLV_OK) {

        Dwarf_Die cu_die = 0;
        if (dwarf_siblingof(dbg, 0, &cu_die, &error) != DW_DLV_OK) {
            continue;
        }

        /* Get line info */
        Dwarf_Line *line_buf = 0;
        Dwarf_Signed line_count = 0;

        if (dwarf_srclines(cu_die, &line_buf, &line_count, &error) == DW_DLV_OK) {
            for (int i = 0; i < line_count && state->num_lines < MAX_LINES; i++) {
                Dwarf_Addr addr = 0;
                char *srcfile = 0;
                Dwarf_Unsigned lineno = 0;

                if (dwarf_lineaddr(line_buf[i], &addr, &error) == DW_DLV_OK &&
                    dwarf_linesrc(line_buf[i], &srcfile, &error) == DW_DLV_OK &&
                    dwarf_lineno(line_buf[i], &lineno, &error) == DW_DLV_OK) {

                    strncpy(state->lines[state->num_lines].file, srcfile, 255);
                    state->lines[state->num_lines].file[255] = '\0';
                    state->lines[state->num_lines].line = (int)lineno;
                    state->lines[state->num_lines].address = (uint64_t)addr;
                    state->num_lines++;
                }

                if (srcfile) dwarf_dealloc(dbg, srcfile, DW_DLA_STRING);
            }
            dwarf_srclines_dealloc(dbg, line_buf, line_count);
        }

        /* Get functions */
        Dwarf_Die child = 0;
        if (dwarf_child(cu_die, &child, &error) == DW_DLV_OK) {
            Dwarf_Die cur = child;
            while (cur && state->num_functions < MAX_FUNCS) {
                Dwarf_Half tag;
                if (dwarf_tag(cur, &tag, &error) == DW_DLV_OK && tag == DW_TAG_subprogram) {
                    char *name = 0;
                    if (dwarf_diename(cur, &name, &error) == DW_DLV_OK && name) {
                        strncpy(state->functions[state->num_functions].name, name, 255);
                        dwarf_dealloc(dbg, name, DW_DLA_STRING);

                        Dwarf_Addr low_pc = 0, high_pc = 0;
                        if (dwarf_lowpc(cur, &low_pc, &error) == DW_DLV_OK &&
                            dwarf_highpc(cur, &high_pc, &error) == DW_DLV_OK) {

                            state->functions[state->num_functions].low_pc = (uint64_t)low_pc;
                            state->functions[state->num_functions].high_pc = (uint64_t)high_pc;
                            state->num_functions++;
                        }
                    }
                }

                Dwarf_Die sib = 0;
                if (dwarf_siblingof(dbg, cur, &sib, &error) == DW_DLV_OK) {
                    if (cur != child) dwarf_dealloc(dbg, cur, DW_DLA_DIE);
                    cur = sib;
                } else {
                    cur = 0;
                }
            }
            if (child) dwarf_dealloc(dbg, child, DW_DLA_DIE);
        }

        if (cu_die) dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
    }

    dwarf_finish(dbg, &error);
    close(fd);

    state->initialized = 1;
    output_normal(CAT_PROCESS, "DWARF: %d lines, %d functions\n", state->num_lines, state->num_functions);
    return 0;
}

void dwarf_free(dwarf_state_t *state) {
    if (state) {
        free(state->lines);
        free(state->functions);
        free(state->program_path);
        state->lines = NULL;
        state->functions = NULL;
        state->program_path = NULL;
        state->num_lines = 0;
        state->num_functions = 0;
        state->initialized = 0;
    }
}

uint64_t dwarf_line_to_addr(dwarf_state_t *state, const char *file, int line) {
    if (!state || !state->initialized) return 0;

    /* Try exact match */
    for (int i = 0; i < state->num_lines; i++) {
        if (state->lines[i].line == line && strstr(state->lines[i].file, file)) {
            return state->lines[i].address;
        }
    }

    /* Try basename match */
    const char *basename = strrchr(file, '/');
    if (basename) basename++;

    for (int i = 0; i < state->num_lines; i++) {
        if (state->lines[i].line == line) {
            const char *line_base = strrchr(state->lines[i].file, '/');
            if (line_base) line_base++;
            else line_base = state->lines[i].file;

            if (strcmp(line_base, basename) == 0) {
                return state->lines[i].address;
            }
        }
    }

    return 0;
}

const source_location_t* dwarf_addr_to_line(dwarf_state_t *state, uint64_t addr) {
    if (!state || !state->initialized) return NULL;

    const source_location_t *closest = NULL;
    uint64_t closest_dist = UINT64_MAX;

    for (int i = 0; i < state->num_lines; i++) {
        if (state->lines[i].address <= addr) {
            uint64_t dist = addr - state->lines[i].address;
            if (dist < closest_dist) {
                closest_dist = dist;
                closest = &state->lines[i];
            }
        }
    }

    return closest;
}

const function_info_t* dwarf_addr_to_function(dwarf_state_t *state, uint64_t addr) {
    if (!state || !state->initialized) return NULL;

    for (int i = 0; i < state->num_functions; i++) {
        if (addr >= state->functions[i].low_pc && addr < state->functions[i].high_pc) {
            return &state->functions[i];
        }
    }
    return NULL;
}

void dwarf_list_source(dwarf_state_t *state, const char *file, int start_line, int count) {
    if (!state || !state->initialized) {
        output_error("No DWARF info available");
        return;
    }

    /* Find full file path */
    char found_file[256] = {0};
    for (int i = 0; i < state->num_lines; i++) {
        if (strstr(state->lines[i].file, file)) {
            strncpy(found_file, state->lines[i].file, 255);
            break;
        }
    }

    if (found_file[0] == '\0') {
        output_error("File not found in debug info: %s", file);
        return;
    }

    FILE *fp = fopen(found_file, "r");
    if (!fp) {
        output_error("Cannot open: %s", found_file);
        return;
    }

    section_print_header("SOURCE", sections_get_global_expand(), NULL);
    output_normal(CAT_PROCESS, "  File: %s\n\n", found_file);

    char line[512];
    int current = 1, printed = 0;

    while (fgets(line, sizeof(line), fp) && printed < count) {
        if (current >= start_line) {
            size_t len = strlen(line);
            if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
            output_normal(CAT_PROCESS, "  %5d  %s\n", current, line);
            printed++;
        }
        current++;
    }

    fclose(fp);
}
