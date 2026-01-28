/**
 * @file repl.c
 * @brief Interactive mode (REPL)
 */

#include "repl.h"
#include "parser.h"
#include "../display/output.h"
#include "../display/theme.h"
#include "../display/sections.h"
#include "../core/debugger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LINENOISE
#include <linenoise.h>
#endif

/* Global REPL configuration */
static repl_config_t g_repl_config = {
    .prompt = "dbg> ",
    .history_max_len = 1000,
    .history_path = NULL
};

/* Global debugger state */
static debugger_state_t *g_debugger_state = NULL;

/* Command completion suggestions */
static const char *commands[] = {
    "continue", "step", "next",
    "register", "register dump", "register read", "register write",
    "memory", "memory read", "memory write",
    "backtrace",
    "breakpoint", "break",
    "set", "set output", "set expand",
    "filter",
    "help",
    "quit",
    NULL
};

int repl_init(const repl_config_t *config) {
    if (config) {
        memcpy(&g_repl_config, config, sizeof(g_repl_config));
    }

#ifdef HAVE_LINENOISE
    /* Configure linenoise */
    linenoiseHistoryMaxLen(g_repl_config.history_max_len);

    if (g_repl_config.history_path) {
        linenoiseHistoryLoad(g_repl_config.history_path);
    }

    /* Set completion callback */
    linenoiseSetCompletionCallback(repl_completion);

    /* Set hints callback */
    linenoiseSetHintsCallback(repl_hint);
#else
    printf("Warning: linenoise not available, using basic input\n");
#endif

    return 0;
}

void repl_set_debugger_state(debugger_state_t *state) {
    g_debugger_state = state;
}

#ifdef HAVE_LINENOISE
void repl_completion(const char *buf, linenoiseCompletions *lc) {
    if (!buf || !buf[0]) return;

    /* Match commands */
    size_t buflen = strlen(buf);
    for (int i = 0; commands[i] != NULL; i++) {
        if (strncmp(buf, commands[i], buflen) == 0) {
            linenoiseAddCompletion(lc, commands[i]);
        }
    }

    /* Special completions */
    if (strncmp(buf, "set output ", 11) == 0) {
        linenoiseAddCompletion(lc, "set output quiet");
        linenoiseAddCompletion(lc, "set output normal");
        linenoiseAddCompletion(lc, "set output verbose");
        linenoiseAddCompletion(lc, "set output debug");
    } else if (strncmp(buf, "set expand ", 11) == 0) {
        linenoiseAddCompletion(lc, "set expand none");
        linenoiseAddCompletion(lc, "set expand normal");
        linenoiseAddCompletion(lc, "set expand full");
    }
}

char *repl_hint(const char *buf, int *color, int *bold) {
    /* Provide hints for commands */
    if (strncmp(buf, "set", 3) == 0 && strlen(buf) == 3) {
        *color = 35;  /* Magenta */
        *bold = 0;
        return " output | expand";
    }

    if (strncmp(buf, "register", 8) == 0 && strlen(buf) == 8) {
        *color = 35;
        *bold = 0;
        return " dump | read | write";
    }

    if (strncmp(buf, "memory", 6) == 0 && strlen(buf) == 6) {
        *color = 35;
        *bold = 0;
        return " read | write";
    }

    return NULL;
}
#endif

