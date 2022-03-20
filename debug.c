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
#include <libunwind-ptrace.h>
#include <libunwind.h>

#include "lib/libdwarf.h"
#include "lib/dwarf.h"
#include "lib/utilities.h"

static unw_addr_space_t as;
static struct UPT_info *ui;

int main(int argc, char **argv)
{
    as = unw_create_addr_space(&_UPT_accessors, 0);

    // Name of the program has not been inputed (return -1)
    if (argc < 2)
        return printf("You need to put the name of the program to be debugged"), -1;

    pid_t child;

    child = fork();

    // If it's the child, then it's the tracee
    if (child == 0)
    {
        // Ptrace to follow the program
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
            return printf("Error with ptrace, check the manual"), -2;

        // Launch program ( without arg for the test)

        // TODO : use exevp
        execl(argv[1], argv[1], NULL);
    }
    // Etherwise it's the tracee
    else
    {
        ui = _UPT_create(child);

        char cmd[80];
        int wait_status;

        get_dbg(argv[1]);

        //
        if (count_func == 0)
            get_elf(argv[1]);

        // Get program offset
        long long prog_offset;

        char fname[128];
        sprintf(fname, "/proc/%d/maps", child);
        FILE *f = fopen(fname, "rb");
        fscanf(f, "%llx", &prog_offset);
        // Print proc information
        printf("PID: %d\n", getpid());
        printf("GID: %d\n", getgid());
        printf("PPID: %d\n", getppid());
        printf("Path : %s\n", realpath(argv[1], NULL));
        printf("Offset = %llx\n", prog_offset);
        for (int i = 0; i < count_func; i++)
            printf("Function : %s\n", func[i].name);

        for (int i = 0; i < count_var; i++)
            printf("Variable  : %s dans func: %s\n", var[i].name, var[i].funcname);
        // Wait for the program (first execution)
        waitpid(child, &wait_status, 0);
        scanf("%[^\n]", cmd);

        while (cmd != NULL)
        {
            if (strcasestr(cmd, "continue"))
            {

                // Continue the child program and waiting for sys calls
                ptrace(PTRACE_SINGLESTEP, child, 0, 0);
                waitpid(child, &wait_status, 0);
            }

            // If the user entered "b, break" then he want to setup a breakpoint
            if (strcasestr(cmd, "b"))
            {
                char *string = cmd;
                char *token = strtok(string, " ");
                if (strcasecmp(cmd, "func"))
                {

                    token = strtok(NULL, " ");

                    for (int i = 0; i < count_func; i++)
                    {
                        if (strcasestr(token, func[i].name))
                        {

                            Dwarf_Addr adr = func[i].lowpc + prog_offset;

                            // Add 3 to the adress

                            long long bef = (ptrace(PTRACE_PEEKDATA, child, (void *)adr, 0) & ~0xff) | 0xcc;

                            ptrace(PTRACE_POKEDATA, child, (void *)adr, (void *)bef);
                        }
                    }
                }
                else
                {
                    char *string = cmd;
                    token = strtok(string, " ");
                    token = strtok(NULL, " ");
                    printf("DEBUG:\tSetting a breakpoint on adress %s\n", token);
                    // Add 3 to the adress
                    u_int64_t w3 = (ptrace(PTRACE_PEEKDATA, child, token, 0) & ~0xff) | 0xcc;
                    if (ptrace(PTRACE_POKEDATA, child, token, w3) < 0)
                        printf("Wrong adress for the breakpoint");
                }
            }

            // Simple management of memory
            else if (strcasestr(cmd, "memory"))
            {
                if (strcasestr(cmd, "read"))
                {
                    char *string = cmd;
                    // Extract the first token
                    char *token = strtok(string, " ");
                    // get the adress (second in the string)
                    token = strtok(NULL, " ");
                    token = strtok(NULL, " ");

                    printf("%lx\n", ptrace(PTRACE_PEEKDATA, child, token, NULL));
                }
                else if (strcasestr(cmd, "write"))
                {
                    char *string = cmd;
                    // Extract the first token
                    char *token = strtok(string, " ");
                    // get the adress (second in the string)
                    token = strtok(NULL, " ");
                    char *token2 = strtok(NULL, " ");
                    token = strtok(NULL, " ");

                    ptrace(PTRACE_PEEKDATA, child, token2, token, NULL);
                }
            }
            // Section with registers
            if (strcasestr(cmd, "register"))
            {

                // Shows all adresses of al registers
                if (strcasestr(cmd, "dump"))
                {
                    struct user_regs_struct reg;
                    ptrace(PTRACE_GETREGS, child, NULL, &reg);
                    printf("rax = 0x%llx\nrbx = 0x%llx\nrcx = 0x%llx\nrdx = 0x%llx\nrdi = 0x%llx\nrsi = 0x%llx\nrbp = 0x%llx\nrsp = 0x%llx\nr8 = 0x%llx\nr9 = 0x%llx\nr10 = 0x%llx\nr11 = 0x%llx\nr12 = 0x%llx\nr13 = 0x%llx\nr14 = 0x%llx\nr15 = 0x%llx\nrip = 0x%llx\neflags = 0x%llx\ncs = 0x%llx\norig_rax = 0x%llx\nfs_base = 0x%llx\ngs_base = 0x%llx\nfs = 0x%llx\ngs = 0x%llx\nss = 0x%llx\nds = 0x%llx\nes = 0x%llx\n", reg.rax, reg.rbx, reg.rcx, reg.rdx, reg.rdi, reg.rsi, reg.rbp, reg.rsp, reg.r8, reg.r9, reg.r10, reg.r11, reg.r12, reg.r13, reg.r14, reg.r15, reg.rip, reg.eflags, reg.cs, reg.orig_rax, reg.fs_base, reg.gs_base, reg.fs, reg.gs, reg.ss, reg.ds, reg.es);
                }
                else if (strstr(cmd, "read"))
                {
                    char *string = cmd;
                    // Extract the first token
                    char *token = strtok(string, " ");
                    // get the adress (second in the string)
                    token = strtok(NULL, " ");
                    token = strtok(NULL, " ");

                    struct user_regs_struct reg;
                    ptrace(PTRACE_GETREGS, child, NULL, &reg);

                    if (strcasestr(token, "rax"))
                    {
                        printf("rax adress = 0x%llx\n", reg.rax);
                    }
                    else if (strcasestr(token, "rbx"))
                    {
                        printf("rbx adress = 0x%llx\n", reg.rbx);
                    }
                    else if (strcasestr(token, "rcx"))
                    {
                        printf("rcx adress = 0x%llx\n", reg.rcx);
                    }
                    else if (strcasestr(token, "rdx"))
                    {
                        printf("rdx adress = 0x%llx\n", reg.rdx);
                    }
                    else if (strcasestr(token, "rdi"))
                    {
                        printf("rdi adress = 0x%llx\n", reg.rdi);
                    }
                    else if (strcasestr(token, "rsi"))
                    {
                        printf("rsi adress = 0x%llx\n", reg.rsi);
                    }
                    else if (strcasestr(token, "rbp"))
                    {
                        printf("rbp adress = 0x%llx\n", reg.rbp);
                    }
                    else if (strcasestr(token, "rsp"))
                    {
                        printf("rsp adress = 0x%llx\n", reg.rsp);
                    }
                    else if (strcasestr(token, "r8"))
                    {
                        printf("r8 adress = 0x%llx\n", reg.r8);
                    }
                    else if (strcasestr(token, "r9"))
                    {
                        printf("r9 adress = 0x%llx\n", reg.r9);
                    }
                    else if (strcasestr(token, "r10"))
                    {
                        printf("r10 adress = 0x%llx\n", reg.r10);
                    }
                    else if (strcasestr(token, "r11"))
                    {
                        printf("r11 adress = 0x%llx\n", reg.r11);
                    }
                    else if (strcasestr(token, "r12"))
                    {
                        printf("r12 adress = 0x%llx\n", reg.r12);
                    }
                    else if (strcasestr(token, "r13"))
                    {
                        printf("r13 adress = 0x%llx\n", reg.r13);
                    }
                    else if (strcasestr(token, "r14"))
                    {
                        printf("r14 adress = 0x%llx\n", reg.r14);
                    }
                    else if (strcasestr(token, "r15"))
                    {
                        printf("r15 adress = 0x%llx\n", reg.r15);
                    }
                    else if (strcasestr(token, "rip"))
                    {
                        printf("rip adress = 0x%llx\n", reg.rip);
                    }
                    else if (strcasestr(token, "rdx"))
                    {
                        printf("rdx adress = 0x%llx\n", reg.rdx);
                    }
                    else if (strcasestr(token, "eflags"))
                    {
                        printf("eflags adress = 0x%llx\n", reg.eflags);
                    }
                    else if (strcasestr(token, "cs"))
                    {
                        printf("cs adress = 0x%llx\n", reg.cs);
                    }
                    else if (strcasestr(token, "orig_rax"))
                    {
                        printf("orig_rax adress = 0x%llx\n", reg.orig_rax);
                    }
                    else if (strcasestr(token, "fs_base"))
                    {
                        printf("fs_base adress = 0x%llx\n", reg.fs_base);
                    }
                    else if (strcasestr(token, "gs_base"))
                    {
                        printf("gs_base adress = 0x%llx\n", reg.gs_base);
                    }
                    else if (strcasestr(token, "fs"))
                    {
                        printf("fs adress = 0x%llx\n", reg.fs);
                    }
                    else if (strcasestr(token, "gs"))
                    {
                        printf("gs adress = 0x%llx\n", reg.gs);
                    }
                    else if (strcasestr(token, "ss"))
                    {
                        printf("ss adress = 0x%llx\n", reg.ss);
                    }
                    else if (strcasestr(token, "ds"))
                    {
                        printf("ds adress = 0x%llx\n", reg.ds);
                    }
                    else if (strcasestr(token, "es"))
                    {
                        printf("es adress = 0x%llx\n", reg.es);
                    }
                }
                else if (strcasestr(cmd, "write"))
                {
                    char *string = cmd;
                    // Extract the first token
                    char *token = strtok(string, " ");
                    // get the adress (second in the string)
                    token = strtok(NULL, " ");
                    char *token2 = strtok(NULL, " ");
                    struct user_regs_struct reg;
                    ptrace(PTRACE_GETREGS, child, NULL, &reg);

                    if (strcasestr(token, "rax"))
                    {
                        reg.rax = atoi(token2);
                    }
                    else if (strcasestr(token, "rbx"))
                    {
                        reg.rbx = atoi(token2);
                    }
                    else if (strcasestr(token, "rcx"))
                    {
                        reg.rcx = atoi(token2);
                    }
                    else if (strcasestr(token, "rdx"))
                    {
                        reg.rdx = atoi(token2);
                    }
                    else if (strcasestr(token, "rdi"))
                    {
                        reg.rdi = atoi(token2);
                    }
                    else if (strcasestr(token, "rsi"))
                    {
                        reg.rsi = atoi(token2);
                    }
                    else if (strcasestr(token, "rbp"))
                    {
                        reg.rbp = atoi(token2);
                    }
                    else if (strcasestr(token, "rsp"))
                    {
                        reg.rsp = atoi(token2);
                    }
                    else if (strcasestr(token, "r8"))
                    {
                        reg.r8 = atoi(token2);
                    }
                    else if (strcasestr(token, "r9"))
                    {
                        reg.r9 = atoi(token2);
                    }
                    else if (strcasestr(token, "r10"))
                    {
                        reg.r10 = atoi(token2);
                    }
                    else if (strcasestr(token, "r11"))
                    {
                        reg.r11 = atoi(token2);
                    }
                    else if (strcasestr(token, "r12"))
                    {
                        reg.r12 = atoi(token2);
                    }
                    else if (strcasestr(token, "r13"))
                    {
                        reg.r13 = atoi(token2);
                    }
                    else if (strcasestr(token, "r14"))
                    {
                        reg.r14 = atoi(token2);
                    }
                    else if (strcasestr(token, "r15"))
                    {
                        reg.r15 = atoi(token2);
                    }
                    else if (strcasestr(token, "rip"))
                    {
                        reg.rip = atoi(token2);
                    }
                    else if (strcasestr(token, "rdx"))
                    {
                        reg.rdx = atoi(token2);
                    }
                    else if (strcasestr(token, "eflags"))
                    {
                        reg.eflags = atoi(token2);
                    }
                    else if (strcasestr(token, "cs"))
                    {
                        reg.cs = atoi(token2);
                    }
                    else if (strcasestr(token, "orig_rax"))
                    {
                        reg.orig_rax = atoi(token2);
                    }
                    else if (strcasestr(token, "fs_base"))
                    {
                        reg.fs_base = atoi(token2);
                    }
                    else if (strcasestr(token, "gs_base"))
                    {
                        reg.gs_base = atoi(token2);
                    }
                    else if (strcasestr(token, "fs"))
                    {
                        reg.fs = atoi(token2);
                    }
                    else if (strcasestr(token, "gs"))
                    {
                        reg.gs = atoi(token2);
                    }
                    else if (strcasestr(token, "ss"))
                    {
                        reg.ss = atoi(token2);
                    }
                    else if (strcasestr(token, "ds"))
                    {
                        reg.ds = atoi(token2);
                    }
                    else if (strcasestr(token, "es"))
                    {
                        reg.es = atoi(token2);
                    }
                    ptrace(PTRACE_SETREGS, child, NULL, &reg);
                }
            }
            if (strcasestr(cmd, "backtrace"))
            {
                unw_word_t ip, start_ip = 0, sp, off;
                int n = 0, ret;
                unw_cursor_t c;
                char buf[512];
                size_t len;
                ret = unw_init_remote(&c, as, ui);

                do
                {
                    if ((ret = unw_get_reg(&c, UNW_REG_IP, &ip)) < 0 || (ret = unw_get_reg(&c, UNW_REG_SP, &sp)) < 0)
                        printf("unw_get_reg/unw_get_proc_name() failed: ret=%d\n", ret);

                    if (n == 0)
                        start_ip = ip;

                    buf[0] = '\0';
                    unw_get_proc_name(&c, buf, sizeof(buf), &off);

                    printf("Nom du proc : %s\n", buf);

                    if (off)
                    {
                        len = strlen(buf);
                        if (len >= sizeof(buf) - 32)
                            len = sizeof(buf) - 32;
                        sprintf(buf + len, "+0x%lx", (unsigned long)off);
                    }
                    printf(" - %-32s \n", buf);

                    ret = unw_step(&c);
                    printf("\n");

                } while (ret > 0);
            }
            /*  struct user_regs_struct reg;
              ptrace(PTRACE_GETREGS, child, NULL, &reg);

              printf("rip = %llx , %llx offset = %llx\n", reg.rip,  ptrace(PTRACE_PEEKDATA, child, reg.rip, 0),prog_offset);

              sleep(800000);*/

            if (WIFSTOPPED(wait_status))
            {
                siginfo_t signinf;
                ptrace(PTRACE_GETSIGINFO, child, NULL, &signinf);

                // If it's not a breakpoint made by ptrace, it's an error from the child
                if (signinf.si_signo != 5)
                {
                    printf("\n child stopped : %s (%d) at adress %llx \n", strsignal(signinf.si_signo), signinf.si_signo, (long long unsigned)signinf.si_addr);
                }

                scanf("%[^\n]", cmd);
            }

            //  scanf("%[^\n]",cmd);

            // return;
        }
    }
}
