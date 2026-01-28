/**
 * @file repl.h
 * @brief Interactive mode (REPL with linenoise)
 */

#ifndef REPL_H
#define REPL_H

#ifdef HAVE_LINENOISE
#include <linenoise.h>
#endif

/* REPL configuration */
typedef struct {
    const char *prompt;         /* Default: "dbg> " */
    int history_max_len;       /* Default: 1000 */
    const char *history_path;   /* NULL = no history file */
} repl_config_t;

/* Initialize REPL */
int repl_init(const repl_config_t *config);

/* REPL main loop */
int repl_run(void);

/* Cleanup REPL */
void repl_cleanup(void);

#ifdef HAVE_LINENOISE
/* Command completion callback (only available with linenoise) */
void repl_completion(const char *buf, linenoiseCompletions *lc);

/* Hints (optional inline help) */
char *repl_hint(const char *buf, int *color, int *bold);
#endif

#endif /* REPL_H */
