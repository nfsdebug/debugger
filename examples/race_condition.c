/*
 * Race Condition Example
 *
 * This program demonstrates a race condition where two threads
 * access shared data without proper synchronization, leading to
 * inconsistent results.
 *
 * Bug: Multiple threads incrementing a shared counter without mutex
 *
 * Note: This requires pthread library
 * Compile with: gcc -pthread -g race_condition.c -o race_condition
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 10
#define INCREMENTS_PER_THREAD 100000

// Shared global variable
int shared_counter = 0;

// Mutex for synchronization (unused in buggy version)
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Function that each thread executes
 * Bug: Increments shared_counter without locking
 */
void* increment_counter(void *arg) {
    int thread_id = *(int*)arg;

    printf("Thread %d starting\n", thread_id);

    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        // BUG: This is not atomic!
        // The increment operation is actually three steps:
        // 1. Read shared_counter into register
        // 2. Add 1 to register
        // 3. Write register back to shared_counter
        // Multiple threads can interleave these steps
        shared_counter++;  // RACE CONDITION HERE
    }

    printf("Thread %d finished\n", thread_id);
    return NULL;
}

/*
 * Fixed version with mutex
 */
void* increment_counter_fixed(void *arg) {
    (void)arg;  // Unused parameter

    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        // Fixed: Use mutex to make increment atomic
        pthread_mutex_lock(&mutex);
        shared_counter++;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/*
 * Another race condition example: check-then-act
 */
int bank_balance = 1000;

void* withdraw(void *amount_ptr) {
    int amount = *(int*)amount_ptr;

    // BUG: Check-then-act race condition
    if (bank_balance >= amount) {
        // Simulate some delay
        usleep(1);  // 1 microsecond

        // Another thread might have changed balance here!
        bank_balance -= amount;
        printf("Withdrew %d, new balance: %d\n", amount, bank_balance);
    } else {
        printf("Insufficient funds for %d\n", amount);
    }

    return NULL;
}

/*
 * Example with shared data structure
 */
typedef struct {
    int value;
    int ready;
} SharedData;

SharedData shared_data = {0, 0};

void* producer(void *arg) {
    (void)arg;  // Unused parameter

    for (int i = 0; i < 5; i++) {
        shared_data.value = i * 100;
        // BUG: No memory barrier or synchronization
        shared_data.ready = 1;

        printf("Producer: wrote value %d\n", shared_data.value);
        usleep(100);
    }
    return NULL;
}

void* consumer(void *arg) {
    (void)arg;  // Unused parameter

    for (int i = 0; i < 5; i++) {
        while (!shared_data.ready) {
            // Busy wait
        }

        // BUG: Might see stale data due to lack of synchronization
        printf("Consumer: read value %d\n", shared_data.value);
        shared_data.ready = 0;
        usleep(100);
    }
    return NULL;
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    printf("=== Race Condition Example ===\n\n");

    // Example 1: Simple counter race
    printf("Example 1: Counter race condition\n");
    printf("Expected result: %d\n", NUM_THREADS * INCREMENTS_PER_THREAD);

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    shared_counter = 0;

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, increment_counter, &thread_ids[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // Wait for threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Actual result: %d\n", shared_counter);
    printf("Lost increments: %d\n\n",
           (NUM_THREADS * INCREMENTS_PER_THREAD) - shared_counter);

    // Example 2: Bank account race
    printf("Example 2: Check-then-act race condition\n");
    printf("----------------------------------------\n");

    bank_balance = 1000;
    pthread_t t1, t2;
    int amount1 = 600;
    int amount2 = 600;

    printf("Initial balance: %d\n", bank_balance);
    printf("Attempting to withdraw %d and %d simultaneously\n", amount1, amount2);

    pthread_create(&t1, NULL, withdraw, &amount1);
    pthread_create(&t2, NULL, withdraw, &amount2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final balance: %d\n", bank_balance);
    printf("(Should be -200 or error, but race condition allows it)\n\n");

    // Example 3: Producer-consumer
    printf("Example 3: Producer-consumer race\n");
    printf("--------------------------------\n");

    shared_data.value = 0;
    shared_data.ready = 0;
    pthread_t prod, cons;

    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    printf("\nProgram completed!\n");
    printf("\nNote: Run multiple times to see different results!\n");
    printf("Race conditions are non-deterministic.\n");

    return 0;
}
