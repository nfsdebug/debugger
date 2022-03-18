#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){

    ///////////////////
    //  parametrage d'un cas-test
    ///////////////////

    char *command[] = { "exec", "value", "adress", "reg",   (char *)NULL} ;    
    char input[30] = "exec monprogram 982 4973 -a";
    const char delim[2] = " ";
    printf("Commande : %s\n", input);
    printf("Delimiteur : %s\n", delim);


    /////////////////
    // etape de recuperation de l'input
    /////////////////

    char *token ; 
    char *parsed[10] ; 
    token = strtok(input, delim);
    parsed[0] = malloc( strlen(token) * sizeof(char) );
    strcpy(parsed[0] , token);    
    printf("Le mot suivant est |%s|\n", token);   
    int i = 0 ; 
    while( token != NULL){
        token = strtok(NULL, delim);
        printf("Le mot suivant est |%s|\n", token);  
        if (token != NULL){
            i++;
            parsed[i] = malloc( strlen(token) * sizeof(char) + 1);
            strcpy(parsed[i] , token);
        }
    }

    for (int j = 0 ; j <= i ; j++ ){
        printf("La valeur enregistree est |%s|\n", parsed[j]);
    }




    ///////////////////
    // etape de parsing
    //////////////////
    printf("La premiere commande est |%s|\n", command[0]);


    char tmp1[4] = "exec";
    char tmp2[4] = "exec";

    if (!strcmp(command[0], parsed[0])){
        printf("EXEC\n");
    }
    else{
        printf("NOTHING\n");
    }


    ///////////////////
    // deallocation
    //////////////////
    for (int j = i ; j >= 0 ; j--){
        free(parsed[j]);
    }

}


