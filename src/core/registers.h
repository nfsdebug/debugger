/**
 * @file registers.h
 * @brief Register operations (read/write)
 */

#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>
#include <sys/types.h>

/* Register names */
typedef enum {
    REG_RAX, REG_RBX, REG_RCX, REG_RDX,
    REG_RDI, REG_RSI, REG_RBP, REG_RSP,
    REG_R8, REG_R9, REG_R10, REG_R11,
    REG_R12, REG_R13, REG_R14, REG_R15,
    REG_RIP, REG_RFLAGS,
    REG_CS, REG_SS, REG_DS, REG_ES,
    REG_FS, REG_GS, REG_FS_BASE, REG_GS_BASE,
    REG_ORIG_RAX,
    REG_COUNT
} register_id_t;

/* Register value */
typedef struct {
    char name[8];
    uint64_t value;
    int valid;
} register_value_t;

/* Register state (all registers) */
typedef struct {
    register_value_t regs[REG_COUNT];
} register_state_t;

/* Read a single register */
int register_read(pid_t pid, register_id_t reg, uint64_t *value);

/* Write a single register */
int register_write(pid_t pid, register_id_t reg, uint64_t value);

/* Dump all registers */
int register_dump(pid_t pid, register_state_t *state);

/* Get register by name */
register_id_t register_get_id(const char *name);

/* Print register (with formatting) */
void register_print(const register_value_t *reg);

/* Print all registers (formatted) */
void registers_print(const register_state_t *state);

/* Get register ID by name (const-correct) */
register_id_t register_get_id(const char *name);

#endif /* REGISTERS_H */
