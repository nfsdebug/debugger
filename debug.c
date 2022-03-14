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

#include "lib/linenoise.h"


pid_t child;
int wait_status;

/*
void completion(const char *buf, linenoiseCompletions *lc) {
    if (buf[0] == 'c') {
        linenoiseAddCompletion(lc,"continue");
    }
}*/

char *hints(const char *buf, int *color, int *bold) {
    if (!strcasecmp(buf,"continue")) {
        printf("\n");

        //Continue the child program
        ptrace(PTRACE_CONT, child, 0, 0);

        //Wait its execution
        waitpid(child, &wait_status, WUNTRACED | WCONTINUED);  

        if (WIFEXITED(wait_status) || WIFSIGNALED(wait_status))
                    exit(0);

        int  signum = WSTOPSIG(wait_status);

        printf("child has stopped with signal %s (%d)\n", strsignal(signum),signum);
        
    }
    return NULL;
}

int main(int argc, char** argv){

    //Name of the program has not been inputed (return -1)
    if ( argc < 2) return printf("You need to put the name of the program to be debugged"), -1;


    child = fork();
    
    //If it's the child, then it's the tracee
    if ( child == 0 )
    {
        //Ptrace to follow the program
        if (ptrace(PTRACE_TRACEME,0,NULL,NULL) < 0) return printf("Error with ptrace, check the manual"),-2;

        //Launch program ( without arg for the test)

        execl(argv[1], argv[1], NULL); 


        
      
    }
    //Etherwise it's the tracee
    else
    {

        char *line;



   // linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);
    linenoiseHistoryLoad("history.txt");


    // Print proc information
                    printf("PID: %d\n",getpid());
                    printf("GID: %d\n",getgid());
                    printf("PPID: %d\n",getppid());
                    printf("Path : %s\n",realpath(argv[1], NULL));

       //Wait for the program (first execution)
       waitpid(child,&wait_status,0);
    while((line = linenoise("NFSD> ")) != NULL) {
        if (line[0] != '\0' && line[0] != '/') {
          //  printf("echo: '%s'\n", line);
            linenoiseHistoryAdd(line);
            linenoiseHistorySave("history.txt");
        } else if (line[0] == '/') {
            printf("Unreconized command: %s\n", line);
        }
        free(line);
    }
    }

}


