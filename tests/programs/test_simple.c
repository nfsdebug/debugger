/**
 * @file test_simple.c
 * @brief Simple test program for basic debugger tests
 */

#include <stdio.h>

int main() {
    printf("Program started\n");
    int x = 42;
    printf("x = %d\n", x);
    x = 100;
    printf("x = %d\n", x);
    printf("Program ended\n");
    return 0;
}
