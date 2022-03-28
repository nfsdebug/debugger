#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>





void f1(void){

    double *A = malloc(1000 * sizeof(double) ) ; 
    double *B = malloc(1000 * sizeof(double) ) ; 
    double *C = malloc(1000 * sizeof(double) ) ;  

    for (int i = 0 ; i < 1000 ; i++){
        A[i] = 12.343234 ;
        B[i] = 12.343234 ; 
        C[i] = 0.0 ; 
    } 

    __m256d ra, rb, rc, rd, re, rf, rg, rh, ri, rj, rk, rl; 
    for (unsigned long long i = 0 ; i < 62 ; i+=16){
        ra = _mm256_load_pd(&A[i]);
        rb = _mm256_load_pd(&B[i]);
        rd = _mm256_load_pd(&A[i+4]);
        re = _mm256_load_pd(&B[i+4]);
        rg = _mm256_load_pd(&A[i+8]);
        rh = _mm256_load_pd(&B[i+8]);
        rj = _mm256_load_pd(&A[i+12]);
        rk = _mm256_load_pd(&B[i+12]);

        rc = _mm256_add_pd(ra, rb);
        rf = _mm256_add_pd(re, rd); 
        ri = _mm256_add_pd(rg, rh); 
        rl = _mm256_add_pd(rj, rk);       

        _mm256_store_pd(&C[i] , rc) ;
        _mm256_store_pd(&C[i+4] , rf) ;     
        _mm256_store_pd(&C[i] , ri) ;
        _mm256_store_pd(&C[i+4] , rl) ;
    }
    free(A) ; 
    free(B) ; 
    free(C) ; 


    int a = 0 ; 
    int b = 0 ; 
    int c ; 
    c = a / b ; 

}

void f2(void){
    f1() ; 
}

void f3(void){
    f2() ; 
}

void f4(void){
    f3() ; 
}

void f5(void){
    f4() ; 
}


int main(int argc, char **argv){
    f5();

}