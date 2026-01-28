#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv){



    int a  = 0; 
    int b = 0 ; 

    if (argc == 3){
        a = atoi(argv[1]) ; 
        b = atoi(argv[2]) ; 
    }    
    int c = a / b ; 
    printf("%d\n", c);

    return 1 ; 
}