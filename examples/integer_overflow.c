/*
 * Integer Overflow Example
 *
 * This program demonstrates integer overflow bugs where arithmetic
 * operations exceed the maximum value that can be stored in a type,
 * causing unexpected behavior and security vulnerabilities.
 *
 * Bug: Various integer overflow scenarios
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

/*
 * Example 1: Simple signed integer overflow
 */
void signed_overflow() {
    printf("Example 1: Signed integer overflow\n");
    printf("----------------------------------\n");

    int max = INT_MAX;
    printf("INT_MAX = %d\n", max);
    printf("Adding 1 to INT_MAX:\n");

    int result = max + 1;  // BUG: Undefined behavior!
    printf("Result: %d\n", result);
    printf("(Should be negative due to overflow!)\n\n");
}

/*
 * Example 2: Unsigned integer overflow (wraps around)
 */
void unsigned_overflow() {
    printf("Example 2: Unsigned integer overflow\n");
    printf("-----------------------------------\n");

    unsigned int max = UINT_MAX;
    printf("UINT_MAX = %u\n", max);
    printf("Adding 1 to UINT_MAX:\n");

    unsigned int result = max + 1;  // Wraps to 0
    printf("Result: %u\n", result);
    printf("(Wraps around to 0)\n\n");
}

/*
 * Example 3: Overflow in allocation size calculation
 * This is a serious security vulnerability!
 */
char* vulnerable_allocation(int count, int size) {
    printf("Example 3: Allocation size overflow\n");
    printf("-----------------------------------\n");
    printf("Allocating %d items of %d bytes each\n", count, size);
    printf("Total size: %d * %d = %d\n", count, size, count * size);

    // BUG: count * size can overflow!
    size_t total = count * size;

    printf("Allocating %zu bytes\n", total);

    // If overflow occurred, total will be much smaller than expected
    char *buffer = malloc(total);
    if (buffer) {
        printf("Allocation succeeded\n");
        // This might succeed but allocate too little memory
        // Then we write past the end of the buffer!
    }

    return buffer;
}

/*
 * Example 4: Integer overflow in loop counter
 */
void loop_counter_overflow() {
    printf("\nExample 4: Loop counter overflow\n");
    printf("-------------------------------\n");

    // Start close to maximum value
    unsigned char counter = 250;

    printf("Starting counter at: %u\n", counter);
    printf("Loop condition: counter < 10\n");

    int iterations = 0;
    while (counter < 10) {
        printf("Iteration %d: counter = %u\n", iterations, counter);
        counter++;  // BUG: Wraps from 255 to 0
        iterations++;

        if (iterations > 20) {
            printf("Emergency break after %d iterations\n", iterations);
            break;
        }
    }

    printf("Total iterations: %d\n", iterations);
    printf("(Expected: 0, Got: %d due to overflow!)\n\n", iterations);
}

/*
 * Example 5: Overflow in buffer size check
 */
void unsafe_buffer_copy(int src_len, int dst_len) {
    printf("Example 5: Unsafe buffer copy\n");
    printf("-----------------------------\n");
    printf("Source length: %d\n", src_len);
    printf("Destination buffer size: %d\n", dst_len);

    // BUG: If src_len is negative (or very large), this check passes
    // but the copy is still unsafe
    if (src_len > dst_len) {
        printf("Error: Source too large!\n");
        return;
    }

    char *src = malloc(src_len);
    char *dst = malloc(dst_len);

    if (src && dst) {
        printf("Copy would proceed (but might overflow if src_len overflowed)\n");
        // memcpy(dst, src, src_len);  // DANGEROUS!
    }

    free(src);
    free(dst);
}

/*
 * Example 6: Signed/unsigned comparison bug
 */
