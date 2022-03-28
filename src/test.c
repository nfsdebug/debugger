#include <signal.h>
#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <immintrin.h>

#include <sys/types.h>

int globalvariable;

void foo(int b);


void sigh(int sig)
{

	//printf("J'ai planté :'(\n");

	void * addrs[64];
	int cnt = backtrace((void**)&addrs, 64);

	int i;

	for(i = 0; i < cnt; i++)
	{
		//printf("%d == %p\n", i, addrs[i]);
	}

       //backtrace_symbols_fd(addrs, cnt, STDOUT_FILENO);
       //libunwind

	exit(1);
}

void funcdetest2(){
	int a = 0 ; 
	long long *b = malloc(512 * sizeof(long long)) ; 
	int64_t  *d = malloc(512 * sizeof(int64_t)) ;	
	float *c = malloc(512 * sizeof(float)) ; 	
	for (int i = 0 ; i < 512 ; i++){
		b[i] = -1212121212 ; 
		c[i] = 1.11111111 ; 
		d[i] = 123123 ; 
	}
	__m256 ra, rb, rc ; 
	ra = _mm256_load_ps(&c[0]) ;
	rb = _mm256_load_ps(&c[0]) ; 
	rc = _mm256_load_ps(&c[0]) ;
	__m256i rd, re, rf ; 
	rd = _mm256_load_si256(&d[0]) ;
	re = _mm256_load_si256(&d[0]) ; 
	rf = _mm256_load_si256(&d[0]) ;
	//free(b) ; 
	int aa = 0 ; 
	int bb = 0 ; 
	int cc = aa / bb ; 
	kill(getpid(),SIGSEGV);
}

void funcdetest()
{
	printf("salut je suis ici \n");
	//printf(" puis encore ici ?\n");
	funcdetest2() ; 
	//printf("salut je suis ici \n");
	//printf(" puis encore ici ?\n");	
}


int main(int argc, char ** argv)
{
	//printf("Début\n");

	//printf("1\n");

	//printf("2\n");	
	int var = 3;

	funcdetest();

	//printf("en dehors\n");

	//printf("Ok\n");

	raise(SIGSEGV);
	
	signal(SIGSEGV, sigh);

	char * buff = malloc(512);



	return 0;
}