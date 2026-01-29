/*
 * Infinite Loop Example
 *
 * This program demonstrates various types of infinite loops that
 * can cause programs to hang. These are particularly difficult to
 * debug because the program doesn't crash - it just stops responding.
 *
 * Bug: Various loop conditions that never become false
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

// Flag for controlled termination
volatile bool running = true;

/*
 * Example 1: Classic infinite loop with wrong condition
 */
void wrong_condition_loop() {
    printf("Example 1: Wrong loop condition\n");
    printf("Expected: Count from 0 to 9\n");
    printf("Actually: Never reaches 10\n\n");

    int i = 0;
    while (i < 10) {
        printf("Count: %d\n", i);

        // BUG: Should be i++, but we're decrementing
        // or doing nothing to progress toward the exit condition
        i = i - 1;  // This makes i more negative!

        if (i < -50) {
            printf("Emergency break to prevent actual infinite loop\n");
            break;
        }
    }
}

/*
 * Example 2: Off-by-one error in loop
 */
void off_by_one_loop() {
    printf("Example 2: Off-by-one error\n");
    printf("Expected: Process array of 10 elements\n");
    printf("Actually: Index never reaches array size\n\n");

    int array[10];
    for (int i = 0; i <= 10; i++) {
        // BUG: Should be i < 10, not i <= 10
        // This is buffer overflow, but also shows how we skip index 10
        if (i < 10) {
            array[i] = i * 2;
            printf("array[%d] = %d\n", i, array[i]);
        } else {
            printf("Writing past array bounds!\n");
        }
    }
}

/*
 * Example 3: Waiting for condition that never happens
 */
void waiting_forever() {
    printf("Example 3: Waiting for impossible condition\n");
    printf("Expected: Wait for data to be ready\n");
    printf("Actually: Data never becomes ready\n\n");

    int data_ready = 0;
    int timeout = 0;

    // Simulate waiting for data
    while (!data_ready) {
        printf("Waiting for data... (%d)\n", timeout);

        // BUG: No code here ever sets data_ready = 1
        // In real code, this might be waiting for a signal,
        // callback, or another thread that never happens

        timeout++;
        if (timeout > 5) {
            printf("Timeout! Breaking loop\n");
            break;
        }

        sleep(1);
    }
}

/*
 * Example 4: Mutual deadlock with threads (simplified)
 */
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_func(void *arg __attribute__((unused))) {
    printf("Thread 1: Trying to lock mutex1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: Locked mutex1\n");

    sleep(1);

    printf("Thread 1: Trying to lock mutex2...\n");
    pthread_mutex_lock(&mutex2);  // Deadlock here
    printf("Thread 1: Locked mutex2\n");

    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

void* thread2_func(void *arg __attribute__((unused))) {
    printf("Thread 2: Trying to lock mutex2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: Locked mutex2\n");

    sleep(1);

    printf("Thread 2: Trying to lock mutex1...\n");
    pthread_mutex_lock(&mutex1);  // Deadlock here
    printf("Thread 2: Locked mutex1\n");

    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);

    return NULL;
}

void deadlock_example() {
    printf("Example 4: Deadlock between threads\n");
    printf("------------------------------------\n");
    printf("This would actually hang forever, so we skip it.\n");
    printf("In a real scenario, both threads would be stuck waiting.\n\n");

    // Uncomment to actually see the deadlock:
    /*
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread1_func, NULL);
    pthread_create(&t2, NULL, thread2_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    */
}

/*
 * Example 5: Flag not being updated
 */
volatile bool flag = false;

void* flag_setter_thread(void *arg __attribute__((unused))) {
    printf("Flag setter: Sleeping before setting flag...\n");
    sleep(2);
    printf("Flag setter: Setting flag to true\n");
    flag = true;
    return NULL;
}

void flag_waiting_loop() {
    printf("Example 5: Waiting for flag (with timeout)\n");
    printf("------------------------------------------\n");

    flag = false;
    pthread_t setter_thread;
    pthread_create(&setter_thread, NULL, flag_setter_thread, NULL);

    int count = 0;
    while (!flag) {
        printf("Waiting... (%d)\n", count);
        count++;

        if (count > 5) {
            printf("Timeout! Breaking loop\n");
            break;
        }

        sleep(1);
    }

    if (flag) {
        printf("Flag was set! Exiting loop.\n");
    }

    pthread_join(setter_thread, NULL);
}

/*
 * Example 6: Infinite recursion (stack overflow)
 */
int infinite_recursion(int n) {
    printf("Recursion depth: %d\n", n);

    // BUG: No base case, or base case that's never reached
    // This will eventually cause stack overflow
    if (n > 1000) {
        printf("Stopping recursion to prevent crash\n");
        return n;
    }

    return infinite_recursion(n + 1) + 1;
}

/*
 * Example 7: Loop that modifies wrong variable
 */
void wrong_variable_loop() {
    printf("\nExample 7: Modifying wrong variable\n");
    printf("-----------------------------------\n");

    int count = 0;
    int max = 10;

    while (count < max) {
        printf("Count: %d, Max: %d\n", count, max);

        // BUG: Should increment count, but increments max instead
        max++;  // This makes the condition further from true!

        if (max > 20) {
            printf("Emergency break\n");
            break;
        }

        sleep(1);
    }
}

int main() {
    printf("=== Infinite Loop Examples ===\n\n");

    wrong_condition_loop();
    printf("\n");

    off_by_one_loop();
    printf("\n");

    waiting_forever();
    printf("\n");

    deadlock_example();
    printf("\n");

    flag_waiting_loop();
    printf("\n");

    printf("Example 6: Infinite recursion\n");
    printf("------------------------------\n");
    infinite_recursion(0);
    printf("\n");

    wrong_variable_loop();

    printf("\n=== All examples completed ===\n");
    printf("\nDebugging tips for infinite loops:\n");
    printf("1. Use Ctrl-C to interrupt and check where you are\n");
    printf("2. Set breakpoints at loop conditions\n");
    printf("3. Watch variables that control loop exit\n");
    printf("4. Use 'info threads' to check all thread states\n");
    printf("5. Look for deadlocks with backtrace of all threads\n");

    return 0;
}
