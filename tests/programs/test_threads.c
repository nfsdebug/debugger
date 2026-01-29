/**
 * @file test_threads.c
 * @brief Test program with pthreads for multi-threaded debugger testing
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

// Global variables for thread testing
volatile int global_counter = 0;
volatile int thread_created = 0;
volatile int thread_started = 0;
volatile int thread_finished = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread data structure
typedef struct {
    int thread_id;
    int iterations;
    int delay_ms;
    char name[32];
} ThreadData;

// Worker thread function
void* worker_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    printf("[Thread %d] Starting (name: %s)\n", data->thread_id, data->name);
    thread_started++;

    for (int i = 0; i < data->iterations; i++) {
        // Lock mutex for thread-safe counter update
        pthread_mutex_lock(&counter_mutex);
        global_counter++;
        int current_count = global_counter;
        pthread_mutex_unlock(&counter_mutex);

        printf("[Thread %d] Iteration %d, global_counter=%d\n",
               data->thread_id, i, current_count);

        usleep(data->delay_ms * 1000);
    }

    printf("[Thread %d] Finished\n", data->thread_id);
    thread_finished++;

    // Return result
    int* result = malloc(sizeof(int));
    *result = data->iterations;
    return result;
}

// Thread that computes factorial recursively
int factorial_thread(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial_thread(n - 1);
}

void* factorial_thread_func(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int n = data->thread_id;  // Use thread_id as the factorial number

    printf("[Factorial Thread %d] Computing factorial(%d)\n", data->thread_id, n);
    thread_started++;

    int result = factorial_thread(n);

    printf("[Factorial Thread %d] factorial(%d) = %d\n",
           data->thread_id, n, result);
    thread_finished++;

    int* ret = malloc(sizeof(int));
    *ret = result;
    return ret;
}

// Thread with deep call stack
void level5(int depth) {
    printf("[Thread] Level 5, depth=%d\n", depth);
    usleep(10000);
}

void level4(int depth) {
    printf("[Thread] Level 4, depth=%d\n", depth);
    level5(depth + 1);
}

void level3(int depth) {
    printf("[Thread] Level 3, depth=%d\n", depth);
    level4(depth + 1);
}

void level2(int depth) {
    printf("[Thread] Level 2, depth=%d\n", depth);
    level3(depth + 1);
}

void level1(int depth) {
    printf("[Thread] Level 1, depth=%d\n", depth);
    level2(depth + 1);
}

void* deep_stack_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    printf("[Deep Stack Thread %d] Starting deep call stack\n", data->thread_id);
    thread_started++;

    level1(0);

    printf("[Deep Stack Thread %d] Finished\n", data->thread_id);
    thread_finished++;

    return NULL;
}

// Thread that waits for condition
void* waiting_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    printf("[Waiting Thread %d] Waiting for %d seconds\n",
           data->thread_id, data->delay_ms);
    thread_started++;

    sleep(data->delay_ms);

    printf("[Waiting Thread %d] Done waiting\n", data->thread_id);
    thread_finished++;

    return NULL;
}

// Thread with signal handling
void sigusr1_handler(int sig) {
    (void)sig;
    printf("[Thread] SIGUSR1 received in thread\n");
}

void* signal_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    printf("[Signal Thread %d] Setting up signal handler\n", data->thread_id);
    thread_started++;

    // Setup signal handler for this thread
    signal(SIGUSR1, sigusr1_handler);

    for (int i = 0; i < 5; i++) {
        printf("[Signal Thread %d] Iteration %d\n", data->thread_id, i);
        sleep(1);
    }

    printf("[Signal Thread %d] Finished\n", data->thread_id);
    thread_finished++;

    return NULL;
}

// Thread with mutex contention
void* mutex_contention_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    printf("[Mutex Thread %d] Starting\n", data->thread_id);
    thread_started++;

    for (int i = 0; i < data->iterations; i++) {
        pthread_mutex_lock(&counter_mutex);

        // Critical section
        printf("[Mutex Thread %d] Acquired mutex, iteration %d\n",
               data->thread_id, i);
        global_counter++;

        // Hold mutex for a bit to cause contention
        usleep(50000);  // 50ms

        pthread_mutex_unlock(&counter_mutex);

        usleep(10000);  // 10ms
    }

    printf("[Mutex Thread %d] Finished\n", data->thread_id);
    thread_finished++;

    return NULL;
}

// Thread with local and static variables
void* static_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    static int static_var = 0;
    int local_var = 0;

    printf("[Static Thread %d] Starting\n", data->thread_id);
    thread_started++;

    for (int i = 0; i < data->iterations; i++) {
        static_var++;
        local_var++;
        printf("[Static Thread %d] static=%d, local=%d\n",
               data->thread_id, static_var, local_var);
        usleep(100000);  // 100ms
    }

    printf("[Static Thread %d] Finished (static=%d, local=%d)\n",
           data->thread_id, static_var, local_var);
    thread_finished++;

    return NULL;
}

// Create and run a single thread
pthread_t create_and_run_thread(void* (*func)(void*), ThreadData* data) {
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, func, data);
    if (ret != 0) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(ret));
        return (pthread_t)0;
    }
    thread_created++;
    return thread;
}

// Test basic thread creation and joining
void test_basic_threads() {
    printf("\n=== Test 1: Basic Thread Creation ===\n");
    const int num_threads = 3;
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    // Create threads
    for (int i = 0; i < num_threads; i++) {
        data[i].thread_id = i + 1;
        data[i].iterations = 3;
        data[i].delay_ms = 100;
        snprintf(data[i].name, sizeof(data[i].name), "worker_%d", i + 1);

        threads[i] = create_and_run_thread(worker_thread, &data[i]);
    }

    // Wait for threads to complete
    for (int i = 0; i < num_threads; i++) {
        void* result;
        pthread_join(threads[i], &result);
        if (result != NULL) {
            printf("Thread %d returned %d\n", i + 1, *(int*)result);
            free(result);
        }
    }

    printf("All threads finished. Final global_counter=%d\n", global_counter);
}

// Test threads with deep call stacks
void test_deep_stack_threads() {
    printf("\n=== Test 2: Threads with Deep Call Stacks ===\n");
    const int num_threads = 2;
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    for (int i = 0; i < num_threads; i++) {
        data[i].thread_id = i + 1;
        data[i].iterations = 0;
        data[i].delay_ms = 0;

        threads[i] = create_and_run_thread(deep_stack_thread, &data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Test factorial computation in threads
void test_factorial_threads() {
    printf("\n=== Test 3: Factorial Threads ===\n");
    const int num_threads = 5;
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    for (int i = 0; i < num_threads; i++) {
        data[i].thread_id = i + 1;  // Use as factorial number
        data[i].iterations = 0;
        data[i].delay_ms = 0;

        threads[i] = create_and_run_thread(factorial_thread_func, &data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        void* result;
        pthread_join(threads[i], &result);
        if (result != NULL) {
            printf("Factorial thread %d result: %d\n", i + 1, *(int*)result);
            free(result);
        }
    }
}

// Test mutex contention
void test_mutex_contention() {
    printf("\n=== Test 4: Mutex Contention ===\n");
    const int num_threads = 4;
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    global_counter = 0;

    for (int i = 0; i < num_threads; i++) {
        data[i].thread_id = i + 1;
        data[i].iterations = 3;
        data[i].delay_ms = 0;

        threads[i] = create_and_run_thread(mutex_contention_thread, &data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Mutex contention test complete. global_counter=%d\n", global_counter);
}

// Test static vs local variables in threads
void test_static_local_threads() {
    printf("\n=== Test 5: Static vs Local Variables ===\n");
    const int num_threads = 2;
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    for (int i = 0; i < num_threads; i++) {
        data[i].thread_id = i + 1;
        data[i].iterations = 5;
        data[i].delay_ms = 0;

        threads[i] = create_and_run_thread(static_thread, &data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Test signals in threads
void test_signal_threads() {
    printf("\n=== Test 6: Signals in Threads ===\n");
    pthread_t thread;
    ThreadData data;

    data.thread_id = 1;
    data.iterations = 0;
    data.delay_ms = 0;

    thread = create_and_run_thread(signal_thread, &data);

    // Send signal to the thread after a delay
    sleep(2);
    printf("Main: Sending SIGUSR1 to thread\n");
    pthread_kill(thread, SIGUSR1);

    pthread_join(thread, NULL);
}

// Test many threads
void test_many_threads() {
    printf("\n=== Test 7: Many Threads ===\n");
    const int num_threads = 10;
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    for (int i = 0; i < num_threads; i++) {
        data[i].thread_id = i + 1;
        data[i].iterations = 2;
        data[i].delay_ms = 50;

        threads[i] = create_and_run_thread(worker_thread, &data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Many threads test complete\n");
}

// Test thread with waiting
void test_waiting_threads() {
    printf("\n=== Test 8: Waiting Threads ===\n");
    const int num_threads = 3;
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    for (int i = 0; i < num_threads; i++) {
        data[i].thread_id = i + 1;
        data[i].iterations = 0;
        data[i].delay_ms = (i + 1) * 2;  // Different wait times

        threads[i] = create_and_run_thread(waiting_thread, &data[i]);
    }

    printf("Main: Waiting for threads to complete...\n");
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        printf("Main: Thread %d joined\n", i + 1);
    }
}

int main(int argc, char* argv[]) {
    printf("=== Thread Test Program ===\n");
    printf("This program tests multi-threading for debugger testing\n");
    printf("Usage: %s [test_type]\n", argv[0]);
    printf("  test_type: all|basic|deep|factorial|mutex|static|signal|many|wait (default: all)\n\n");

    // Parse command line argument
    const char* test_type = (argc > 1) ? argv[1] : "all";

    // Initialize mutex
    pthread_mutex_init(&counter_mutex, NULL);

    // Run tests based on type
    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "basic") == 0) {
        test_basic_threads();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "deep") == 0) {
        test_deep_stack_threads();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "factorial") == 0) {
        test_factorial_threads();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "mutex") == 0) {
        test_mutex_contention();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "static") == 0) {
        test_static_local_threads();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "signal") == 0) {
        test_signal_threads();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "many") == 0) {
        test_many_threads();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "wait") == 0) {
        test_waiting_threads();
    }

    // Cleanup
    pthread_mutex_destroy(&counter_mutex);

    // Summary
    printf("\n=== Thread Test Summary ===\n");
    printf("Threads created: %d\n", thread_created);
    printf("Threads started: %d\n", thread_started);
    printf("Threads finished: %d\n", thread_finished);
    printf("Final global_counter: %d\n", global_counter);

    printf("\n=== Test Complete ===\n");
    printf("Use debugger to inspect thread states and stacks\n");

    return 0;
}
