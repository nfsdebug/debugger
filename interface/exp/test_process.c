
#include <stdio.h>
#include <stdlib.h>
#include "../ext/vec/src/vec.h"


struct mystruct{
    double a  ; 
    double b ; 
};

struct process_t{
    // state = 0 for killed , state = 1 for active, state = 2 for inactive
    // this cnvention may change
    int status ;
    int pid ; 
    int ppid ; 
    int gid   ;  
    int num_threads ;   // number of threads in the process
    int memory ;           // memory used by the process
};

int main(int argc, char ** argv){

    printf("process example \n");

    // declare vec
    double var = 1.0 ;
    vec_t* vec = vec_with_value(&var, 10, sizeof(double)) ; 
    printf("Affichage par macro") ; 
    //VEC_PRINT(vec, double) ; 
    double res = 0.0 ; 
    res = ((double*)vec->data)[0]; 
    printf("valer de res : %f\n", res);
 
    // init struct vec
    unsigned long long len = 10000000 ;
    vec_t* vec_struct = vec_with_capacity(len, sizeof(struct mystruct));
    for (unsigned long long i = 0 ; i < vec_struct->capacity ; i++){
        ((struct mystruct*)vec_struct->data)[i].a = 3.0 * (double)i ; 
        ((struct mystruct*)vec_struct->data)[i].b = 4.0 * (double)i;  
        vec_struct->len++;       
    }
    // affichage 
    for (unsigned long long i = 0 ; i < vec_struct->len ; i++){
        if (((struct mystruct*)vec_struct->data)[i].a != 3.0 * (double)i){
            printf("issue in %llu: diff = %f\n", i, ((struct mystruct*)vec_struct->data)[i].a - (3.0 * (double)i));
        }  
    }    

    // process part. We need to test the push back method
    struct process_t p ;  
    p.pid = 1 ; 
    p.ppid = 2 ; 
    p.gid = 4398 ; 
    p.status = 1 ; 
    p.num_threads = 1 ; 
    p.memory = 3298498 ; 

    struct process_t p2 = {  
        .pid = 2, 
        .ppid = 3, 
        .gid = 4399,
        .status = 0, 
        .num_threads = 2, 
        .memory = 329848
    }; 
    vec_t* vp = vec_with_capacity(1 , sizeof(struct process_t) ) ; 
    vec_push(vp, &p) ;  
    vec_push(vp, &p2) ; 
    for(int i = 0 ; i < vp->len ; i++){
        printf("the %d th pid is %d \n" , i, ((struct process_t*)vp->data)[i].pid);
    }

    vec_push(vp, &p) ;  
    vec_push(vp, &p2) ; 
    for(int i = 0 ; i < vp->len ; i++){
        printf("the %d th pid is %d \n" , i, ((struct process_t*)vp->data)[i].pid);
    }

}


