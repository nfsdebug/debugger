#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){


    int double_free = 0 ; 
    if (argc == 2){
        double_free = atoi(argv[1]) ; 
    }


    int *vec1 = malloc( 1000 * sizeof(int) ); 
    for (int i = 0 ; i < 1000 ; i++){
        vec1[i] = i ; 
        printf("%d", vec1[i]) ; 
    }


    free(vec1) ; 
    if (double_free == 1){
        free(vec1) ; 
    }



    return 1;
}