/**
 * @file memory.c
 * @brief Memory operations (read/write)
 */

#include "memory.h"
#include <sys/ptrace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int memory_read(pid_t pid, void *addr, uint64_t *value) {
    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);

    if (errno != 0) {
        return -1;
    }

    *value = (uint64_t)data;
    return 0;
}

int memory_write(pid_t pid, void *addr, uint64_t value) {
    errno = 0;
    ptrace(PTRACE_POKEDATA, pid, addr, (void *)value);

    return errno != 0 ? -1 : 0;
}

int memory_read_bytes(pid_t pid, void *addr, uint8_t *buffer, size_t length) {
    for (size_t i = 0; i < length; i += sizeof(long)) {
        errno = 0;
        long data = ptrace(PTRACE_PEEKDATA, pid, (char *)addr + i, NULL);

        if (errno != 0) {
            return -1;
        }

        size_t remaining = length - i;
        size_t copy_size = (remaining < sizeof(long)) ? remaining : sizeof(long);
        memcpy(buffer + i, &data, copy_size);
    }

    return 0;
}

int memory_write_bytes(pid_t pid, void *addr, const uint8_t *buffer, size_t length) {
    for (size_t i = 0; i < length; i += sizeof(long)) {
        size_t remaining = length - i;
        size_t copy_size = (remaining < sizeof(long)) ? remaining : sizeof(long);

        long data = 0;
        memcpy(&data, buffer + i, copy_size);

        errno = 0;
        ptrace(PTRACE_POKEDATA, pid, (char *)addr + i, (void *)data);

        if (errno != 0) {
            return -1;
        }
    }

    return 0;
}

int memory_get_maps(pid_t pid, memory_region_t **regions, int *count) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", pid);

    FILE *f = fopen(path, "r");
    if (!f) {
        return -1;
    }

    /* Count lines first */
    int capacity = 64;
    *regions = malloc(capacity * sizeof(memory_region_t));
    *count = 0;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (*count >= capacity) {
            capacity *= 2;
            *regions = realloc(*regions, capacity * sizeof(memory_region_t));
        }

        memory_region_t *r = &(*regions)[*count];

        char perms[5];
        unsigned long start, end;
        int offset;
        char dev[32];
        unsigned long inode;
        char pathname[256];

        int n = sscanf(line, "%lx-%lx %4s %x %s %lx %255[^\n]",
                       &start, &end, perms, &offset, dev, &inode, pathname);

        r->start = (void *)start;
        r->end = (void *)end;
        strncpy(r->permissions, perms, sizeof(r->permissions) - 1);

        if (n >= 7) {
            strncpy(r->path, pathname, sizeof(r->path) - 1);
        } else {
            r->path[0] = '\0';
        }

        (*count)++;
    }

    fclose(f);
    return 0;
}

void memory_free_maps(memory_region_t *regions) {
    free(regions);
}

void memory_print_value(const memory_value_t *mem) {
    printf("0x%016lx: 0x%016lx\n", (uint64_t)mem->address, mem->value);
}

void memory_print_region(const memory_region_t *region) {
    printf("%p-%p %s %s\n",
           region->start, region->end,
           region->permissions,
           region->path[0] ? region->path : "[anonymous]");
}
