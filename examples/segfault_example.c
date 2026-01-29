/*
 * Segfault Example: Null Pointer Dereference
 *
 * This program demonstrates a classic segmentation fault caused by
 * dereferencing a NULL pointer. This is one of the most common bugs
 * in C programming.
 *
 * Bug: The function tries to access a struct through a NULL pointer
 * without checking if it's valid first.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char name[32];
    float score;
} Student;

/*
 * Function that prints student information
 * Bug: Doesn't check if student pointer is NULL
 */
void print_student(Student *student) {
    printf("Student ID: %d\n", student->id);        // CRASHES HERE if student is NULL
    printf("Name: %s\n", student->name);
    printf("Score: %.2f\n", student->score);
}

/*
 * Function that creates a new student
 * Returns NULL on failure (simulating allocation failure)
 */
Student* create_student(int id, const char *name, float score) {
    Student *s = malloc(sizeof(Student));
    if (!s) {
        return NULL;  // Allocation failed
    }

    s->id = id;
    strncpy(s->name, name, sizeof(s->name) - 1);
    s->name[sizeof(s->name) - 1] = '\0';
    s->score = score;

    return s;
}

int main() {
    printf("=== Segfault Example ===\n\n");

    // This works fine
    printf("Creating student 1...\n");
    Student *s1 = create_student(1, "Alice", 95.5);
    if (s1) {
        print_student(s1);
        free(s1);
    }

    printf("\n");

    // This will cause a segfault
    printf("Creating student 2 (simulating malloc failure)...\n");
    Student *s2 = NULL;  // Simulating allocation failure
    // In real code, this might happen if malloc fails
    // or if a function returns NULL on error

    printf("Attempting to print student 2...\n");
    print_student(s2);  // SEGFAULT HERE!

    printf("This line will never be reached.\n");

    return 0;
}
