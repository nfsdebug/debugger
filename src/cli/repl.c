/**
 * @file repl.c
 * @brief Interactive mode (REPL with linenoise)
 */

#include "repl.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declaration */
struct debugger_state;

#ifdef HAVE_LINENOISE

/* Global REPL configuration */
static repl_config_t g_repl_config = {
    .prompt = "dbg> ",
    .history_max_len = 1000,
    .history_path = NULL
};

/* Global debugger state */
static debugger_state_t *g_debugger_state = NULL;

int repl_init(const repl_config_t *config) {
    if (config) {
        memcpy(&g_repl_config, config, sizeof(g_repl_config));
    }

    /* Configure linenoise */
    linenoiseHistoryMaxLen(g_repl_config.history_max_len);

    if (g_repl_config.history_path) {
        linenoiseHistoryLoad(g_repl_config.history_path);
    }

    /* Set completion callback */
    linenoiseSetCompletionCallback(repl_completion);

    /* Set hints callback (optional) */
    linenoiseSetHintsCallback(repl_hint);

    return 0;
}

void repl_completion(const char *buf, linenoiseCompletions *lc) {
    /* TODO: Implement command completion */
    if (buf[0] == 'r') {
        linenoiseAddCompletion(lc, "register");
        linenoiseAddCompletion(lc, "register read");
        linenoiseAddCompletion(lc, "register write");
    } else if (buf[0] == 'm') {
        linenoiseAddCompletion(lc, "memory");
        linenoiseAddCompletion(lc, "memory read");
        linenoiseAddCompletion(lc, "memory write");
    } else if (buf[0] == 'b') {
        linenoiseAddCompletion(lc, "b func");
        linenoiseAddCompletion(lc, "breakpoint");
    }
}

char *repl_hint(const char *buf, int *color, int *bold) {
    /* TODO: Implement hints */
    (void)buf;
    *color = 35;  /* Magenta */
    *bold = 0;
    return NULL;
}

int repl_run(void) {
    char *line;

    printf("Debugger CLI - Type 'help' for commands, 'quit' to exit\n");

    while ((line = linenoise(g_repl_config.prompt)) != NULL) {
        /* Add to history */
        linenoiseHistoryAdd(line);

        /* Parse command */
        command_t cmd;
        if (parser_parse_command(line, &cmd) == 0) {
            /* Execute command */
            switch (cmd.type) {
                case CMD_CONTINUE:
                    printf("Continuing...\n");
                    if (g_debugger_state) {
                        debugger_continue(g_debugger_state);
                    }
                    break;

                case CMD_REGISTER_DUMP:
                    printf("Register dump\n");
                    /* TODO: Call registers_print() */
                    break;

                case CMD_BACKTRACE:
                    printf("Backtrace\n");
                    /* TODO: Call backtrace_print() */
                    break;

                case CMD_HELP:
                    parser_print_usage();
                    break;

                case CMD_QUIT:
                    linenoiseFree(line);
                    return 0;

                default:
                    printf("Command not yet implemented\n");
                    break;
            }

            parser_free_command(&cmd);
        }

        linenoiseFree(line);
    }

    return 0;
}

void repl_cleanup(void) {
    if (g_repl_config.history_path) {
        linenoiseHistorySave(g_repl_config.history_path);
    }
    linenoiseHistoryFree();
}

#else /* !HAVE_LINENOISE */

/* Stub implementation without linenoise */

int repl_init(const repl_config_t *config) {
    (void)config;
    printf("Warning: linenoise not available, interactive mode disabled\n");
    return -1;
}

int repl_run(void) {
    printf("Error: linenoise not available\n");
    return -1;
}

void repl_cleanup(void) {
    /* Nothing to do */
}

#endif
