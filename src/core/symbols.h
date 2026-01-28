/**
 * @file symbols.h
 * @brief ELF symbol resolution for breakpoints
 */

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdint.h>
#include <sys/types.h>

/* Symbol information */
typedef struct {
    char name[256];
    uint64_t address;
    uint64_t size;
    int type;  /* 0 = unknown, 1 = function, 2 = object */
} symbol_t;

/* Symbol table state */
typedef struct {
    symbol_t *symbols;
    int count;
    char *program_path;
} symbol_table_t;

/* Initialize symbol table from program */
int symbols_init(symbol_table_t *table, const char *program_path);

/* Cleanup symbol table */
void symbols_cleanup(symbol_table_t *table);

/* Find symbol by name */
uint64_t symbols_find_address(symbol_table_t *table, const char *name);

/* Find symbol name by address (reverse lookup) */
const char* symbols_find_name(symbol_table_t *table, uint64_t addr);

/* List all functions */
void symbols_list_functions(symbol_table_t *table);

#endif /* SYMBOLS_H */
