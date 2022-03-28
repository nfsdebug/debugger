#include <stdio.h>
#include <stdlib.h>





void f1(void){
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