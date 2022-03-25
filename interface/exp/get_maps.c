
#include <stdio.h>
#include <stdlib.h>
#include <endian.h>



int hex2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

unsigned long long letoull(int *array, int length){
    unsigned long long result = 0 ; 
    unsigned long long multiplicateur = 1 ; 
    for (int i = 0 ; i < length ; i++){
        result = result + multiplicateur * array[i];
        multiplicateur*=16;
        //printf("%d %llu\n", array[i], result);
    }
    return result;
}

int main(int argc, char **argv){

    int size_buff = 1000 ; 
    int number_line = 10 ; 
    int length_char_adress = 12 ; 
    char *buff = malloc( size_buff * sizeof(char) ) ;
    char *char_adress = malloc( length_char_adress * sizeof(char)) ; 
    char *reversed_char_adress = malloc( length_char_adress * sizeof(char)) ;   
    int *array = malloc( length_char_adress * sizeof(int));      
    unsigned long long *adress = malloc(number_line * sizeof(unsigned long long) ) ;

    for (int i = 0 ; i < size_buff ; i++){
        buff[i] = '\0' ; //set null
    }

    FILE *fp = fopen("maps", "r") ; 

    for (int i = 0 ; i < number_line ; i++){
        //fscanf( fp ,  "%s\n" , buff ) ; 
        fgets(buff, size_buff, fp) ; 
        //printf("%s", buff) ;  ; 

        // we want the first unsigned int for each line
        for(int j = 0 ; j < length_char_adress ; j++){
            char_adress[j] = buff[j] ; 
        }

        for (int j = 0 ; j < length_char_adress ; j++){
            array[j] = hex2int(char_adress[ length_char_adress - 1 - j]) ;         
        }
        unsigned long long result = letoull(array, length_char_adress);
        printf("le resultat de ma fonction est %llu\n", result) ; 


    }

    free(buff);
    free(char_adress);
    free(reversed_char_adress);
    free(array);
    free(adress);
    return 1;
}