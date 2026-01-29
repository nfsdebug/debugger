/**
 * @file variables.h
 * @brief Variable lookup from DWARF debug information
 */

#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdint.h>
#include <sys/types.h>

/* Load variable information from DWARF */
int variables_load(const char *program_path);

/* Free variable information */
void variables_free(void);

/* Get the address of a variable expression (e.g., "x", "*ptr", "**ptr") */
/* Returns 0 on success, -1 on failure */
int variables_get_address(pid_t pid, const char *expr, uint64_t *addr);

/* Read a value from target process memory */
int variables_read_value(pid_t pid, uint64_t addr, void *buffer, size_t size);

/* Format and print a value */
void variables_print_value(uint64_t value, const char *type_hint);

#endif /* VARIABLES_H */
