#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(int argc, char **argv){



    int a  = 1; 
    int b = 1; 

    if (argc == 3){
        a = atoi(argv[1]) ; 
        b = atoi(argv[2]) ; 
    }    
    int c = (int)sqrt(a) + b ; 
    printf("%d\n", c);

    return 1 ; 
}