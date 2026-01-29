/**
 * @file variables.c
 * @brief Variable lookup from DWARF debug information
 */

#include "variables.h"
#include "../display/output.h"
#include <dwarf.h>
#include <libdwarf/libdwarf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/user.h>

/* Variable info */
typedef struct {
    char name[256];
    char type[256];
    uint64_t location;     /* Frame base offset or address */
    int is_global;         /* 1 if global, 0 if local */
    int offset_from_fp;    /* Offset from frame pointer (for locals) */
} var_info_t;

/* Variables state */
typedef struct {
    int initialized;
    var_info_t *vars;
    int num_vars;
    int max_vars;
} variables_state_t;

#define MAX_VARS 1000

static variables_state_t g_var_state = {0};

/* Free a DIE and all its children */
static void free_die_chain(Dwarf_Debug dbg, Dwarf_Die die) {
    if (!die) return;

    Dwarf_Die child = 0;
    if (dwarf_child(die, &child, NULL) == DW_DLV_OK) {
        Dwarf_Die cur = child;
        while (cur) {
            Dwarf_Die sib = 0;
            if (dwarf_siblingof(dbg, cur, &sib, NULL) == DW_DLV_OK) {
                if (cur != child) dwarf_dealloc(dbg, cur, DW_DLA_DIE);
                cur = sib;
            } else {
                cur = 0;
            }
        }
        if (child) dwarf_dealloc(dbg, child, DW_DLA_DIE);
    }
    dwarf_dealloc(dbg, die, DW_DLA_DIE);
}

