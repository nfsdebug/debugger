/**
 * @file registers.c
 * @brief Register operations (read/write)
 */

#include "registers.h"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Register name mapping */
static const char *reg_names[] = {
    "rax", "rbx", "rcx", "rdx",
    "rdi", "rsi", "rbp", "rsp",
    "r8", "r9", "r10", "r11",
    "r12", "r13", "r14", "r15",
    "rip", "rflags",
    "cs", "ss", "ds", "es",
    "fs", "gs", "fs_base", "gs_base",
    "orig_rax"
};

/* Compile-time assertion: register count matches array size */
enum { REG_NAMES_SIZE = sizeof(reg_names) / sizeof(reg_names[0]) };
#ifndef REG_COUNT_MATCHES
#define REG_COUNT_MATCHES (REG_NAMES_SIZE == REG_COUNT)
#endif
typedef char reg_count_check[(REG_NAMES_SIZE == REG_COUNT) ? 1 : -1];

int register_read(pid_t pid, register_id_t reg, uint64_t *value) {
    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
        return -1;
    }

    switch (reg) {
        case REG_RAX: *value = regs.rax; break;
        case REG_RBX: *value = regs.rbx; break;
        case REG_RCX: *value = regs.rcx; break;
        case REG_RDX: *value = regs.rdx; break;
        case REG_RDI: *value = regs.rdi; break;
        case REG_RSI: *value = regs.rsi; break;
        case REG_RBP: *value = regs.rbp; break;
        case REG_RSP: *value = regs.rsp; break;
        case REG_R8:  *value = regs.r8; break;
        case REG_R9:  *value = regs.r9; break;
        case REG_R10: *value = regs.r10; break;
        case REG_R11: *value = regs.r11; break;
        case REG_R12: *value = regs.r12; break;
        case REG_R13: *value = regs.r13; break;
        case REG_R14: *value = regs.r14; break;
        case REG_R15: *value = regs.r15; break;
        case REG_RIP: *value = regs.rip; break;
        case REG_RFLAGS: *value = regs.eflags; break;
        case REG_CS: *value = regs.cs; break;
        case REG_SS: *value = regs.ss; break;
        case REG_DS: *value = regs.ds; break;
        case REG_ES: *value = regs.es; break;
        case REG_FS: *value = regs.fs; break;
        case REG_GS: *value = regs.gs; break;
        case REG_FS_BASE: *value = regs.fs_base; break;
        case REG_GS_BASE: *value = regs.gs_base; break;
        case REG_ORIG_RAX: *value = regs.orig_rax; break;
        default: return -1;
    }

    return 0;
}

int register_write(pid_t pid, register_id_t reg, uint64_t value) {
    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
        return -1;
    }

    switch (reg) {
        case REG_RAX: regs.rax = value; break;
        case REG_RBX: regs.rbx = value; break;
        case REG_RCX: regs.rcx = value; break;
        case REG_RDX: regs.rdx = value; break;
        case REG_RDI: regs.rdi = value; break;
        case REG_RSI: regs.rsi = value; break;
        case REG_RBP: regs.rbp = value; break;
        case REG_RSP: regs.rsp = value; break;
        case REG_R8:  regs.r8 = value; break;
        case REG_R9:  regs.r9 = value; break;
        case REG_R10: regs.r10 = value; break;
        case REG_R11: regs.r11 = value; break;
        case REG_R12: regs.r12 = value; break;
        case REG_R13: regs.r13 = value; break;
        case REG_R14: regs.r14 = value; break;
        case REG_R15: regs.r15 = value; break;
        case REG_RIP: regs.rip = value; break;
        case REG_RFLAGS: regs.eflags = value; break;
        case REG_CS: regs.cs = value; break;
        case REG_SS: regs.ss = value; break;
        case REG_DS: regs.ds = value; break;
        case REG_ES: regs.es = value; break;
        case REG_FS: regs.fs = value; break;
        case REG_GS: regs.gs = value; break;
        case REG_FS_BASE: regs.fs_base = value; break;
        case REG_GS_BASE: regs.gs_base = value; break;
        case REG_ORIG_RAX: regs.orig_rax = value; break;
        default: return -1;
    }

    return ptrace(PTRACE_SETREGS, pid, NULL, &regs) < 0 ? -1 : 0;
}

int register_dump(pid_t pid, register_state_t *state) {
    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
        return -1;
    }

    state->regs[REG_RAX].value = regs.rax;
    state->regs[REG_RBX].value = regs.rbx;
    state->regs[REG_RCX].value = regs.rcx;
    state->regs[REG_RDX].value = regs.rdx;
    state->regs[REG_RDI].value = regs.rdi;
    state->regs[REG_RSI].value = regs.rsi;
    state->regs[REG_RBP].value = regs.rbp;
    state->regs[REG_RSP].value = regs.rsp;
    state->regs[REG_R8].value = regs.r8;
    state->regs[REG_R9].value = regs.r9;
    state->regs[REG_R10].value = regs.r10;
    state->regs[REG_R11].value = regs.r11;
    state->regs[REG_R12].value = regs.r12;
    state->regs[REG_R13].value = regs.r13;
    state->regs[REG_R14].value = regs.r14;
    state->regs[REG_R15].value = regs.r15;
    state->regs[REG_RIP].value = regs.rip;
    state->regs[REG_RFLAGS].value = regs.eflags;
    state->regs[REG_CS].value = regs.cs;
    state->regs[REG_SS].value = regs.ss;
    state->regs[REG_DS].value = regs.ds;
    state->regs[REG_ES].value = regs.es;
    state->regs[REG_FS].value = regs.fs;
    state->regs[REG_GS].value = regs.gs;
    state->regs[REG_FS_BASE].value = regs.fs_base;
    state->regs[REG_GS_BASE].value = regs.gs_base;
    state->regs[REG_ORIG_RAX].value = regs.orig_rax;

    for (int i = 0; i < REG_COUNT; i++) {
        strncpy(state->regs[i].name, reg_names[i], sizeof(state->regs[i].name) - 1);
        state->regs[i].valid = 1;
    }

    return 0;
}

register_id_t register_get_id(const char *name) {
    char lower_name[8];
    strncpy(lower_name, name, sizeof(lower_name) - 1);
    for (int i = 0; lower_name[i]; i++) {
        lower_name[i] = tolower(lower_name[i]);
    }

    for (int i = 0; i < REG_COUNT; i++) {
        if (strcmp(lower_name, reg_names[i]) == 0) {
            return i;
        }
    }
    return REG_COUNT;
}

void register_print(const register_value_t *reg) {
    printf("%s = 0x%016lx\n", reg->name, reg->value);
}

void registers_print(const register_state_t *state) {
    printf("[REGISTERS]\n");
    for (int i = 0; i < REG_COUNT; i++) {
        if (state->regs[i].valid) {
            printf("  %s = 0x%016lx\n", state->regs[i].name, state->regs[i].value);
        }
    }
}
