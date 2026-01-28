/**
 * @file parser.h
 * @brief Command parsing (argtable3)
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

/* Stub for argtable3 types until it's installed */
#if !defined(HAVE_ARGTABLE3) || HAVE_ARGTABLE3 == 0
typedef void arg_end;
#endif

/* Command types */
typedef enum {
    CMD_CONTINUE,
    CMD_SINGLE_STEP,
    CMD_REGISTER_DUMP,
    CMD_REGISTER_READ,
    CMD_REGISTER_WRITE,
    CMD_BREAKPOINT_FUNC,
    CMD_BREAKPOINT_ADDR,
    CMD_MEMORY_READ,
    CMD_MEMORY_WRITE,
    CMD_BACKTRACE,
    CMD_SET_OUTPUT,
    CMD_SET_EXPAND,
    CMD_FILTER,
    CMD_HELP,
    CMD_QUIT,
    CMD_UNKNOWN
} command_type_t;

/* Parsed command */
typedef struct {
    command_type_t type;
    char *string_arg;      /* For function names, register names, etc. */
    uint64_t addr_arg;     /* For addresses */
    uint64_t value_arg;    /* For values */
    int int_arg;           /* For integers */
    char **list_arg;       /* For comma-separated lists */
    int list_count;
} command_t;

/* Initialize argtable (for command line args) */
int parser_init_args(int argc, char *argv[], arg_end *argtable);

/* Parse a single command string (for REPL) */
int parser_parse_command(const char *input, command_t *cmd);

/* Free command resources */
void parser_free_command(command_t *cmd);

/* Print usage */
void parser_print_usage(void);

#endif /* PARSER_H */
