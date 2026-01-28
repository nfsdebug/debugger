/**
 * @file readline.h
 * @brief Command line input with readline and history
 */

#ifndef READLINE_H
#define READLINE_H

/* Initialize readline (history, completion) */
int rl_init(void);

/* Read a line of input with readline */
char* rl_readline(const char *prompt);

/* Add line to history */
void rl_add_history(const char *line);

/* Cleanup readline */
void rl_cleanup(void);

#endif /* READLINE_H */
