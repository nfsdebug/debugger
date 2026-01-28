/**
 * @file test_loops.c
 * @brief Test program with loops for testing continue and step
 */

#include <stdio.h>

int main() {
    printf("Starting test_loops\n");

    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += i;
        printf("i=%d, sum=%d\n", i, sum);
    }

    printf("Final sum: %d\n", sum);

    int count = 0;
    while (count < 3) {
        printf("count=%d\n", count);
        count++;
    }

    printf("Ending test_loops\n");
    return 0;
}
