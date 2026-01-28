/**
 * @file test_globals.c
 * @brief Test program with global variables for testing watchpoints
 */

#include <stdio.h>

volatile int global_counter = 0;
volatile int global_value = 100;
volatile int global_array[5] = {1, 2, 3, 4, 5};

int main() {
    printf("Starting test_globals\n");
    printf("global_counter = %d\n", global_counter);

    for (int i = 0; i < 10; i++) {
        global_counter = i;
        printf("Set global_counter to %d\n", global_counter);
    }

    global_value = 200;
    printf("global_value = %d\n", global_value);

    for (int i = 0; i < 5; i++) {
        global_array[i] = i * 10;
        printf("global_array[%d] = %d\n", i, global_array[i]);
    }

    printf("Ending test_globals\n");
    return 0;
}
