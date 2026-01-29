/*
 * Use-After-Free Example
 *
 * This program demonstrates a use-after-free vulnerability where
 * memory is accessed after it has been freed. This can lead to
 * crashes, data corruption, or security vulnerabilities.
 *
 * Bug: Continuing to use a pointer after freeing it
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char *data;
    int value;
} DataBlock;

/*
 * Function that creates a data block
 */
DataBlock* create_block(int id, const char *data) {
    DataBlock *block = malloc(sizeof(DataBlock));
    if (!block) {
        return NULL;
    }

    block->id = id;
    block->data = strdup(data);  // Allocate and copy string
    block->value = 0;

    return block;
}

/*
 * Function that destroys a data block
 */
void destroy_block(DataBlock *block) {
    if (block) {
        free(block->data);
        free(block);
    }
}

/*
 * Function that processes a block
 * Bug: Uses block after it's been freed
 */
void process_block_with_bug() {
    DataBlock *block = create_block(1, "Important data");
    if (!block) {
        return;
    }

    printf("Block created: id=%d, data=%s\n", block->id, block->data);

    // Process the block
    block->value = 42;
    printf("Block value set to: %d\n", block->value);

    // Save the data pointer for later use
    char *saved_data = block->data;

    // Free the block
    destroy_block(block);

    // BUG: Try to use the saved data pointer
    // This is use-after-free because the data was freed with destroy_block
    printf("Trying to access saved data: %s\n", saved_data);  // DANGEROUS!

    // Even worse - use the freed block pointer
    printf("Trying to access block: id=%d\n", block->id);  // CRASH HERE!
}

/*
 * Another example: dangling pointer in array
 */
void dangling_pointer_array() {
    int *array[5];

    // Allocate and fill array
    for (int i = 0; i < 5; i++) {
        array[i] = malloc(sizeof(int));
        if (array[i]) {
            *(array[i]) = i * 10;
        }
    }

    // Free some elements
    for (int i = 0; i < 3; i++) {
        free(array[i]);
        array[i] = NULL;  // Good practice, but often forgotten
    }

    // BUG: Try to use freed element
    printf("Array[1] value: %d\n", *array[1]);  // DANGEROUS!
}

/*
 * Example with double-free potential
 */
void double_free_example() {
    int *ptr = malloc(sizeof(int));
    if (!ptr) {
        return;
    }

    *ptr = 123;
    printf("Value: %d\n", *ptr);

    free(ptr);
    ptr = NULL;  // Good practice

    // This would be a double-free if we didn't set ptr to NULL
    // free(ptr);  // This is safe because ptr is NULL
}

int main() {
    printf("=== Use-After-Free Example ===\n\n");

    printf("Example 1: Basic use-after-free\n");
    printf("--------------------------------\n");
    process_block_with_bug();

    printf("\nExample 2: Dangling pointer in array\n");
    printf("------------------------------------\n");
    dangling_pointer_array();

    printf("\nExample 3: Proper handling (no crash)\n");
    printf("-------------------------------------\n");
    double_free_example();

    printf("\nProgram completed!\n");

    return 0;
}
