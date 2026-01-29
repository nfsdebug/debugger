/**
 * @file test_signals.c
 * @brief Test program that triggers various signals for debugger testing
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

// Global variables for signal testing
volatile sig_atomic_t signal_received = 0;
volatile sig_atomic_t sigsegv_count = 0;
volatile sig_atomic_t sigfpe_count = 0;
volatile sig_atomic_t sigill_count = 0;
volatile sig_atomic_t sigusr1_count = 0;

sigjmp_buf jump_buffer;

// Signal handlers
void sigsegv_handler(int sig) {
    (void)sig;
    sigsegv_count++;
    signal_received = SIGSEGV;
    printf("SIGSEGV handler called (count=%d)\n", sigsegv_count);
    siglongjmp(jump_buffer, 1);
}

void sigfpe_handler(int sig) {
    (void)sig;
    sigfpe_count++;
    signal_received = SIGFPE;
    printf("SIGFPE handler called (count=%d)\n", sigfpe_count);
    siglongjmp(jump_buffer, 1);
}

void sigill_handler(int sig) {
    (void)sig;
    sigill_count++;
    signal_received = SIGILL;
    printf("SIGILL handler called (count=%d)\n", sigill_count);
    siglongjmp(jump_buffer, 1);
}

void sigusr1_handler(int sig) {
    (void)sig;
    sigusr1_count++;
    signal_received = SIGUSR1;
    printf("SIGUSR1 handler called (count=%d)\n", sigusr1_count);
}

void sigint_handler(int sig) {
    (void)sig;
    printf("SIGINT handler called - use this to test debugger interrupt\n");
}

void sigterm_handler(int sig) {
    (void)sig;
    printf("SIGTERM handler called\n");
}

// Function that causes SIGSEGV
void cause_sigsegv() {
    printf("About to cause SIGSEGV (null pointer dereference)...\n");
    int* null_ptr = NULL;
    *null_ptr = 42;  // This will cause SIGSEGV
    printf("This line should not be reached\n");
}

// Function that causes SIGFPE (division by zero)
void cause_sigfpe() {
    printf("About to cause SIGFPE (division by zero)...\n");
    int a = 10;
    int b = 0;
    int c = a / b;  // This will cause SIGFPE
    printf("This line should not be reached, c=%d\n", c);
}

// Function that causes SIGFPE (integer overflow - on some systems)
void cause_sigfpe_overflow() {
    printf("About to cause SIGFPE (integer overflow)...\n");
    volatile int x = INT_MAX;
    int y = x + 1;  // May cause SIGFPE on some systems
    printf("Result: %d\n", y);
}

// Function that causes SIGILL
void cause_sigill() {
    printf("About to cause SIGILL (illegal instruction)...\n");
    // This is platform-specific, but we'll try a common approach
    // Using inline assembly to execute an invalid instruction
    #if defined(__x86_64__) || defined(__i386__)
        __asm__ volatile(".byte 0x0f, 0x0b");  // UD2 - undefined instruction
    #elif defined(__aarch64__)
        __asm__ volatile(".word 0x00000000");  // Unimplemented instruction
    #endif
    printf("This line should not be reached\n");
}

// Safe wrapper for signal testing
void test_sigsegv_safe() {
    printf("\n--- Testing SIGSEGV (Segmentation Fault) ---\n");
    if (sigsetjmp(jump_buffer, 1) == 0) {
        cause_sigsegv();
    } else {
        printf("Recovered from SIGSEGV\n");
    }
}

void test_sigfpe_safe() {
    printf("\n--- Testing SIGFPE (Floating Point Exception) ---\n");
    if (sigsetjmp(jump_buffer, 1) == 0) {
        cause_sigfpe();
    } else {
        printf("Recovered from SIGFPE\n");
    }
}

void test_sigill_safe() {
    printf("\n--- Testing SIGILL (Illegal Instruction) ---\n");
    if (sigsetjmp(jump_buffer, 1) == 0) {
        cause_sigill();
    } else {
        printf("Recovered from SIGILL\n");
    }
}

// Function to test SIGUSR1
void test_sigusr1() {
    printf("\n--- Testing SIGUSR1 ---\n");
    printf("Sending SIGUSR1 to self...\n");
    raise(SIGUSR1);
    sleep(1);
    printf("SIGUSR1 test complete\n");
}

// Function to test signal handling in nested calls
void level3_signal() {
    printf("In level3_signal\n");
}

void level2_signal() {
    printf("In level2_signal\n");
    level3_signal();
}

void level1_signal() {
    printf("In level1_signal\n");
    level2_signal();
}

// Test signals during execution flow
void test_signal_during_execution() {
    printf("\n--- Testing signals during execution ---\n");
    for (int i = 0; i < 5; i++) {
        printf("Iteration %d\n", i);
        if (i == 2) {
            printf("Sending SIGUSR1 at iteration 2\n");
            raise(SIGUSR1);
        }
        sleep(1);
    }
}

// Test stack trace with signal handler
void recursive_signal_test(int depth) {
    printf("Recursive depth: %d\n", depth);
    if (depth > 0) {
        if (depth == 3) {
            printf("Sending SIGUSR1 in deep call stack\n");
            raise(SIGUSR1);
        }
        recursive_signal_test(depth - 1);
    }
}

// Test multiple rapid signals
void test_rapid_signals() {
    printf("\n--- Testing rapid signals ---\n");
    for (int i = 0; i < 5; i++) {
        printf("Sending SIGUSR1 #%d\n", i + 1);
        raise(SIGUSR1);
    }
    printf("Total SIGUSR1 received: %d\n", sigusr1_count);
}

// Setup signal handlers
void setup_signal_handlers() {
    struct sigaction sa;

    // SIGSEGV handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER;  // Allow nested signals
    sigaction(SIGSEGV, &sa, NULL);

    // SIGFPE handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigfpe_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER;
    sigaction(SIGFPE, &sa, NULL);

    // SIGILL handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigill_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER;
    sigaction(SIGILL, &sa, NULL);

    // SIGUSR1 handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    // SIGINT handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // SIGTERM handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigterm_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);

    printf("Signal handlers installed\n");
}

// Test signals in different scenarios
void test_signal_scenarios() {
    printf("\n=== Testing Signal Scenarios ===\n");

    printf("\n1. Testing signal with nested calls\n");
    level1_signal();

    printf("\n2. Testing signal during recursion\n");
    recursive_signal_test(5);

    printf("\n3. Testing rapid signals\n");
    test_rapid_signals();
}

// Test ignoring signals
void test_signal_ignore() {
    printf("\n--- Testing signal ignore ---\n");
    printf("Ignoring SIGUSR1 temporarily...\n");
    signal(SIGUSR1, SIG_IGN);
    raise(SIGUSR1);
    printf("SIGUSR1 was ignored (count still %d)\n", sigusr1_count);

    printf("Restoring SIGUSR1 handler...\n");
    signal(SIGUSR1, sigusr1_handler);
    raise(SIGUSR1);
    printf("SIGUSR1 count now %d\n", sigusr1_count);
}

// Test default signal handling
void test_signal_default() {
    printf("\n--- Testing default signal handling (safe test) ---\n");
    printf("Setting SIGUSR1 to default and testing with SIGUSR2 instead\n");
    signal(SIGUSR1, sigusr1_handler);  // Restore handler
}

int main(int argc, char* argv[]) {
    printf("=== Signal Test Program ===\n");
    printf("This program tests various signals for debugger testing\n");
    printf("Usage: %s [test_type]\n", argv[0]);
    printf("  test_type: all|segv|fpe|ill|usr1|rapid|nested (default: all)\n\n");

    // Parse command line argument
    const char* test_type = (argc > 1) ? argv[1] : "all";

    // Setup signal handlers
    setup_signal_handlers();

    // Run tests based on type
    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "segv") == 0) {
        test_sigsegv_safe();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "fpe") == 0) {
        test_sigfpe_safe();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "ill") == 0) {
        // Skip SIGILL test by default as it can be unstable
        printf("\n--- Skipping SIGILL test (use 'ill' argument to enable) ---\n");
        if (strcmp(test_type, "ill") == 0) {
            test_sigill_safe();
        }
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "usr1") == 0) {
        test_sigusr1();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "rapid") == 0) {
        test_rapid_signals();
    }

    if (strcmp(test_type, "all") == 0 || strcmp(test_type, "nested") == 0) {
        test_signal_scenarios();
    }

    if (strcmp(test_type, "all") == 0) {
        test_signal_ignore();
        test_signal_default();
    }

    // Summary
    printf("\n=== Signal Test Summary ===\n");
    printf("SIGSEGV caught: %d\n", sigsegv_count);
    printf("SIGFPE caught: %d\n", sigfpe_count);
    printf("SIGILL caught: %d\n", sigill_count);
    printf("SIGUSR1 caught: %d\n", sigusr1_count);

    printf("\n=== Test Complete ===\n");
    printf("Use Ctrl+\\ (SIGQUIT) to trigger debugger pause\n");
    printf("Use Ctrl+C (SIGINT) to test interrupt handling\n");

    // Wait a bit for manual testing
    printf("\nWaiting 5 seconds for manual debugger attachment...\n");
    sleep(5);

    return 0;
}
