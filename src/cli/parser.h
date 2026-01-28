/**
 * @file parser.h
 * @brief Command parsing
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
    CMD_STEP_OVER,          /* New: step over function calls */
    CMD_REGISTER_DUMP,
    CMD_REGISTER_READ,
    CMD_REGISTER_WRITE,
    CMD_BREAKPOINT_FUNC,
    CMD_BREAKPOINT_ADDR,
    CMD_BREAKPOINT_CURRENT, /* New: breakpoint at current RIP */
    CMD_BREAKPOINT_LIST,    /* New: list all breakpoints */
    CMD_BREAKPOINT_ENABLE,  /* New: enable a breakpoint */
    CMD_BREAKPOINT_DISABLE, /* New: disable a breakpoint */
    CMD_BREAKPOINT_DELETE,  /* New: delete a breakpoint */
    CMD_MEMORY_READ,
    CMD_MEMORY_WRITE,
    CMD_BACKTRACE,
    CMD_LIST_SOURCE,        /* New: list source code */
    CMD_INFO_FUNCTIONS,     /* New: list all functions */
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
int parser_init_args(int argc, char *argv[], void *argtable);

/* Parse a single command string (for REPL) */
int parser_parse_command(const char *input, command_t *cmd);

/* Free command resources */
void parser_free_command(command_t *cmd);

/* Get command name as string */
const char* command_type_name(command_type_t type);

/* Print usage/help */
void parser_print_usage(void);
void parser_print_command_help(command_type_t type);

#endif /* PARSER_H */
