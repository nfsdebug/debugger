/**
 * @file test_functions.c
 * @brief Test program with multiple functions for testing breakpoints and step over
 */

#include <stdio.h>

int helper_function(int x) {
    int result = x * 2;
    printf("helper_function(%d) = %d\n", x, result);
    return result;
}

int another_function(int a, int b) {
    int sum = a + b;
    printf("another_function(%d, %d) = %d\n", a, b, sum);
    return sum;
}

int main() {
    printf("Starting test_functions\n");

    int value = 5;
    value = helper_function(value);
    value = another_function(value, 10);

    printf("Final value: %d\n", value);
    printf("Ending test_functions\n");
    return 0;
}
