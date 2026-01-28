/**
 * @file debugger.h
 * @brief Core debugger functionality - Ptrace, signal handling
 */

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG
#include <libconfig.h>
#endif

/* Process information */
typedef struct {
    pid_t pid;
    pid_t ppid;
    gid_t gid;
    char path[512];
    uint64_t offset;
    uint64_t base;
} process_info_t;

/* Signal information */
typedef struct {
    int signo;
    int err_no;
    int code;
    void *addr;
    const char *name;
} signal_info_t;

/* Debugger state */
typedef struct {
    process_info_t proc;
    signal_info_t signal;
    int running;
    int exited;
    int exit_code;
} debugger_state_t;

/* Core functions */
int debugger_init(const char *program, char *const argv[], debugger_state_t *state);
int debugger_continue(debugger_state_t *state);
int debugger_single_step(debugger_state_t *state);
int debugger_wait(debugger_state_t *state);
void debugger_cleanup(debugger_state_t *state);

/* Signal handling */
const char *signal_get_name(int signo);
int debugger_get_signal(debugger_state_t *state, signal_info_t *info);

#endif /* DEBUGGER_H */