void signed_unsigned_bug() {
    printf("\nExample 6: Signed/unsigned comparison\n");
    printf("------------------------------------\n");

    int signed_val = -1;
    unsigned int unsigned_val = 10;

    printf("Signed value: %d\n", signed_val);
    printf("Unsigned value: %u\n", unsigned_val);
    printf("Comparing: %d < %u\n", signed_val, unsigned_val);

    // BUG: -1 is converted to UINT_MAX, which is > 10
    if (signed_val < unsigned_val) {
        printf("Result: %d is LESS than %u\n", signed_val, unsigned_val);
    } else {
        printf("Result: %d is GREATER than %u (WRONG!)\n", signed_val, unsigned_val);
    }

    printf("\nThis happens because -1 is converted to %u\n", (unsigned int)-1);
}

/*
 * Example 7: Overflow in arithmetic causing wrong logic
 */
int calculate_average(int a, int b) {
    printf("\nExample 7: Overflow in average calculation\n");
    printf("-----------------------------------------\n");
    printf("Calculating average of %d and %d\n", a, b);

    // BUG: (a + b) can overflow!
    int sum = a + b;
    int avg = sum / 2;

    printf("Sum: %d\n", sum);
    printf("Average: %d\n", avg);
    printf("Correct average should be: %d\n", (a / 2) + (b / 2) + ((a % 2 + b % 2) / 2));

    return avg;
}

/*
 * Example 8: Overflow in pointer arithmetic
 */
void pointer_arithmetic_overflow() {
    printf("\nExample 8: Pointer arithmetic overflow\n");
    printf("-------------------------------------\n");

    int arr[10];
    int *ptr = arr;

    printf("Array address: %p\n", (void*)ptr);
    printf("Adding SIZE_MAX to pointer...\n");

    // BUG: This can wrap around
    ptr = ptr + SIZE_MAX;
    printf("New pointer address: %p\n", (void*)ptr);
    printf("(Should wrap around!)\n");
}

/*
 * Example 9: Real-world bug: malloc parameter overflow
 */
struct header {
    int length;
    char data[1];  // Flexible array member (old style)
};

void* allocate_struct(int data_length) {
    printf("\nExample 9: Struct allocation overflow\n");
    printf("------------------------------------\n");
    printf("Data length: %d\n", data_length);

    // BUG: sizeof(struct header) + data_length can overflow
    size_t total_size = sizeof(struct header) + data_length;

    printf("Total size: %zu\n", total_size);

    // If data_length is very large, total_size wraps to small value
    // malloc succeeds but we allocate too little memory
    struct header *h = malloc(total_size);

    if (h) {
        h->length = data_length;
        printf("Allocated struct with %d bytes of data\n", data_length);
        // Writing to h->data[data_length-1] would overflow!
    }

    return h;
}

int main() {
    printf("=== Integer Overflow Examples ===\n\n");

    signed_overflow();
    unsigned_overflow();

    char *buf = vulnerable_allocation(0x10000000, 0x10);
    if (buf) free(buf);

    loop_counter_overflow();
    unsafe_buffer_copy(-1, 10);
    signed_unsigned_bug();
    calculate_average(INT_MAX, INT_MAX);
    pointer_arithmetic_overflow();

    struct header *h = allocate_struct(0xFFFFFFFF);
    if (h) free(h);

    printf("\n=== Summary ===\n");
    printf("\nInteger overflows are dangerous because:\n");
    printf("1. They can cause buffer overflows (allocation size wraps)\n");
    printf("2. They can bypass security checks (negative becomes huge positive)\n");
    printf("3. They can cause logic errors (counters wrap around)\n");
    printf("4. Signed overflow is undefined behavior in C\n");
    printf("\nPrevention:\n");
    printf("- Use size_t for sizes and counts\n");
    printf("- Check for overflow before operations\n");
    printf("- Use safer alternatives (calloc instead of malloc*size)\n");
    printf("- Enable compiler warnings (-Wall -Wextra)\n");
    printf("- Use sanitizers (fsanitize=undefined)\n");

    return 0;
}
