/**
 * @file parser.c
 * @brief Command parsing (argtable3)
 */

#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/* arg_end will be provided by argtable3 when installed */
/* For now, we use a minimal stub */
#ifndef HAVE_ARGTABLE3
#define HAVE_ARGTABLE3 0
#endif

int parser_init_args(int argc, char *argv[], void *argtable) {
    /* Define command line arguments */
    /* TODO: Implement with argtable3 */
    (void)argc;
    (void)argv;
    (void)argtable;
    return 0;
}

int parser_parse_command(const char *input, command_t *cmd) {
    if (!input || !cmd) {
        return -1;
    }

    memset(cmd, 0, sizeof(*cmd));

    /* Copy input for tokenization */
    char *copy = strdup(input);
    if (!copy) {
        return -1;
    }

    /* Get first token (command) */
    char *token = strtok(copy, " \t\n");
    if (!token) {
        free(copy);
        return -1;
    }

    /* Convert to lowercase */
    for (char *p = token; *p; p++) {
        *p = tolower(*p);
    }

    /* Parse command */
    if (strcmp(token, "continue") == 0 || strcmp(token, "c") == 0) {
        cmd->type = CMD_CONTINUE;
    } else if (strcmp(token, "step") == 0 || strcmp(token, "s") == 0) {
        cmd->type = CMD_SINGLE_STEP;
    } else if (strcmp(token, "register") == 0 || strcmp(token, "r") == 0) {
        char *subtoken = strtok(NULL, " \t\n");
        if (!subtoken) {
            cmd->type = CMD_REGISTER_DUMP;
        } else if (strcmp(subtoken, "dump") == 0) {
            cmd->type = CMD_REGISTER_DUMP;
        } else if (strcmp(subtoken, "read") == 0) {
            cmd->type = CMD_REGISTER_READ;
            cmd->string_arg = strdup(strtok(NULL, " \t\n"));
        } else if (strcmp(subtoken, "write") == 0) {
            cmd->type = CMD_REGISTER_WRITE;
            char *reg = strtok(NULL, " \t\n");
            char *val = strtok(NULL, " \t\n");
            if (reg) {
                cmd->string_arg = strdup(reg);
                if (val) {
                    cmd->value_arg = strtoll(val, NULL, 0);
                }
            }
        }
    } else if (strcmp(token, "memory") == 0 || strcmp(token, "m") == 0) {
        char *subtoken = strtok(NULL, " \t\n");
        if (subtoken && strcmp(subtoken, "read") == 0) {
            cmd->type = CMD_MEMORY_READ;
            char *addr = strtok(NULL, " \t\n");
            if (addr) {
                cmd->addr_arg = strtoll(addr, NULL, 0);
            }
        } else if (subtoken && strcmp(subtoken, "write") == 0) {
            cmd->type = CMD_MEMORY_WRITE;
            char *addr = strtok(NULL, " \t\n");
            char *val = strtok(NULL, " \t\n");
            if (addr && val) {
                cmd->addr_arg = strtoll(addr, NULL, 0);
                cmd->value_arg = strtoll(val, NULL, 0);
            }
        }
    } else if (strcmp(token, "backtrace") == 0 || strcmp(token, "bt") == 0) {
        cmd->type = CMD_BACKTRACE;
    } else if (strcmp(token, "b") == 0 || strcmp(token, "break") == 0) {
        char *subtoken = strtok(NULL, " \t\n");
        if (subtoken && strcmp(subtoken, "func") == 0) {
            cmd->type = CMD_BREAKPOINT_FUNC;
            cmd->string_arg = strdup(strtok(NULL, " \t\n"));
        } else {
            cmd->type = CMD_BREAKPOINT_ADDR;
            if (subtoken) {
                cmd->addr_arg = strtoll(subtoken, NULL, 0);
            }
        }
    } else if (strcmp(token, "quit") == 0 || strcmp(token, "q") == 0) {
        cmd->type = CMD_QUIT;
    } else if (strcmp(token, "help") == 0 || strcmp(token, "h") == 0) {
        cmd->type = CMD_HELP;
    } else {
        cmd->type = CMD_UNKNOWN;
    }

    free(copy);
    return 0;
}

void parser_free_command(command_t *cmd) {
    if (cmd) {
        free(cmd->string_arg);
        free(cmd->list_arg);
        memset(cmd, 0, sizeof(*cmd));
    }
}

void parser_print_usage(void) {
    printf("Commands:\n");
    printf("  continue, c           Continue execution\n");
    printf("  step, s               Single step\n");
    printf("  register dump         Dump all registers\n");
    printf("  register read <reg>   Read a register\n");
    printf("  register write <r> <v> Write a register\n");
    printf("  memory read <addr>    Read memory\n");
    printf("  memory write <a> <v>  Write memory\n");
    printf("  backtrace, bt         Show backtrace\n");
    printf("  b func <name>         Breakpoint by function\n");
    printf("  b <addr>              Breakpoint by address\n");
    printf("  quit, q               Quit\n");
    printf("  help, h               Show this help\n");
}
