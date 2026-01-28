/**
 * @file debugger.c
 * @brief Core debugger functionality - Ptrace, signal handling
 */

#include "debugger.h"
#include "utilities.h"
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/* Global debugger state */
static debugger_state_t g_state;

int debugger_init(const char *program, char *const argv[], debugger_state_t *state) {
    pid_t pid = fork();

    if (pid == 0) {
        /* Child */
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(program, argv);
        perror("execvp");
        return -1;
    } else if (pid < 0) {
        perror("fork");
        return -1;
    }

    /* Parent */
    memset(state, 0, sizeof(*state));
    state->proc.pid = pid;
    state->proc.ppid = getppid();
    state->proc.gid = getgid();
    strncpy(state->proc.path, program, sizeof(state->proc.path) - 1);

    /* Wait for initial stop */
    int status;
    waitpid(pid, &status, 0);

    /* Get program offset from /proc/pid/maps */
    char fname[128];
    snprintf(fname, sizeof(fname), "/proc/%d/maps", pid);
    FILE *f = fopen(fname, "r");
    if (f) {
        if (fscanf(f, "%llx", &state->proc.offset) == 1) {
            /* Success */
        }
        fclose(f);
    }

    state->running = 1;
    return 0;
}

int debugger_continue(debugger_state_t *state) {
    ptrace(PTRACE_SINGLESTEP, state->proc.pid, 0, 0);
    return debugger_wait(state);
}

int debugger_single_step(debugger_state_t *state) {
    ptrace(PTRACE_SINGLESTEP, state->proc.pid, 0, 0);
    return debugger_wait(state);
}

int debugger_wait(debugger_state_t *state) {
    int status;
    waitpid(state->proc.pid, &status, 0);

    if (WIFEXITED(status)) {
        state->running = 0;
        state->exited = 1;
        state->exit_code = WEXITSTATUS(status);
        return 0;
    }

    if (WIFSIGNALED(status)) {
        state->running = 0;
        state->exited = 1;
        state->signal.signo = WTERMSIG(status);
        state->signal.err_no = 0;
        return 0;
    }

    if (WIFSTOPPED(status)) {
        int sig = WSTOPSIG(status);
        if (sig == SIGTRAP) {
            /* Breakpoint hit */
        } else {
            state->signal.signo = sig;
            state->signal.err_no = 0;
            state->signal.name = signal_get_name(sig);
        }
    }

    return 1;
}

void debugger_cleanup(debugger_state_t *state) {
    /* Nothing to clean for now */
}

const char *signal_get_name(int signo) {
    switch (signo) {
        case SIGSEGV: return "SIGSEGV";
        case SIGTRAP: return "SIGTRAP";
        case SIGILL:  return "SIGILL";
        case SIGFPE:  return "SIGFPE";
        case SIGABRT: return "SIGABRT";
        default: return strsignal(signo);
    }
}

int debugger_get_signal(debugger_state_t *state, signal_info_t *info) {
    memcpy(info, &state->signal, sizeof(*info));
    return 0;
}