static int execute_command(const command_t *cmd) {
    if (!cmd) return -1;

    switch (cmd->type) {
        case CMD_CONTINUE:
            output_verbose(CAT_PROCESS, "Continuing execution...\n");
            if (g_debugger_state) {
                /* TODO: Call debugger_continue() */
                (void)g_debugger_state;
            }
            break;

        case CMD_SINGLE_STEP:
            output_verbose(CAT_PROCESS, "Single step...\n");
            if (g_debugger_state) {
                /* TODO: Call debugger_single_step() */
                (void)g_debugger_state;
            }
            output_stats_inc_step();
            break;

        case CMD_REGISTER_DUMP:
            output_normal(CAT_REGISTERS, "Dumping registers...\n");
            registers_print(NULL);
            break;

        case CMD_BACKTRACE:
            output_normal(CAT_BACKTRACE, "Showing backtrace...\n");
            backtrace_print(g_debugger_state, NULL);
            break;

        case CMD_SET_OUTPUT:
            if (cmd->int_arg >= OUTPUT_QUIET && cmd->int_arg <= OUTPUT_DEBUG) {
                output_set_level((output_level_t)cmd->int_arg);
                const char *levels[] = {"QUIET", "NORMAL", "VERBOSE", "DEBUG"};
                output_normal(CAT_PROCESS, "Output level set to %s\n", levels[cmd->int_arg]);
            } else {
                output_error("Invalid output level");
            }
            break;

        case CMD_SET_EXPAND:
            if (cmd->int_arg >= EXPAND_NONE && cmd->int_arg <= EXPAND_FULL) {
                sections_set_global_expand((expand_level_t)cmd->int_arg);
                const char *levels[] = {"NONE", "NORMAL", "FULL"};
                output_normal(CAT_PROCESS, "Expand level set to %s\n", levels[cmd->int_arg]);
            } else {
                output_error("Invalid expand level");
            }
            break;

        case CMD_FILTER: {
            if (cmd->list_count > 0) {
                /* First disable all, then enable only specified */
                output_config_t cfg = {
                    .level = OUTPUT_NORMAL,
                    .enabled_categories = 0,
                    .use_colors = 1,
                    .show_prefix = 1,
                    .show_timestamp = 0
                };
                output_init(&cfg);

                /* Parse and enable categories */
                for (int i = 0; i < cmd->list_count; i++) {
                    const char *cat = cmd->list_arg[i];
                    output_category_t c = CAT_ALL;

                    if (strcmp(cat, "process") == 0) c = CAT_PROCESS;
                    else if (strcmp(cat, "signal") == 0) c = CAT_SIGNAL;
                    else if (strcmp(cat, "backtrace") == 0) c = CAT_BACKTRACE;
                    else if (strcmp(cat, "registers") == 0) c = CAT_REGISTERS;
                    else if (strcmp(cat, "memory") == 0) c = CAT_MEMORY;
                    else if (strcmp(cat, "breakpoint") == 0) c = CAT_BREAKPOINT;

                    if (c != CAT_ALL) {
                        output_enable_category(c);
                    }
                }
                output_normal(CAT_PROCESS, "Filter applied\n");
            }
            break;
        }

        case CMD_HELP:
            parser_print_usage();
            break;

        case CMD_QUIT:
            return 1;  /* Signal to exit */

        default:
            output_error("Unknown or unimplemented command");
            break;
    }

    return 0;
}

int repl_run(void) {
#ifdef HAVE_LINENOISE
    char *line;

    printf("Debugger CLI - Type 'help' for commands, 'quit' to exit\n\n");

    while ((line = linenoise(g_repl_config.prompt)) != NULL) {
        /* Skip empty lines */
        if (line[0] == '\0') {
            linenoiseFree(line);
            continue;
        }

        /* Add to history */
        linenoiseHistoryAdd(line);

        /* Parse command */
        command_t cmd;
        if (parser_parse_command(line, &cmd) == 0) {
            /* Execute command */
            if (execute_command(&cmd) == 1) {
                parser_free_command(&cmd);
                linenoiseFree(line);
                break;
            }
            parser_free_command(&cmd);
        } else {
            output_error("Failed to parse command");
        }

        linenoiseFree(line);
    }

    return 0;
#else
    /* Basic input without linenoise */
    char line[4096];

    printf("Debugger CLI - Type 'help' for commands, 'quit' to exit\n\n");

    while (1) {
        printf("%s", g_repl_config.prompt);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        /* Remove newline */
        line[strcspn(line, "\n")] = 0;

        /* Skip empty lines */
        if (line[0] == '\0') continue;

        /* Parse command */
        command_t cmd;
        if (parser_parse_command(line, &cmd) == 0) {
            /* Execute command */
            if (execute_command(&cmd) == 1) {
                parser_free_command(&cmd);
                break;
            }
            parser_free_command(&cmd);
        } else {
            output_error("Failed to parse command");
        }
    }

    return 0;
#endif
}

void repl_cleanup(void) {
#ifdef HAVE_LINENOISE
    if (g_repl_config.history_path) {
        linenoiseHistorySave(g_repl_config.history_path);
    }
    linenoiseHistoryFree();
#endif
}
