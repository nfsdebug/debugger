#ifndef utilities
#include "libdwarf.h"
#include "dwarf.h"

/* Struct to access all functions info */

struct functions_s
{
    Dwarf_Addr lowpc;
    Dwarf_Addr highpc;
    char *name;
};

struct variable_s
{
    Dwarf_Addr lowpc;
    Dwarf_Addr highpc;
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

#else
#endif