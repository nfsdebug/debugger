#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv){

	int a = 0 ; 
	if (argc == 2){
		a = atoi(argv[1]) ; 
	}
	
	printf("La valeur est %d\n", a) ;
	// we want to wrinte a file fith argv args for parsing purpose 
	
	FILE *fp = fopen("parsing_test.txt", "w+" ) ;
	if (!fp){
		printf("cant work on this file...\n") ; 
	}
	else{
		fprintf(fp,"%d",  a) ; 
		fprintf(fp, "\n") ; 
	}
	
	fclose(fp) ; 
	
	return 1 ; 
}
