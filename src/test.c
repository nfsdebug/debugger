#include <signal.h>
#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>


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
	char *b = malloc(512) ; 
	free(b) ; 
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