#ifndef utilities
#include "libdwarf.h"
#include "dwarf.h"

/* Struct to access all functions info */

struct functions_s
{
    Dwarf_Addr lowpc;
    Dwarf_Addr highpc;
    Dwarf_Unsigned line;
    char *name;
    char *path;
};

struct variable_s
{
    Dwarf_Addr lowpc;
    Dwarf_Addr highpc;
    Dwarf_Unsigned line;
    char *name;
    char *funcname;
    // struct function_s* func_var;
};

extern struct functions_s *func;
extern int count_func;

extern struct variable_s *var;
extern int count_var;

void dwarf_get_info(Dwarf_Debug dbg, Dwarf_Die in_die,
                    int is_info, int in_level,
                    char *name);

void get_dwarf(Dwarf_Debug dbg, Dwarf_Die die,
               int level,
               char *name);

void get_dbg(char *name);

void get_elf(char *name);

long int set_register(char *choice, pid_t child_pid, long long content);

#else
#endif