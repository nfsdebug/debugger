#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){
    char *e[]  = {"./interface", (char*)NULL};

    if (fork() == 0){
       
        //printf("forked terminal\n") ;         
        execvp(e[0], e) ;        

    }
    else{
        //freopen("/dev/pts/1", "a", stdout);
        printf("main terminal\n") ; 
        freopen("/dev/pts/2", "a", stdout);
        freopen("/dev/pts/2", "a", stdin);         
        execvp(e[0], e) ; 
    }
    sleep(10) ;


    return 1 ; 
}