/**
 * @file disasm.c
 * @brief Disassembly view
 */

#include "disasm.h"
#include "../display/output.h"
#include "../display/sections.h"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* Very basic x86-64 instruction decoder (simplified) */
static const char* decode_insn(uint8_t *bytes, int *len) {
    /* Common single-byte instructions */
    switch (bytes[0]) {
        case 0x90: *len = 1; return "nop";
        case 0xC3: *len = 1; return "ret";
        case 0x55: *len = 1; return "push   %rbp";
        case 0x5D: *len = 1; return "pop    %rbp";
        case 0x89: *len = 2; return "mov";
        case 0x8B: *len = 2; return "mov";
        case 0xB8: case 0xB9: case 0xBA: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF:
            *len = 5; return "mov    $imm, %r";
        case 0xE8: *len = 5; return "call";
        case 0xE9: *len = 5; return "jmp";
        case 0xEB: *len = 2; return "jmp";
        case 0xCC: *len = 1; return "int3";
        case 0x53: *len = 1; return "push   %rbx";
        case 0x5B: *len = 1; return "pop    %rbx";
        case 0x50: *len = 1; return "push   %rax";
        case 0x58: *len = 1; return "pop    %rax";
        case 0x48: *len = 1; return "rex...";
        /* endbr64 */
        case 0xF3:
            if (bytes[1] == 0x0F && bytes[2] == 0x1E && bytes[3] == 0xFA) {
                *len = 4; return "endbr64";
            }
            *len = 1; return "data";
        case 0x0F:
            if (bytes[1] == 0x05) { *len = 5; return "syscall"; }
            if (bytes[1] == 0xAE) { *len = 3; return "pause"; }
            *len = 2; return "data...";
        default:
            *len = 1; return "byte";
    }
}

/* Print instruction at address */
static int print_instruction(pid_t pid, uint64_t addr, int is_current) {
    /* Read instruction */
    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, pid, (void *)addr, NULL);
    if (errno != 0) {
        return -1;
    }

    uint8_t *bytes = (uint8_t *)&data;

    /* Decode */
    int insn_len = 1;
    const char *mnemonic = decode_insn(bytes, &insn_len);

    /* Print bytes */
    char byte_str[24] = {0};
    for (int i = 0; i < insn_len && i < 8; i++) {
        snprintf(byte_str + i * 3, sizeof(byte_str) - i * 3, "%02x ", bytes[i]);
    }

    const char *prefix = is_current ? "→" : "  ";
    output_normal(CAT_PROCESS, "%s %016lx  %-24s  %s\n", prefix, addr, byte_str, mnemonic);

    return insn_len;
}

/* Disassemble around current RIP */
int disasm_around_rip(pid_t pid, int before, int after) {
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
        output_error("Failed to read registers");
        return -1;
    }

    section_print_header("DISASSEMBLY", sections_get_global_expand(), NULL);
    output_normal(CAT_PROCESS, "  Current RIP: 0x%016lx\n\n", regs.rip);

    /* Try to show some instructions before and after */
    uint64_t addr = regs.rip;

    /* Show 3 instructions before (heuristic: go back 3*15 bytes) */
    output_normal(CAT_PROCESS, "  ↑ Previous:\n");
    for (int i = 0; i < before; i++) {
        uint64_t test_addr = addr - (before - i) * 16;
        if (test_addr < 0x1000) continue;
        if (print_instruction(pid, test_addr, 0) < 0) continue;
    }

    /* Show current */
    output_normal(CAT_PROCESS, "\n  → Current:\n");
    int insn_len = print_instruction(pid, addr, 1);
    addr += insn_len;

    /* Show instructions after */
    output_normal(CAT_PROCESS, "\n  ↓ Next:\n");
    for (int i = 0; i < after; i++) {
        if (print_instruction(pid, addr, 0) < 0) break;
        /* Get next instruction length by decoding */
        long data = ptrace(PTRACE_PEEKDATA, pid, (void *)addr, NULL);
        if (errno != 0) break;
        uint8_t *bytes = (uint8_t *)&data;
        int len = 1;
        decode_insn(bytes, &len);
        if (len < 1) len = 1;
        addr += len;
    }

    return 0;
}

int disasm_around_addr(pid_t pid, uint64_t addr, int before, int after) {
    section_print_header("DISASSEMBLY", sections_get_global_expand(), NULL);
    output_normal(CAT_PROCESS, "  Address: 0x%016lx\n\n", addr);

    /* Show instructions before (heuristic) */
    if (before > 0) {
        output_normal(CAT_PROCESS, "  ↑ Previous:\n");
        for (int i = 0; i < before; i++) {
            uint64_t test_addr = addr - (before - i) * 16;
            if (test_addr < 0x1000) continue;
            if (print_instruction(pid, test_addr, 0) < 0) continue;
        }
        output_normal(CAT_PROCESS, "\n");
    }

    /* Show current */
    output_normal(CAT_PROCESS, "  → Target:\n");
    int insn_len = print_instruction(pid, addr, 1);
    if (insn_len < 1) insn_len = 1;

    /* Show instructions after */
    if (after > 0) {
        output_normal(CAT_PROCESS, "\n  ↓ Next:\n");
        uint64_t current = addr + insn_len;
        for (int i = 0; i < after; i++) {
            if (print_instruction(pid, current, 0) < 0) break;
            /* Get next instruction length */
            long data = ptrace(PTRACE_PEEKDATA, pid, (void *)current, NULL);
            if (errno != 0) break;
            uint8_t *bytes = (uint8_t *)&data;
            int len = 1;
            decode_insn(bytes, &len);
            if (len < 1) len = 1;
            current += len;
        }
    }

    return 0;
}

int disasm_around_func(pid_t pid, const char *func_name, int before, int after) {
    (void)pid;
    (void)func_name;
    (void)before;
    (void)after;
    output_error("Function resolution not yet implemented for disasm");
    output_normal(CAT_PROCESS, "  Tip: Use the function address directly\n");
    return -1;
}
