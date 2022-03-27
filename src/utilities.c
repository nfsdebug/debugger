#define _GNU_SOURCE
#include <sys/ptrace.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>

#include "libdwarf.h"
#include "dwarf.h"
#include "utilities.h"

#include <elf.h>
#include <sys/mman.h>

#define MAX_FUNCTIONS 100

struct functions_s *func;
int count_func = 0;
static int dienumber = 0;

struct variable_s *var;
int count_var = 0;
char *namesrc ;

long int set_register(char *choice, pid_t child_pid, long long content)
{
    struct user_regs_struct reg;
    ptrace(PTRACE_GETREGS, child_pid, NULL, &reg);

    if (strcasestr(choice, "rax"))
    {
        reg.rax = content;
    }
    else if (strcasestr(choice, "rbx"))
    {
        reg.rbx = content;
    }
    else if (strcasestr(choice, "rcx"))
    {
        reg.rcx = content;
    }
    else if (strcasestr(choice, "rdx"))
    {
        reg.rdx = content;
    }
    else if (strcasestr(choice, "rdi"))
    {
        reg.rdi = content;
    }
    else if (strcasestr(choice, "rsi"))
    {
        reg.rsi = content;
    }
    else if (strcasestr(choice, "rbp"))
    {
        reg.rbp = content;
    }
    else if (strcasestr(choice, "rsp,"))
    {
        reg.rsp = content;
    }
    else if (strcasestr(choice, "r8"))
    {
        reg.r8 = content;
    }
    else if (strcasestr(choice, "r9"))
    {
        reg.r9 = content;
    }
    else if (strcasestr(choice, "r10"))
    {
        reg.r10 = content;
    }
    else if (strcasestr(choice, "r11"))
    {
        reg.r11 = content;
    }
    else if (strcasestr(choice, "r12"))
    {
        reg.r12 = content;
    }
    else if (strcasestr(choice, "r13"))
    {
        reg.r13 = content;
    }
    else if (strcasestr(choice, "r14"))
    {
        reg.r14 = content;
    }
    else if (strcasestr(choice, "r15"))
    {
        reg.r15 = content;
    }
    else if (strcasestr(choice, "rip"))
    {
        reg.rip = content;
    }
    else if (strcasestr(choice, "rdx"))
    {
        reg.rdx = content;
    }
    else if (strcasestr(choice, "eflags"))
    {
        reg.eflags = content;
    }
    else if (strcasestr(choice, "cs"))
    {
        reg.cs = content;
    }
    else if (strcasestr(choice, "orig_rax"))
    {
        reg.orig_rax = content;
    }
    else if (strcasestr(choice, "fs_base"))
    {
        reg.fs_base = content;
    }
    else if (strcasestr(choice, "gs_base"))
    {
        reg.gs_base = content;
    }
    else if (strcasestr(choice, "fs"))
    {
        reg.fs = content;
    }
    else if (strcasestr(choice, "gs"))
    {
        reg.gs = content;
    }
    else if (strcasestr(choice, "ss"))
    {
        reg.ss = content;
    }
    else if (strcasestr(choice, "ds"))
    {
        reg.ds = content;
    }
    else if (strcasestr(choice, "es"))
    {
        reg.es = content;
    }
    ptrace(PTRACE_SETREGS, child_pid, NULL, &reg);
}
void get_dbg(char *name)
{
    func = malloc(sizeof(struct functions_s) * MAX_FUNCTIONS);
    var = malloc(sizeof(struct variable_s) * MAX_FUNCTIONS);

    Dwarf_Debug dbg = 0;
    int res = DW_DLV_ERROR;
    Dwarf_Error error = 0;
    Dwarf_Half tag = 0;
    const char *tagname = 0;
    Dwarf_Line *line;

    Dwarf_Bool bAttr;
    Dwarf_Attribute attr;
    Dwarf_Unsigned in_line;
    Dwarf_Unsigned in_file = 0;

    Dwarf_Signed num_loc;

    Dwarf_Off ptr_address = 0;
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Unsigned next_cu_header = 0;
    int cu_number = 0;
    Dwarf_Locdesc *loc_list;
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errarg = 0;

    int fd = -1;
    if ((fd = open(name, O_RDONLY, 0666)) < 0)
        printf("Error file\n");

    res = dwarf_init(fd, 0, errhand, errarg, &dbg, &error);
    if (res != DW_DLV_OK)
    {
        printf("Giving up, cannot do DWARF processing\n");
        exit(1);
    }

    Dwarf_Half offset_size = 0;
    Dwarf_Half extension_size = 0;
    Dwarf_Sig8 signature;
    Dwarf_Unsigned typeoffset = 0;
    Dwarf_Half header_cu_type = 0x01;
    Dwarf_Bool is_info = 1;
    for (;; ++cu_number)
    {
        Dwarf_Die no_die = 0;
        Dwarf_Die cu_die = 0;
        memset(&signature, 0, sizeof(signature));

        res = dwarf_next_cu_header_d(dbg, is_info, &cu_header_length,
                                     &version_stamp, &abbrev_offset,
                                     &address_size, &offset_size,
                                     &extension_size, &signature,
                                     &typeoffset, &next_cu_header, &header_cu_type, &error);

        if (res == DW_DLV_ERROR)
        {
            printf("Error in dwarf_next_cu_header\n");
            exit(1);
        }
        if (res == DW_DLV_NO_ENTRY)
        {
            // printf("DONE\n");
            return;
        }

        res = dwarf_siblingof_b(dbg, no_die, is_info,
                                &cu_die, &error);
        if (res == DW_DLV_ERROR)
        {
            printf("Error in dwarf_siblingof on CU die \n");
            exit(1);
        }
        if (res == DW_DLV_NO_ENTRY)
        {
            printf("no entry! in dwarf_siblingof on CU die \n");
            exit(1);
        }

        dwarf_get_info(dbg, cu_die, is_info, 0, name);
    }
}

