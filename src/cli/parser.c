/**
 * @file parser.c
 * @brief Command parsing
 */

#include "parser.h"
#include "../display/output.h"
#include "../display/theme.h"
#include "../display/sections.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/* Command name lookup table */
static const struct {
    const char *name;
    command_type_t type;
} command_table[] = {
    /* Full names */
    {"continue", CMD_CONTINUE},
    {"step", CMD_SINGLE_STEP},
    {"next", CMD_SINGLE_STEP},  /* Alias for step */
    {"register", CMD_REGISTER_DUMP},
    {"memory", CMD_MEMORY_READ},
    {"backtrace", CMD_BACKTRACE},
    {"breakpoint", CMD_BREAKPOINT_ADDR},
    {"break", CMD_BREAKPOINT_ADDR},
    {"quit", CMD_QUIT},
    {"exit", CMD_QUIT},
    {"help", CMD_HELP},
    {"set", CMD_SET_OUTPUT},
    {"filter", CMD_FILTER},
    /* Short aliases */
    {"c", CMD_CONTINUE},
    {"s", CMD_SINGLE_STEP},
    {"n", CMD_SINGLE_STEP},
    {"r", CMD_REGISTER_DUMP},
    {"m", CMD_MEMORY_READ},
    {"bt", CMD_BACKTRACE},
    {"b", CMD_BREAKPOINT_ADDR},
    {"q", CMD_QUIT},
    {"h", CMD_HELP},
    {NULL, CMD_UNKNOWN}
};

static command_type_t lookup_command(const char *token) {
    for (int i = 0; command_table[i].name != NULL; i++) {
        if (strcmp(token, command_table[i].name) == 0) {
            return command_table[i].type;
        }
    }
    return CMD_UNKNOWN;
}

int parser_init_args(int argc, char *argv[], void *argtable) {
    /* Define command line arguments */
    /* TODO: Implement with argtable3 */
    (void)argc;
    (void)argv;
    (void)argtable;
    return 0;
}