int variables_load(const char *program_path) {
    memset(&g_var_state, 0, sizeof(g_var_state));

    g_var_state.vars = calloc(MAX_VARS, sizeof(var_info_t));
    if (!g_var_state.vars) {
        return -1;
    }
    g_var_state.max_vars = MAX_VARS;

    /* Open file */
    int fd = open(program_path, O_RDONLY);
    if (fd < 0) {
        output_error("Failed to open %s", program_path);
        free(g_var_state.vars);
        return -1;
    }

    /* Initialize libdwarf */
    Dwarf_Debug dbg = 0;
    Dwarf_Error error = 0;

    if (dwarf_init(fd, DW_DLC_READ, 0, 0, &dbg, &error) != DW_DLV_OK) {
        output_error("No DWARF info (compile with -g)");
        close(fd);
        free(g_var_state.vars);
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

        /* Walk through all DIEs to find variables */
        int in_subprogram = 0;
        char current_func[256] = {0};

        /* First, get the subprogram (function) info */
        Dwarf_Die child = 0;
        if (dwarf_child(cu_die, &child, &error) == DW_DLV_OK) {
            Dwarf_Die cur = child;
            while (cur && g_var_state.num_vars < MAX_VARS) {
                Dwarf_Half tag;
                if (dwarf_tag(cur, &tag, &error) == DW_DLV_OK) {
                    if (tag == DW_TAG_subprogram) {
                        /* Get function name */
                        char *name = 0;
                        if (dwarf_diename(cur, &name, &error) == DW_DLV_OK && name) {
                            strncpy(current_func, name, 255);
                            in_subprogram = 1;
                            dwarf_dealloc(dbg, name, DW_DLA_STRING);

                            /* Look for variables in this function */
                            Dwarf_Die var_child = 0;
                            if (dwarf_child(cur, &var_child, &error) == DW_DLV_OK) {
                                Dwarf_Die var_cur = var_child;
                                while (var_cur && g_var_state.num_vars < MAX_VARS) {
                                    Dwarf_Half var_tag;
                                    if (dwarf_tag(var_cur, &var_tag, &error) == DW_DLV_OK) {
                                        if (var_tag == DW_TAG_variable || var_tag == DW_TAG_formal_parameter) {
                                            char *var_name = 0;
                                            if (dwarf_diename(var_cur, &var_name, &error) == DW_DLV_OK && var_name) {
                                                strncpy(g_var_state.vars[g_var_state.num_vars].name, var_name, 255);
                                                strncpy(g_var_state.vars[g_var_state.num_vars].type, "int", 255); /* Default */
                                                g_var_state.vars[g_var_state.num_vars].is_global = 0;
                                                g_var_state.vars[g_var_state.num_vars].offset_from_fp = 0;
                                                dwarf_dealloc(dbg, var_name, DW_DLA_STRING);

                                                /* Try to get location info */
                                                Dwarf_Attribute loc_attr = 0;
                                                if (dwarf_attr(var_cur, DW_AT_location, &loc_attr, &error) == DW_DLV_OK) {
                                                    Dwarf_Locdesc *locbuf = 0;
                                                    Dwarf_Signed listlen = 0;
                                                    if (dwarf_loclist(loc_attr, &locbuf, &listlen, &error) == DW_DLV_OK) {
                                                        if (listlen > 0 && locbuf != NULL && locbuf[0].ld_cents > 0) {
                                                            Dwarf_Loc *loc = &locbuf[0].ld_s[0];
                                                            if (loc->lr_atom == DW_OP_fbreg) {
                                                                g_var_state.vars[g_var_state.num_vars].offset_from_fp = loc->lr_number;
                                                            }
                                                        }
                                                        dwarf_dealloc(dbg, locbuf, DW_DLA_LOCDESC);
                                                    }
                                                    dwarf_dealloc(dbg, loc_attr, DW_DLA_ATTR);
                                                }

                                                g_var_state.num_vars++;
                                            }
                                        }
                                    }
                                    Dwarf_Die var_sib = 0;
                                    if (dwarf_siblingof(dbg, var_cur, &var_sib, &error) == DW_DLV_OK) {
                                        if (var_cur != var_child) dwarf_dealloc(dbg, var_cur, DW_DLA_DIE);
                                        var_cur = var_sib;
                                    } else {
                                        var_cur = 0;
                                    }
                                }
                                if (var_child) dwarf_dealloc(dbg, var_child, DW_DLA_DIE);
                            }
                        }
                    } else if (tag == DW_TAG_variable && !in_subprogram) {
                        /* Global variable */
                        char *name = 0;
                        if (dwarf_diename(cur, &name, &error) == DW_DLV_OK && name) {
                            strncpy(g_var_state.vars[g_var_state.num_vars].name, name, 255);
                            strncpy(g_var_state.vars[g_var_state.num_vars].type, "int", 255);
                            g_var_state.vars[g_var_state.num_vars].is_global = 1;
                            g_var_state.vars[g_var_state.num_vars].offset_from_fp = 0;
                            dwarf_dealloc(dbg, name, DW_DLA_STRING);

                            /* Try to get location (address) */
                            Dwarf_Attribute loc_attr = 0;
                            if (dwarf_attr(cur, DW_AT_location, &loc_attr, &error) == DW_DLV_OK) {
                                Dwarf_Locdesc *locbuf = 0;
                                Dwarf_Signed listlen = 0;
                                if (dwarf_loclist(loc_attr, &locbuf, &listlen, &error) == DW_DLV_OK) {
                                    if (listlen > 0 && locbuf[0].ld_cents > 0) {
                                        Dwarf_Loc *loc = &locbuf[0].ld_s[0];
                                        if (loc->lr_atom == DW_OP_addr) {
                                            g_var_state.vars[g_var_state.num_vars].location = loc->lr_number;
                                        }
                                    }
                                    dwarf_dealloc(dbg, locbuf, DW_DLA_LOCDESC);
                                }
                                dwarf_dealloc(dbg, loc_attr, DW_DLA_ATTR);
                            }

                            g_var_state.num_vars++;
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

    g_var_state.initialized = 1;
    return 0;
}

void variables_free(void) {
    if (g_var_state.vars) {
        free(g_var_state.vars);
        g_var_state.vars = NULL;
    }
    g_var_state.num_vars = 0;
    g_var_state.initialized = 0;
}

static int count_asterisks(const char *expr) {
    int count = 0;
    while (*expr == '*') {
        count++;
        expr++;
    }
    return count;
}

static int is_pointer_expression(const char *expr) {
    return (expr[0] == '*');
}

/* Simple expression evaluation */
/* Supports: var, *ptr, **ptr, ptr[i] */
/* For now, we'll implement a basic version */
int variables_eval_expression(pid_t pid, const char *expr, uint64_t *result, int *deref_count) {
    if (!expr || !result) return -1;

    *deref_count = 0;

    /* Count leading asterisks (dereference count) */
    const char *expr_start = expr;
    while (*expr_start == '*') {
        (*deref_count)++;
        expr_start++;
    }

    /* Skip whitespace */
    while (*expr_start == ' ' || *expr_start == '\t') {
        expr_start++;
    }

    /* Check for parentheses in pointer arithmetic like *(ptr+1) */
    if (expr_start[0] == '(' && *deref_count > 0) {
        /* Find matching closing paren */
        const char *paren_end = strchr(expr_start + 1, ')');
        if (paren_end) {
            /* Extract the inner expression (e.g., ptr+1) */
            char inner_expr[256];
            size_t len = paren_end - (expr_start + 1);
            if (len < sizeof(inner_expr) - 1) {
                strncpy(inner_expr, expr_start + 1, len);
                inner_expr[len] = '\0';

                /* Check for pointer arithmetic (ptr+1, ptr-1, etc) */
                char *plus = strchr(inner_expr, '+');
                char *minus = strchr(inner_expr, '-');

                if (plus || minus) {
                    /* Split variable name and offset */
                    char *var_name = inner_expr;
                    char *op = plus ? plus : minus;
                    *op = '\0';
                    int offset = atoi(op + 1);
                    if (minus) offset = -offset;

                    /* Look up variable */
                    const var_info_t *var = NULL;
                    for (int i = 0; i < g_var_state.num_vars; i++) {
                        if (strcmp(g_var_state.vars[i].name, var_name) == 0) {
                            var = &g_var_state.vars[i];
                            break;
                        }
                    }

                    if (var && var->is_global) {
                        /* Global variable - use its address */
                        uint64_t addr = var->location;
                        *result = addr + (offset * 8); /* Assume 8-byte pointers */
                        return 0;
                    }

                    /* For local variables, we'd need frame pointer - skip for now */
                    return -1;
                }
            }
        }
    }

    /* Simple variable name or *ptr */
    char var_name[256];
    strncpy(var_name, expr_start, sizeof(var_name) - 1);
    var_name[sizeof(var_name) - 1] = '\0';

    /* Remove any trailing whitespace or garbage */
    char *end = var_name + strlen(var_name) - 1;
    while (end > var_name && (*end == ' ' || *end == '\t' || *end == '\n')) {
        *end = '\0';
        end--;
    }

    /* Look up variable */
    const var_info_t *var = NULL;
    for (int i = 0; i < g_var_state.num_vars; i++) {
        if (strcmp(g_var_state.vars[i].name, var_name) == 0) {
            var = &g_var_state.vars[i];
            break;
        }
    }

    if (!var) {
        return -1; /* Variable not found */
    }

    if (var->is_global) {
        /* Global variable - use its address */
        *result = var->location;
        return 0;
    } else {
        /* Local variable - need to get from stack */
        /* For simplicity, we'll try reading from RBP+offset */
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
            return -1;
        }

        /* Get address from frame pointer + offset */
        *result = regs.rbp + var->offset_from_fp;
        return 0;
    }
}

/* Get the address of a variable expression */
int variables_get_address(pid_t pid, const char *expr, uint64_t *addr) {
    if (!expr || !addr) return -1;

    int deref_count = 0;
    uint64_t base_addr = 0;

    if (variables_eval_expression(pid, expr, &base_addr, &deref_count) < 0) {
        return -1;
    }

    /* Apply dereferences */
    *addr = base_addr;
    for (int i = 0; i < deref_count; i++) {
        /* Read pointer value */
        errno = 0;
        long ptr_val = ptrace(PTRACE_PEEKDATA, pid, (void *)*addr, NULL);
        if (errno != 0) {
            return -1;
        }
        *addr = (uint64_t)ptr_val;
    }

    return 0;
}

/* Read a value from target process memory */
int variables_read_value(pid_t pid, uint64_t addr, void *buffer, size_t size) {
    if (!buffer || size == 0 || size > 8) {
        return -1;
    }

    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, pid, (void *)addr, NULL);

    if (errno != 0) {
        return -1;
    }

    /* Copy the requested bytes */
    memcpy(buffer, &data, size);
    return 0;
}

/* Format and print a value */
void variables_print_value(uint64_t value, const char *type_hint) {
    /* For now, just print as hex and decimal */
    output_normal(CAT_PROCESS, "  = 0x%016lx (%ld)\n", value, (int64_t)value);
}