void dwarf_get_info(Dwarf_Debug dbg, Dwarf_Die in_die,
                    int is_info, int in_level,
                    char *name)
{

    int res = DW_DLV_ERROR;
    Dwarf_Die cur_die = in_die;
    Dwarf_Die child = 0;
    Dwarf_Error error = 0;
    Dwarf_Error *errp = 0;

    get_dwarf(dbg, in_die, in_level, name);
    dienumber++;

    for (;;)
    {
        Dwarf_Die sib_die = 0;
        res = dwarf_child(cur_die, &child, errp);
        if (res == DW_DLV_ERROR)
        {
            printf("Error in dwarf_child , level %d \n", in_level);
            exit(1);
        }
        if (res == DW_DLV_OK)
        {
            dwarf_get_info(dbg, child, is_info,
                           in_level + 1, name);
        }
        res = dwarf_siblingof_b(dbg, cur_die, is_info, &sib_die, errp);
        if (res == DW_DLV_ERROR)
        {
            char *em = errp ? dwarf_errmsg(error) : "Error siblingof_b";
            printf("Error in dwarf_siblingof_b , level %d :%s \n",
                   in_level, em);
            exit(1);
        }
        if (res == DW_DLV_NO_ENTRY)
        {
            break;
        }
        if (cur_die != in_die)
        {
            dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
            cur_die = 0;
        }
        cur_die = sib_die;
        get_dwarf(dbg, cur_die, in_level, name);
        dienumber++;
    }
    return;
}

