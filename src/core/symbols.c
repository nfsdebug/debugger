/**
 * @file symbols.c
 * @brief ELF symbol resolution
 *
 * Reads function symbols from ELF binaries for breakpoint resolution
 */

#include "symbols.h"
#include "../display/output.h"
#include "../display/sections.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_SYMBOLS 4096

/* Read ELF file into memory */
static void* read_elf_file(const char *path, size_t *size_out) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return NULL;
    }

    void *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (data == MAP_FAILED) {
        return NULL;
    }

    *size_out = st.st_size;
    return data;
}

/* Read symbols from a symbol table section */
static int read_symbol_table(symbol_table_t *table, void *elf_data,
                             Elf64_Sym *syms, int count, const char *strtab) {
    (void)elf_data; /* Unused parameter - kept for future use */
    for (int i = 0; i < count; i++) {
        /* Skip empty symbols */
        if (syms[i].st_name == 0) {
            continue;
        }

        const char *name = strtab + syms[i].st_name;

        /* Skip empty names or special symbols */
        if (name[0] == '\0' || name[0] == '$') {
            continue;
        }

        /* Get symbol type */
        int type = ELF64_ST_TYPE(syms[i].st_info);

        /* We want functions (STT_FUNC) and objects (STT_OBJECT) */
        if (type != STT_FUNC && type != STT_OBJECT) {
            continue;
        }

        /* Skip if in undefined section */
        if (syms[i].st_shndx == SHN_UNDEF) {
            continue;
        }

        /* Check if already in table (avoid duplicates) */
        int found = 0;
        for (int j = 0; j < table->count; j++) {
            if (strcmp(table->symbols[j].name, name) == 0) {
                found = 1;
                break;
            }
        }
        if (found) continue;

        /* Add to table */
        if (table->count >= MAX_SYMBOLS) {
            break;
        }

        strncpy(table->symbols[table->count].name, name, 255);
        table->symbols[table->count].name[255] = '\0';
        table->symbols[table->count].address = syms[i].st_value;
        table->symbols[table->count].size = syms[i].st_size;
        table->symbols[table->count].type = (type == STT_FUNC) ? 1 : 2;
        table->count++;
    }

    return 0;
}

int symbols_init(symbol_table_t *table, const char *program_path) {
    memset(table, 0, sizeof(*table));

    /* Allocate symbol array */
    table->symbols = calloc(MAX_SYMBOLS, sizeof(symbol_t));
    if (!table->symbols) {
        output_error("Failed to allocate symbol table");
        return -1;
    }

    table->program_path = strdup(program_path);

    /* Read ELF file */
    size_t elf_size;
    void *elf_data = read_elf_file(program_path, &elf_size);
    if (!elf_data) {
        output_error("Failed to read ELF file: %s", program_path);
        return -1;
    }

    /* Verify ELF header */
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_data;

    /* Check ELF magic */
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        output_error("Not an ELF file: %s", program_path);
        munmap(elf_data, elf_size);
        return -1;
    }

    /* Check if 64-bit */
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        output_error("Not a 64-bit ELF file");
        munmap(elf_data, elf_size);
        return -1;
    }

    /* Get section headers */
    Elf64_Shdr *shdrs = (Elf64_Shdr *)((char *)elf_data + ehdr->e_shoff);
    int sh_count = ehdr->e_shnum;

    /* Get section header string table */
    char *shstrtab = (char *)elf_data + shdrs[ehdr->e_shstrndx].sh_offset;

    /* Find symbol tables */
    for (int i = 0; i < sh_count; i++) {
        const char *sh_name = shstrtab + shdrs[i].sh_name;

        /* Look for .symtab and .dynsym */
        if ((strcmp(sh_name, ".symtab") == 0 || strcmp(sh_name, ".dynsym") == 0)
            && shdrs[i].sh_type == SHT_SYMTAB) {

            /* Get string table for this symbol table */
            int strtab_idx = shdrs[i].sh_link;
            if (strtab_idx >= 0 && strtab_idx < sh_count) {
                char *strtab = (char *)elf_data + shdrs[strtab_idx].sh_offset;

                /* Read symbols */
                Elf64_Sym *syms = (Elf64_Sym *)((char *)elf_data + shdrs[i].sh_offset);
                int sym_count = shdrs[i].sh_size / sizeof(Elf64_Sym);

                read_symbol_table(table, elf_data, syms, sym_count, strtab);
            }
        }
    }

    munmap(elf_data, elf_size);

    output_normal(CAT_PROCESS, "Loaded %d symbols from %s\n", table->count, program_path);
    return 0;
}

void symbols_cleanup(symbol_table_t *table) {
    if (table) {
        free(table->symbols);
        free(table->program_path);
        table->symbols = NULL;
        table->program_path = NULL;
        table->count = 0;
    }
}

uint64_t symbols_find_address(const symbol_table_t *table, const char *name) {
    if (!table || !name) {
        return 0;
    }

    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return table->symbols[i].address;
        }
    }

    return 0;
}

const char* symbols_find_name(const symbol_table_t *table, uint64_t addr) {
    if (!table) {
        return NULL;
    }

    for (int i = 0; i < table->count; i++) {
        if (addr >= table->symbols[i].address &&
            addr < table->symbols[i].address + table->symbols[i].size) {
            return table->symbols[i].name;
        }
    }

    return NULL;
}

void symbols_list_functions(const symbol_table_t *table) {
    if (!table || table->count == 0) {
        output_normal(CAT_PROCESS, "No symbols loaded\n");
        return;
    }

    section_print_header("FUNCTIONS", sections_get_global_expand(), NULL);
    output_normal(CAT_PROCESS, "  Found %d symbol(s)\n\n", table->count);

    /* Group by type */
    for (int type = 1; type <= 2; type++) {
        const char *label = (type == 1) ? "Functions" : "Objects";
        int found = 0;

        for (int i = 0; i < table->count; i++) {
            if (table->symbols[i].type == type) {
                if (!found) {
                    output_normal(CAT_PROCESS, "  %s:\n", label);
                    found = 1;
                }
                output_normal(CAT_PROCESS, "    %s @ 0x%lx\n",
                              table->symbols[i].name, table->symbols[i].address);
            }
        }
    }
}
