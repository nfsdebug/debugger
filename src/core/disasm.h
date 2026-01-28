/**
 * @file disasm.h
 * @brief Disassembly view
 */

#ifndef DISASM_H
#define DISASM_H

#include <stdint.h>
#include <sys/types.h>

/* Disassemble around current RIP */
int disasm_around_rip(pid_t pid, int before, int after);

/* Disassemble around specific address */
int disasm_around_addr(pid_t pid, uint64_t addr, int before, int after);

/* Disassemble around function (resolve symbol to address) */
int disasm_around_func(pid_t pid, const char *func_name, int before, int after);

#endif /* DISASM_H */
