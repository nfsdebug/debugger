#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>





void f1(void){
 
	long long *b = malloc(512 * sizeof(long long)) ; 
	int64_t  *d = malloc(512 * sizeof(int64_t)) ;	
	float *c = malloc(512 * sizeof(float)) ; 	
	float *e = malloc(512 * sizeof(float)) ; 	    
	for (int i = 0 ; i < 512 ; i++){
		b[i] = -1212121212 ; 
		c[i] = 1.11111111 ; 
		d[i] = 123123 ;
        e[i] = -1.11111111 ;  
	}
	__m256 ra, rb, rc, rd, re, rf, rg, rh ; 
	ra = _mm256_load_ps(&c[0]) ;
	rb = _mm256_load_ps(&c[0]) ; 
	rc = _mm256_load_ps(&c[0]) ;
	rd = _mm256_load_ps(&c[0]) ;
	re = _mm256_load_ps(&c[0]) ; 
	rf = _mm256_load_ps(&c[0]) ;
	rg = _mm256_load_ps(&c[0]) ; 
	rh = _mm256_load_ps(&c[0]) ;    

	int aa = 0 ; 
	int bb = 0 ; 
	int cc = aa / bb ; 

    printf("Bonjour Monsieur le Professeur !!!!") ;
    char *mdp[] = {"my","password", "abcdef" , (char*)NULL}; 

    
    //c = a / b ; 

}

void f2(void){
    f1() ; 
}

void f3(void);
void f4(void){
    f3() ; 
}

void f3(void){
    f2() ; 
}


void f5(void){
    f4() ; 
}


int main(int argc, char **argv){
    f5();

}