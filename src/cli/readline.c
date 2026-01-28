/**
 * @file readline.c
 * @brief Command line input with readline and history
 */

#include "readline.h"
#include "../display/output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

/* Track if we're in interactive mode */
static int g_interactive = 0;

/* Maximum history entries */
#define MAX_HISTORY 1000

/* History file path */
static char *g_history_file = NULL;

/* Command completion */
static char* command_generator(const char *text, int state) {
    static const char *commands[] = {
        "continue", "step", "next",
        "register", "memory", "backtrace",
        "breakpoint", "break", "b",
        "watch", "watchpoint", "w",
        "list", "l",
        "disas", "disassemble",
        "info", "help", "h",
        "set", "filter",
        "quit", "q", "exit",
        NULL
    };
    static int list_index = 0;
    static size_t len = 0;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while (commands[list_index]) {
        const char *cmd = commands[list_index];
        list_index++;
        if (strncmp(cmd, text, len) == 0) {
            return strdup(cmd);
        }
    }

    return NULL;
}

static char** command_completion(const char *text, int start, int end) {
    (void)end;  /* Unused */
    if (start == 0) {
        /* Complete command name */
        return rl_completion_matches(text, command_generator);
    } else {
        /* No completion for command arguments yet */
        return NULL;
    }
}

int rl_init(void) {
    /* Check if we're in an interactive terminal */
    g_interactive = isatty(STDIN_FILENO);

    if (g_interactive) {
        /* Set completion function */
        rl_attempted_completion_function = command_completion;

        /* Read history from file */
        const char *home = getenv("HOME");
        if (home) {
            g_history_file = malloc(strlen(home) + 20);
            sprintf(g_history_file, "%s/.debugger_history", home);
            read_history(g_history_file);
        }
    }

    return 0;
}

char* rl_readline(const char *prompt) {
    if (g_interactive) {
        return readline(prompt);
    } else {
        /* Non-interactive: use fgets */
        static char buffer[4096];
        printf("%s", prompt);
        fflush(stdout);

        if (!fgets(buffer, sizeof(buffer), stdin)) {
            return NULL;
        }

        /* Remove trailing newline */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }

        return buffer;
    }
}

void rl_add_history(const char *line) {
    if (g_interactive && line && line[0] != '\0') {
        add_history(line);
    }
}

void rl_cleanup(void) {
    if (g_interactive) {
        /* Save history to file */
        if (g_history_file) {
            write_history(g_history_file);
            free(g_history_file);
        }

        /* Clear history */
        clear_history();
    }
}
