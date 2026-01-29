/*
 * Memory Leak Example
 *
 * This program demonstrates a memory leak where dynamically allocated
 * memory is not freed, causing the program to consume more and more
 * memory over time.
 *
 * Bug: Allocated memory in process_data() is never freed
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

/*
 * Function that processes data
 * Bug: Allocates memory but caller is responsible for freeing it
 *      and the caller forgets to free it
 */
char* process_data(const char *input) {
    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        return NULL;
    }

    // Simulate some processing
    snprintf(buffer, BUFFER_SIZE, "Processed: %s", input);

    return buffer;  // Caller must free this!
}

/*
 * Another function with a memory leak
 * Bug: Reallocates in a loop without freeing previous allocation
 */
void grow_buffer_leak() {
    char *buffer = NULL;
    size_t size = 10;

    for (int i = 0; i < 100; i++) {
        // BUG: Old buffer is lost when we realloc without assigning
        // the result back to the same variable
        char *new_buffer = realloc(buffer, size);
        if (!new_buffer) {
            free(buffer);
            return;
        }
        buffer = new_buffer;
        size *= 2;

        // Fill buffer with data
        snprintf(buffer, size - 1, "Iteration %d", i);

        printf("Buffer size: %zu, content: %s\n", size, buffer);

        // BUG: We should free(buffer) here or at the end
        // but in this case we're just leaking
    }

    free(buffer);  // Only frees the last allocation
}

/*
 * Function with hidden leak in error path
 */
int hidden_leak_example() {
    char *temp = malloc(100);
    if (!temp) {
        return -1;
    }

    strcpy(temp, "temporary data");

    // Simulate an error condition
    int error_condition = 1;  // Pretend this is from some check

    if (error_condition) {
        // BUG: Return without freeing temp
        return -1;
    }

    free(temp);
    return 0;
}

int main() {
    printf("=== Memory Leak Example ===\n\n");

    printf("Leak 1: Forgetting to free returned pointer\n");
    for (int i = 0; i < 10; i++) {
        char input[64];
        snprintf(input, sizeof(input), "data_%d", i);

        char *result = process_data(input);
        if (result) {
            printf("%s\n", result);
            // BUG: Forgot to free(result)!
        }
    }

    printf("\nLeak 2: Realloc in loop\n");
    grow_buffer_leak();

    printf("\nLeak 3: Hidden leak in error path\n");
    hidden_leak_example();

    printf("\nProgram completed. Check memory usage with Valgrind!\n");
    printf("Run: valgrind --leak-check=full ./memory_leak_example\n");

    return 0;
}
