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



int main(int argc, char** argv){

    //Name of the program has not been inputed (return -1)
    if ( argc < 2) return printf("You need to put the name of the program to be debugged"), -1;

    pid_t child;

    child = fork();
    
    //If it's the child, then it's the tracee
    if ( child == 0 )
    {
        //Ptrace to follow the program
        if (ptrace(PTRACE_TRACEME,0,NULL,NULL) < 0) return printf("Error with ptrace, check the manual"),-2;

        //Launch program ( without arg for the test)

        // TODO : use exevp
        execl(argv[1], argv[1], NULL); 


        
      
    }
    //Etherwise it's the tracee
    else
    {

        char cmd[80];
        int wait_status;



    // Print proc information
                    printf("PID: %d\n",getpid());
                    printf("GID: %d\n",getgid());
                    printf("PPID: %d\n",getppid());
                    printf("Path : %s\n",realpath(argv[1], NULL));

       //Wait for the program (first execution)
       waitpid(child,&wait_status,0);
        scanf("%[^\n]",cmd);

    while (cmd != NULL)
    {


    if (!strcasecmp(cmd,"continue")) {

        //Continue the child program and waiting for sys calls
        ptrace(PTRACE_SYSCALL, child, 0, 0);

        //Wait its execution
        waitpid(child, &wait_status,0);  

        siginfo_t signinf;

        //Sighandler with ptrace
        ptrace(PTRACE_GETSIGINFO,child,NULL,&signinf);

        // If it's not a breakpoint made by ptrace, it's an error from the child
        if(signinf.si_signo != 5)
        {
            printf("\n child stopped : %s (%d) at adress 0x%p\n",strsignal(signinf.si_signo),signinf.si_signo,signinf.si_addr);

            // TODO: toggle when the parser will not do a loop ( to fix)
            break;
        }
        
        scanf("%[^\n]",cmd);
    
    }
  //  return;
    
}
    }
}


