/**
 * @file memory.h
 * @brief Memory operations (read/write)
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

/* Memory read result */
typedef struct {
    void *address;
    uint64_t value;
    int valid;
} memory_value_t;

/* Memory region */
typedef struct {
    void *start;
    void *end;
    char permissions[5];  /* rwxp */
    char path[256];
} memory_region_t;

/* Read a word from memory */
int memory_read(pid_t pid, void *addr, uint64_t *value);

/* Write a word to memory */
int memory_write(pid_t pid, void *addr, uint64_t value);

/* Read multiple words */
int memory_read_bytes(pid_t pid, void *addr, uint8_t *buffer, size_t length);

/* Write multiple words */
int memory_write_bytes(pid_t pid, void *addr, const uint8_t *buffer, size_t length);

/* Get memory maps from /proc/pid/maps */
int memory_get_maps(pid_t pid, memory_region_t **regions, int *count);

/* Free memory maps */
void memory_free_maps(memory_region_t *regions);

/* Print memory value (formatted) */
void memory_print_value(const memory_value_t *mem);

/* Print memory region */
void memory_print_region(const memory_region_t *region);

#endif /* MEMORY_H */