void get_dwarf(Dwarf_Debug dbg, Dwarf_Die die,
               int level,
               char *name)
{
    Dwarf_Error error = 0;
    Dwarf_Error *errp = 0;

    char *die_name = 0;
    const char *tag_name = 0;
    Dwarf_Error err;
    Dwarf_Half tag;
    Dwarf_Attribute *attrs;
    Dwarf_Addr lowpc, highpc;
    Dwarf_Signed attrcount, i;
    Dwarf_Line *lines;

    Dwarf_Addr lineaddr;
    Dwarf_Unsigned lineno;
    char **filesnames;
    Dwarf_Unsigned countname;
    Dwarf_Unsigned countfiles;
    int rc = dwarf_diename(die, &die_name, &err);

    if (rc == DW_DLV_ERROR)
        printf("Error in dwarf_diename\n");
    else if (rc == DW_DLV_NO_ENTRY)
        return;
    // Get all lines
    int n;
    if (dwarf_srclines(die, &lines, &countfiles, errp) == DW_DLV_OK )
   { dwarf_linesrc(lines[0], &namesrc, 0);
    }
    if (dwarf_tag(die, &tag, &err) != DW_DLV_OK)
        printf("Error in dwarf_tag\n");

    if (dwarf_get_TAG_name(tag, &tag_name) != DW_DLV_OK)
        printf("Error in dwarf_get_TAG_name\n");

    // printf("tag name = %s: '%s'\n", tag_name,die_name);

    if (dwarf_attrlist(die, &attrs, &attrcount, &err) != DW_DLV_OK)
        printf("Error in dwarf_attlist\n");

    Dwarf_Unsigned line = 0;
    Dwarf_Addr adresse;
    for (i = 0; i < attrcount; ++i)
    {

        Dwarf_Half attrcode;

        dwarf_whatattr(attrs[i], &attrcode, &err);
        // printf("Error in dwarf_whatattr\n");
        if (attrcode == DW_AT_low_pc)
            dwarf_formaddr(attrs[i], &lowpc, 0);
        else if (attrcode == DW_AT_high_pc)
            dwarf_formaddr(attrs[i], &highpc, 0);
        else if (attrcode == DW_AT_decl_line)
        {
            dwarf_formudata(attrs[i], &line, 0);

        }

    }

    if (tag == DW_TAG_subprogram)
    {

        func[count_func].name = malloc(sizeof(char) * strlen(die_name));
        strcpy(func[count_func].name, die_name);
        func[count_func].lowpc = lowpc;
        func[count_func].highpc = highpc;
        func[count_func].line = countfiles;

        func[count_func].path = malloc(sizeof(char) * strlen(namesrc));
        strcpy(func[count_func].path, namesrc);
        count_func++;
    }
    if (tag == DW_TAG_variable)
    {

        var[count_var].name = malloc(sizeof(char) * strlen(die_name));
        strcpy(var[count_var].name, die_name);
        var[count_var].lowpc = lowpc;
        var[count_var].highpc = highpc;
        var[count_var].line = line;

        if (count_func == 0)
        {
            var[count_var].funcname = malloc(sizeof(char) * strlen("VAR_GLOBALE"));
            var[count_var].funcname = "VAR_GLOBALE";
        }
        else
        {
            var[count_var].funcname = malloc(sizeof(char) * strlen(func[count_func - 1].name));
            strcpy(var[count_var].funcname, func[count_func - 1].name);
        }
        /*var[count_var].func_var = malloc(sizeof ( struct functions_s));
         var[count_var].func_var->name = func[count_func-1].name;*/
        count_var++;
    }
}

void get_elf(char *name)
{
    void *start = NULL;
    int i, fd;
    struct stat stat;
    char *strtab;
    int nb_symbols;

    // ouverture du fichier (pour être mappé)
    fd = open(name, O_RDONLY, 660);
    if (fd < 0)
        perror("open");

    // récupération de la taille du fichier
    fstat(fd, &stat);

    // projection du fichier (MAP_SHARED importe peu ici)
    start = mmap(0, stat.st_size, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
    if (start == MAP_FAILED)
    {
        perror("mmap");
        abort();
    }

    Elf64_Ehdr *hdr = (Elf64_Ehdr *)start;
    Elf64_Sym *symtab;

    Elf64_Shdr *sections = (Elf64_Shdr *)((char *)start + hdr->e_shoff);

    char **functions = malloc(sizeof(char **) * 100);
    // parcours des sections
    for (i = 0; i < hdr->e_shnum; i++)
    {
        // si la section courante est de type 'table de symbole'
        if (sections[i].sh_type == SHT_SYMTAB)
        {
            symtab = (Elf64_Sym *)((char *)start + sections[i].sh_offset);
            nb_symbols = sections[i].sh_size / sections[i].sh_entsize;
            strtab = (char *)((char *)start + sections[sections[i].sh_link].sh_offset);
        }
    }

    for (i = 0; i < nb_symbols; ++i)
    {
        // printf("Nom : %s types:  %hhu %u %llx\n", strtab + symtab[i].st_name, strtab +symtab[i].st_info,(uint16_t)(strtab +symtab[i].st_shndx),(strtab + symtab[i].st_value));
        if (((unsigned char)(strtab + symtab[i].st_info) == 178))
        {
            func[count_func].name = (strtab + symtab[i].st_name);
            func[count_func].highpc = (strtab + symtab[i].st_value);
            func[count_func].lowpc = (strtab + symtab[i].st_value);

            count_func++;
        }
        if ((unsigned char)(strtab + symtab[i].st_info) == 177)
        {
            var[count_var].name = (strtab + symtab[i].st_name);
            var[count_var].highpc = (strtab + symtab[i].st_value);
            var[count_var].lowpc = (strtab + symtab[i].st_value);
            var[count_var].funcname = "ELF";

            count_var++;
        }
    }
    close(fd);
}