static char* tolower_str(const char *str) {
    if (!str) return NULL;
    char *result = strdup(str);
    if (!result) return NULL;

    for (char *p = result; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
    return result;
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

    /* Convert to lowercase for lookup */
    char *token_lower = tolower_str(token);
    if (!token_lower) {
        free(copy);
        return -1;
    }

    /* Look up command type */
    cmd->type = lookup_command(token_lower);
    free(token_lower);

    /* Special handling for multi-word commands */
    if (cmd->type == CMD_REGISTER_DUMP) {
        /* Check for "register read <reg>" or "register dump" */
        char *subcmd = tolower_str(strtok(NULL, " \t\n"));
        if (subcmd && strcmp(subcmd, "read") == 0) {
            cmd->type = CMD_REGISTER_READ;
            free(subcmd);
            cmd->string_arg = tolower_str(strtok(NULL, " \t\n"));
            free(copy);
            return 0;
        } else if (subcmd && strcmp(subcmd, "write") == 0) {
            cmd->type = CMD_REGISTER_WRITE;
            char *reg = tolower_str(strtok(NULL, " \t\n"));
            char *val = strtok(NULL, " \t\n");
            if (reg) {
                cmd->string_arg = reg;
                if (val) {
                    cmd->value_arg = strtoll(val, NULL, 0);
                }
            }
            free(subcmd);
            free(copy);
            return 0;
        }
        free(subcmd);
        /* "register dump" or just "register" */
    }

    /* Special handling for memory read/write */
    if (cmd->type == CMD_MEMORY_READ) {
        char *subcmd = tolower_str(strtok(NULL, " \t\n"));
        if (subcmd && strcmp(subcmd, "write") == 0) {
            cmd->type = CMD_MEMORY_WRITE;
            char *addr = strtok(NULL, " \t\n");
            char *val = strtok(NULL, " \t\n");
            if (addr && val) {
                cmd->addr_arg = strtoll(addr, NULL, 0);
                cmd->value_arg = strtoll(val, NULL, 0);
            }
            free(subcmd);
            free(copy);
            return 0;
        } else if (subcmd && strcmp(subcmd, "read") == 0) {
            /* "memory read <addr>" - already CMD_MEMORY_READ */
            char *addr = strtok(NULL, " \t\n");
            if (addr) {
                cmd->addr_arg = strtoll(addr, NULL, 0);
            }
            free(subcmd);
            free(copy);
            return 0;
        }
        /* "memory <addr>" - just read */
        free(subcmd);
    }

    /* Parse command-specific arguments */
    switch (cmd->type) {
        case CMD_CONTINUE:
        case CMD_SINGLE_STEP:
        case CMD_REGISTER_DUMP:
        case CMD_BACKTRACE:
        case CMD_QUIT:
        case CMD_HELP:
            /* No arguments needed */
            break;

        case CMD_REGISTER_READ:
            cmd->string_arg = tolower_str(strtok(NULL, " \t\n"));
            break;

        case CMD_REGISTER_WRITE: {
            char *reg = tolower_str(strtok(NULL, " \t\n"));
            char *val = strtok(NULL, " \t\n");
            if (reg) {
                cmd->string_arg = reg;
                if (val) {
                    cmd->value_arg = strtoll(val, NULL, 0);
                }
            }
            break;
        }

        case CMD_MEMORY_READ: {
            char *addr = strtok(NULL, " \t\n");
            if (addr) {
                cmd->addr_arg = strtoll(addr, NULL, 0);
            }
            break;
        }

        case CMD_MEMORY_WRITE: {
            char *addr = strtok(NULL, " \t\n");
            char *val = strtok(NULL, " \t\n");
            if (addr && val) {
                cmd->addr_arg = strtoll(addr, NULL, 0);
                cmd->value_arg = strtoll(val, NULL, 0);
            }
            break;
        }

        case CMD_BREAKPOINT_FUNC:
            cmd->string_arg = strdup(strtok(NULL, " \t\n"));
            break;

        case CMD_BREAKPOINT_ADDR: {
            char *addr = strtok(NULL, " \t\n");
            if (addr) {
                cmd->addr_arg = strtoll(addr, NULL, 0);
            }
            break;
        }

        case CMD_SET_OUTPUT: {
            char *subtoken = tolower_str(strtok(NULL, " \t\n"));
            if (subtoken) {
                if (strcmp(subtoken, "output") == 0) {
                    char *level = tolower_str(strtok(NULL, " \t\n"));
                    if (level) {
                        if (strcmp(level, "quiet") == 0) {
                            cmd->int_arg = OUTPUT_QUIET;
                        } else if (strcmp(level, "normal") == 0) {
                            cmd->int_arg = OUTPUT_NORMAL;
                        } else if (strcmp(level, "verbose") == 0) {
                            cmd->int_arg = OUTPUT_VERBOSE;
                        } else if (strcmp(level, "debug") == 0) {
                            cmd->int_arg = OUTPUT_DEBUG;
                        }
                        free(level);
                    }
                } else if (strcmp(subtoken, "expand") == 0) {
                    char *level = tolower_str(strtok(NULL, " \t\n"));
                    if (level) {
                        if (strcmp(level, "none") == 0) {
                            cmd->int_arg = EXPAND_NONE;
                        } else if (strcmp(level, "normal") == 0) {
                            cmd->int_arg = EXPAND_NORMAL;
                        } else if (strcmp(level, "full") == 0) {
                            cmd->int_arg = EXPAND_FULL;
                        }
                        free(level);
                    }
                }
                free(subtoken);
            }
            break;
        }

        case CMD_SET_EXPAND: {
            char *level = tolower_str(strtok(NULL, " \t\n"));
            if (level) {
                if (strcmp(level, "none") == 0) {
                    cmd->int_arg = EXPAND_NONE;
                } else if (strcmp(level, "normal") == 0) {
                    cmd->int_arg = EXPAND_NORMAL;
                } else if (strcmp(level, "full") == 0) {
                    cmd->int_arg = EXPAND_FULL;
                }
                free(level);
            }
            break;
        }

        case CMD_FILTER: {
            /* Parse comma-separated filter list */
            char *filter_str = strtok(NULL, " \t\n");
            if (filter_str) {
                /* Count commas to determine number of filters */
                int count = 1;
                for (char *p = filter_str; *p; p++) {
                    if (*p == ',') count++;
                }

                cmd->list_count = count;
                cmd->list_arg = calloc(count, sizeof(char*));

                /* Split by comma */
                int i = 0;
                char *saveptr = NULL;
                char *token = strtok_r(filter_str, ",", &saveptr);
                while (token && i < count) {
                    cmd->list_arg[i++] = strdup(token);
                    token = strtok_r(NULL, ",", &saveptr);
                }
            }
            break;
        }

        default:
            break;
    }

    free(copy);
    return 0;
}

void parser_free_command(command_t *cmd) {
    if (cmd) {
        free(cmd->string_arg);
        if (cmd->list_arg) {
            for (int i = 0; i < cmd->list_count; i++) {
                free(cmd->list_arg[i]);
            }
            free(cmd->list_arg);
        }
        memset(cmd, 0, sizeof(*cmd));
    }
}

const char* command_type_name(command_type_t type) {
    switch (type) {
        case CMD_CONTINUE:        return "continue";
        case CMD_SINGLE_STEP:     return "step";
        case CMD_REGISTER_DUMP:   return "register dump";
        case CMD_REGISTER_READ:   return "register read";
        case CMD_REGISTER_WRITE:  return "register write";
        case CMD_BREAKPOINT_FUNC: return "breakpoint function";
        case CMD_BREAKPOINT_ADDR: return "breakpoint address";
        case CMD_MEMORY_READ:     return "memory read";
        case CMD_MEMORY_WRITE:    return "memory write";
        case CMD_BACKTRACE:       return "backtrace";
        case CMD_SET_OUTPUT:      return "set output";
        case CMD_SET_EXPAND:      return "set expand";
        case CMD_FILTER:          return "filter";
        case CMD_HELP:            return "help";
        case CMD_QUIT:            return "quit";
        default:                  return "unknown";
    }
}

void parser_print_usage(void) {
    const char *color = NULL;
    const char *reset = "";
    const char *bold = "";

    /* Use theme colors if available */
    extern int theme_using_colors(void);
    extern const char *theme_color(color_t fg);
    extern const char *theme_style(style_t style);

    if (theme_using_colors()) {
        color = theme_color(COLOR_CYAN);
        bold = theme_style(STYLE_BOLD);
        reset = "\033[0m";
    }

    printf("\n%s%s=== Debugger Commands ===%s\n\n", bold, color ? color : "", reset);

    /* Execution control */
    printf("%sExecution Control:%s\n", bold, reset);
    printf("  continue, c           Continue execution\n");
    printf("  step, s               Single step\n");
    printf("  next, n               Step over (alias for step)\n");
    printf("\n");

    /* Information display */
    printf("%sInformation Display:%s\n", bold, reset);
    printf("  backtrace, bt         Show backtrace\n");
    printf("  register dump, r      Dump all registers\n");
    printf("  register read <reg>   Read a register (e.g., rax, rip)\n");
    printf("  memory read <addr>    Read memory at address\n");
    printf("\n");

    /* Breakpoints */
    printf("%sBreakpoints:%s\n", bold, reset);
    printf("  b func <name>         Set breakpoint at function\n");
    printf("  b <addr>              Set breakpoint at address\n");
    printf("\n");

    /* Settings */
    printf("%sSettings:%s\n", bold, reset);
    printf("  set output <level>    Set output level (quiet|normal|verbose|debug)\n");
    printf("  set expand <level>    Set expansion level (none|normal|full)\n");
    printf("  filter <cats>         Filter categories (e.g., signal,backtrace)\n");
    printf("\n");

    /* Other */
    printf("%sOther:%s\n", bold, reset);
    printf("  help, h               Show this help\n");
    printf("  quit, q               Quit debugger\n");
    printf("\n");
}

void parser_print_command_help(command_type_t type) {
    const char *name = command_type_name(type);

    switch (type) {
        case CMD_CONTINUE:
            printf("Usage: continue\n");
            printf("  Continue execution until next breakpoint or signal.\n");
            printf("Aliases: c\n");
            break;

        case CMD_SINGLE_STEP:
            printf("Usage: step\n");
            printf("  Execute a single instruction.\n");
            printf("Aliases: s, n\n");
            break;

        case CMD_REGISTER_DUMP:
            printf("Usage: register dump\n");
            printf("  Display all register values.\n");
            printf("Aliases: r\n");
            break;

        case CMD_BACKTRACE:
            printf("Usage: backtrace\n");
            printf("  Display the current call stack.\n");
            printf("Aliases: bt\n");
            break;

        case CMD_SET_OUTPUT:
            printf("Usage: set output <level>\n");
            printf("  Set output verbosity level.\n");
            printf("  Levels: quiet (signals only), normal, verbose, debug\n");
            break;

        case CMD_SET_EXPAND:
            printf("Usage: set expand <level>\n");
            printf("  Set section expansion level.\n");
            printf("  Levels: none (compact), normal, full (all details)\n");
            break;

        case CMD_FILTER:
            printf("Usage: filter <categories>\n");
            printf("  Enable only specified output categories.\n");
            printf("  Categories: process, signal, backtrace, registers, memory, breakpoint\n");
            printf("  Example: filter signal,backtrace\n");
            break;

        default:
            printf("No detailed help available for: %s\n", name);
            break;
    }
}
