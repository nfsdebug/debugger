#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


void *add_in_thread(void *args){
    int a = 1  ; 
    int b = 0 ; 
    printf("La valeur de a est : %d\n", a) ; 
    printf("La valeur de b est : %d\n", b) ;     
    int c = a / b ; 
    
    printf("La valeur de c est : %d\n", c) ;  
    pthread_exit(NULL);
}

/*
void *get_values(void *args){
    printf("La valeur de a est : %d\n", a) ; 
    printf("La valeur de b est : %d\n", b) ; 
    pthread_exit(NULL);
}
*/

int main(int argc, char **argv){
    pthread_t thread ; 
    pthread_create(&thread, NULL, add_in_thread, NULL) ; 
    int a = 1  ; 
    int b = 0 ; 
    printf("La valeur de a est : %d\n", a) ; 
    printf("La valeur de b est : %d\n", b) ;     
    int c = a / b ; 
    
    printf("La valeur de c est : %d\n", c) ;     
    pthread_join(thread, NULL);
    //pthread_create(&thread, NULL, add_in_thread, NULL) ; 
    //pthread_join(thread, NULL);

    




}