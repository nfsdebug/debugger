/**
 * @file test_complex.c
 * @brief Complex test program with recursion, structs, pointers, and memory allocation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables
int global_counter = 0;
static int static_counter = 0;

// Struct definitions
typedef struct {
    int x;
    int y;
    char name[32];
} Point;

typedef struct {
    Point* points;
    int count;
    int capacity;
} PointList;

// Recursive function - factorial
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// Recursive function - Fibonacci
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Recursive function with deep call stack
int deep_recursion(int depth) {
    if (depth <= 0) {
        return 0;
    }
    global_counter++;
    static_counter++;
    return depth + deep_recursion(depth - 1);
}

// Function with pointer parameters
void swap_ints(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function working with structs
Point create_point(int x, int y, const char* name) {
    Point p;
    p.x = x;
    p.y = y;
    strncpy(p.name, name, sizeof(p.name) - 1);
    p.name[sizeof(p.name) - 1] = '\0';
    return p;
}

// Function with struct pointer
void move_point(Point* p, int dx, int dy) {
    if (p != NULL) {
        p->x += dx;
        p->y += dy;
    }
}

// Dynamic memory allocation
PointList* create_point_list(int capacity) {
    PointList* list = (PointList*)malloc(sizeof(PointList));
    if (list == NULL) {
        return NULL;
    }

    list->points = (Point*)malloc(sizeof(Point) * capacity);
    if (list->points == NULL) {
        free(list);
        return NULL;
    }

    list->count = 0;
    list->capacity = capacity;
    return list;
}

void add_point(PointList* list, Point p) {
    if (list != NULL && list->count < list->capacity) {
        list->points[list->count] = p;
        list->count++;
    }
}

void free_point_list(PointList* list) {
    if (list != NULL) {
        free(list->points);
        free(list);
    }
}

// Function pointer usage
typedef int (*OperationFunc)(int, int);

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

int apply_operation(OperationFunc op, int a, int b) {
    return op(a, b);
}

// Complex nested calls
int level3(int x) {
    return x * 3;
}

int level2(int x) {
    return level3(x * 2);
}

int level1(int x) {
    return level2(x + 1);
}

// Pointer arithmetic
void pointer_arithmetic_test() {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int* ptr = arr;

    for (int i = 0; i < 10; i++) {
        printf("arr[%d] = %d, *(ptr+%d) = %d\n", i, arr[i], i, *(ptr + i));
    }

    ptr = &arr[5];
    printf("*ptr = %d, *(ptr-2) = %d, *(ptr+2) = %d\n",
           *ptr, *(ptr - 2), *(ptr + 2));
}

// Double pointer test
void double_pointer_test() {
    int value = 42;
    int* ptr = &value;
    int** ptr_to_ptr = &ptr;

    printf("value = %d\n", value);
    printf("*ptr = %d\n", *ptr);
    printf("**ptr_to_ptr = %d\n", **ptr_to_ptr);

    **ptr_to_ptr = 100;
    printf("After modification: value = %d\n", value);
}

int main() {
    printf("=== Starting Complex Test Program ===\n\n");

    // Test 1: Recursive factorial
    printf("Test 1: Recursive factorial\n");
    int fact_result = factorial(5);
    printf("factorial(5) = %d\n", fact_result);
    printf("global_counter = %d, static_counter = %d\n\n", global_counter, static_counter);

    // Test 2: Recursive Fibonacci
    printf("Test 2: Recursive Fibonacci\n");
    int fib_result = fibonacci(10);
    printf("fibonacci(10) = %d\n\n", fib_result);

    // Test 3: Deep recursion
    printf("Test 3: Deep recursion (depth=50)\n");
    int deep_result = deep_recursion(50);
    printf("deep_recursion(50) = %d\n", deep_result);
    printf("global_counter = %d, static_counter = %d\n\n", global_counter, static_counter);

    // Test 4: Pointer swap
    printf("Test 4: Pointer swap\n");
    int a = 10, b = 20;
    printf("Before swap: a=%d, b=%d\n", a, b);
    swap_ints(&a, &b);
    printf("After swap: a=%d, b=%d\n\n", a, b);

    // Test 5: Struct operations
    printf("Test 5: Struct operations\n");
    Point p1 = create_point(10, 20, "origin");
    printf("Point p1: x=%d, y=%d, name=%s\n", p1.x, p1.y, p1.name);
    move_point(&p1, 5, -3);
    printf("After move: x=%d, y=%d\n\n", p1.x, p1.y);

    // Test 6: Dynamic memory allocation
    printf("Test 6: Dynamic memory allocation\n");
    PointList* list = create_point_list(5);
    if (list != NULL) {
        add_point(list, create_point(0, 0, "zero"));
        add_point(list, create_point(1, 1, "one"));
        add_point(list, create_point(2, 4, "two"));

        for (int i = 0; i < list->count; i++) {
            printf("  Point %d: (%d, %d) %s\n",
                   i, list->points[i].x, list->points[i].y, list->points[i].name);
        }
        free_point_list(list);
        printf("List freed\n\n");
    }

    // Test 7: Function pointers
    printf("Test 7: Function pointers\n");
    OperationFunc op = add;
    int result1 = apply_operation(op, 5, 3);
    printf("add(5, 3) = %d\n", result1);

    op = multiply;
    int result2 = apply_operation(op, 5, 3);
    printf("multiply(5, 3) = %d\n\n", result2);

    // Test 8: Nested function calls
    printf("Test 8: Nested function calls\n");
    int nested_result = level1(10);
    printf("level1(level2(level3(10))) result = %d\n\n", nested_result);

    // Test 9: Pointer arithmetic
    printf("Test 9: Pointer arithmetic\n");
    pointer_arithmetic_test();
    printf("\n");

    // Test 10: Double pointers
    printf("Test 10: Double pointers\n");
    double_pointer_test();
    printf("\n");

    printf("=== Complex Test Program Complete ===\n");
    return 0;
}
