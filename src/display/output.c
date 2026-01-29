/**
 * @file output.c
 * @brief Output system with levels (quiet/normal/verbose)
 */

#include "output.h"
#include "theme.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Global output configuration */
static output_config_t g_output_config = {
    .level = OUTPUT_NORMAL,
    .enabled_categories = CAT_ALL,
    .use_colors = 1,
    .show_prefix = 1,
    .show_timestamp = 0,
    .log_file = NULL,
    .log_path = "",
    .summary_interval = 0,
    .last_summary = 0,
    .stats = {0}
};

/* Global log configuration */
static log_config_t g_log_config = {
    .rotation = LOG_ROTATION_NONE,
    .max_size = 10 * 1024 * 1024,  /* 10 MB default */
    .max_files = 5,
    .rotate_interval = 3600,         /* 1 hour default */
    .base_path = ""
};

/* Current log file size */
static size_t g_current_log_size = 0;

/* Forward declaration */
static void check_log_rotation(size_t bytes_written);

/* Category names */
static const char *category_names[] = {
    "PROCESS", "SIGNAL", "BACKTRACE", "REGISTERS", "MEMORY", "BREAKPOINT"
};

void output_init(const output_config_t *config) {
    if (config) {
        memcpy(&g_output_config, config, sizeof(g_output_config));
    }

    /* Auto-detect if we're a terminal */
    g_output_config.use_colors = isatty(STDOUT_FILENO);

    /* Initialize stats */
    g_output_config.stats.start_time = time(NULL);
    g_output_config.stats.last_update = g_output_config.stats.start_time;
}

void output_set_level(output_level_t level) {
    g_output_config.level = level;
}

output_level_t output_get_level(void) {
    return g_output_config.level;
}

void output_set_show_prefix(int show) {
    g_output_config.show_prefix = show;
}

void output_set_show_timestamp(int show) {
    g_output_config.show_timestamp = show;
}

void output_enable_category(output_category_t cat) {
    g_output_config.enabled_categories |= cat;
}

void output_disable_category(output_category_t cat) {
    g_output_config.enabled_categories &= ~cat;
}

const char *output_category_name(output_category_t cat) {
    /* Find which bit is set */
    for (int i = 0; i < 6; i++) {
        if (cat & (1 << i)) {
            return category_names[i];
        }
    }
    return "UNKNOWN";
}

static void print_prefix(output_level_t level, output_category_t cat) {
    (void)level; /* Unused parameter - kept for future use */
    if (!g_output_config.show_prefix) {
        return;
    }

    /* Timestamp */
    if (g_output_config.show_timestamp) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
        printf("%s ", timestamp);

        if (g_output_config.log_file) {
            fprintf(g_output_config.log_file, "%s ", timestamp);
        }
    }

    /* Category prefix with color */
    if (g_output_config.use_colors) {
        const char *color = theme_color(theme_color_for_category(cat));
        const char *reset = theme_reset();
        printf("%s[%s]%s ", color, output_category_name(cat), reset);

        if (g_output_config.log_file) {
            /* Don't use colors in log file */
            fprintf(g_output_config.log_file, "[%s] ", output_category_name(cat));
        }
    } else {
        printf("[%s] ", output_category_name(cat));

        if (g_output_config.log_file) {
            fprintf(g_output_config.log_file, "[%s] ", output_category_name(cat));
        }
    }
}

void output_print(output_level_t level, output_category_t cat,
                  const char *fmt, ...) {
    /* Check level */
    if (level > g_output_config.level) {
        return;
    }

    /* Check category */
    if (!(cat & g_output_config.enabled_categories)) {
        return;
    }

    /* Print prefix */
    print_prefix(level, cat);

    /* Format the message */
    va_list args;
    va_start(args, fmt);

    /* Print to console */
    vprintf(fmt, args);

    /* Print to log file if open */
    if (g_output_config.log_file) {
        /* Get formatted length for rotation tracking */
        va_list args_copy;
        va_copy(args_copy, args);
        int len = vsnprintf(NULL, 0, fmt, args_copy);
        va_end(args_copy);

        vfprintf(g_output_config.log_file, fmt, args);
        fflush(g_output_config.log_file);

        /* Track size and check rotation */
        if (len > 0) {
            check_log_rotation((size_t)len);
        }
    }

    va_end(args);

    /* Update stats timestamp */
    g_output_config.stats.last_update = time(NULL);

    /* Check if we should print a summary (long-running) */
    if (g_output_config.summary_interval > 0) {
        time_t now = time(NULL);
        if (now - g_output_config.last_summary >= g_output_config.summary_interval) {
            output_summary();
            g_output_config.last_summary = now;
        }
    }
}

/* Specialized output functions */

void output_signal(int signo, const char *msg) {
    /* Always show signals, even in QUIET mode */
    const char *color = theme_color(COLOR_RED);
    const char *reset = theme_reset();

    if (g_output_config.use_colors) {
        printf("%s[SIG%d]%s %s\n", color, signo, reset, msg ? msg : "");
    } else {
        printf("[SIG%d] %s\n", signo, msg ? msg : "");
    }

    if (g_output_config.log_file) {
        fprintf(g_output_config.log_file, "[SIG%d] %s\n", signo, msg ? msg : "");
    }

    output_stats_inc_signal();
}

void output_process_info(const char *fmt, ...) {
    if (g_output_config.level < OUTPUT_NORMAL) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    const char *color = theme_color(COLOR_CYAN);
    const char *reset = theme_reset();

    if (g_output_config.use_colors) {
        printf("%s[PROCESS]%s ", color, reset);
    } else {
        printf("[PROCESS] ");
    }

    vprintf(fmt, args);
    printf("\n");

    if (g_output_config.log_file) {
        fprintf(g_output_config.log_file, "[PROCESS] ");
        vfprintf(g_output_config.log_file, fmt, args);
        fprintf(g_output_config.log_file, "\n");
        fflush(g_output_config.log_file);
    }

    va_end(args);
}

void output_error(const char *fmt, ...) {
    /* Always show errors */
    va_list args;
    va_start(args, fmt);

    const char *color = theme_color(COLOR_BRIGHT_RED);
    const char *bold = theme_style(STYLE_BOLD);
    const char *reset = theme_reset();

    if (g_output_config.use_colors) {
        printf("%s%s[ERROR]%s ", color, bold, reset);
    } else {
        printf("[ERROR] ");
    }

    vprintf(fmt, args);
    printf("\n");

    if (g_output_config.log_file) {
        fprintf(g_output_config.log_file, "[ERROR] ");
        vfprintf(g_output_config.log_file, fmt, args);
        fprintf(g_output_config.log_file, "\n");
        fflush(g_output_config.log_file);
    }

    va_end(args);
    output_stats_inc_error();
}

/* Statistics functions */

void output_stats_reset(void) {
    memset(&g_output_config.stats, 0, sizeof(g_output_config.stats));
    g_output_config.stats.start_time = time(NULL);
    g_output_config.stats.last_update = g_output_config.stats.start_time;
}

void output_stats_inc_signal(void) {
    g_output_config.stats.signals_received++;
}

void output_stats_inc_breakpoint(void) {
    g_output_config.stats.breakpoints_hit++;
}

void output_stats_inc_step(void) {
    g_output_config.stats.steps_executed++;
}

void output_stats_inc_memory_read(void) {
    g_output_config.stats.memory_reads++;
}

void output_stats_inc_memory_write(void) {
    g_output_config.stats.memory_writes++;
}

void output_stats_inc_error(void) {
    g_output_config.stats.errors++;
}

void output_summary(void) {
    time_t now = time(NULL);
    long runtime = now - g_output_config.stats.start_time;

    const char *color = theme_color(COLOR_CYAN);
    const char *reset = theme_reset();

    if (g_output_config.use_colors) {
        printf("\n%s%s", color, theme_format(COLOR_CYAN, STYLE_BOLD));
        printf("+-------------------------------+");
        printf("%s\n", reset);
    } else {
        printf("\n+-------------------------------+\n");
    }

    printf("[SUMMARY] Runtime: %ld seconds\n", runtime);

    if (g_output_config.stats.signals_received > 0) {
        printf("  Signals received: %lu\n", g_output_config.stats.signals_received);
    }
    if (g_output_config.stats.breakpoints_hit > 0) {
        printf("  Breakpoints hit: %lu\n", g_output_config.stats.breakpoints_hit);
    }
    if (g_output_config.stats.steps_executed > 0) {
        printf("  Steps executed: %lu\n", g_output_config.stats.steps_executed);
    }
    if (g_output_config.stats.memory_reads > 0 || g_output_config.stats.memory_writes > 0) {
        printf("  Memory operations: %lu reads, %lu writes\n",
               g_output_config.stats.memory_reads, g_output_config.stats.memory_writes);
    }
    if (g_output_config.stats.errors > 0) {
        const char *err_color = theme_color(COLOR_RED);
        printf("  %sErrors: %lu%s\n", err_color, g_output_config.stats.errors, reset);
    }

    if (g_output_config.use_colors) {
        printf("%s+-------------------------------+%s\n\n", color, reset);
    } else {
        printf("+-------------------------------+\n\n");
    }

    /* Flush to log as well */
    if (g_output_config.log_file) {
        fprintf(g_output_config.log_file, "\n[SUMMARY] Runtime: %ld seconds\n", runtime);
        fprintf(g_output_config.log_file, "  Signals: %lu, Breakpoints: %lu, Steps: %lu\n",
                g_output_config.stats.signals_received,
                g_output_config.stats.breakpoints_hit,
                g_output_config.stats.steps_executed);
        fprintf(g_output_config.log_file, "  Memory: %lu reads, %lu writes\n",
                g_output_config.stats.memory_reads, g_output_config.stats.memory_writes);
        fprintf(g_output_config.log_file, "  Errors: %lu\n\n", g_output_config.stats.errors);
        fflush(g_output_config.log_file);
    }
}

/* Logging */

int output_open_log(const char *path) {
    FILE *f = fopen(path, "a");
    if (!f) {
        return -1;
    }

    if (g_output_config.log_file) {
        fclose(g_output_config.log_file);
    }

    g_output_config.log_file = f;
    strncpy(g_output_config.log_path, path, sizeof(g_output_config.log_path) - 1);

    /* Write header */
    fprintf(f, "\n=== Log started at %ld ===\n", (long)time(NULL));
    fflush(f);

    return 0;
}

void output_close_log(void) {
    if (g_output_config.log_file) {
        fprintf(g_output_config.log_file, "\n=== Log ended at %ld ===\n", (long)time(NULL));
        fclose(g_output_config.log_file);
        g_output_config.log_file = NULL;
    }
}

/* === LOG ROTATION === */

static void rotate_log_files(const char *base_path, int max_files) {
    char old_path[512];
    char new_path[512];

    /* Remove the oldest file if it exists */
    snprintf(old_path, sizeof(old_path), "%s.%d", base_path, max_files);
    remove(old_path);

    /* Rotate existing files */
    for (int i = max_files - 1; i >= 1; i--) {
        snprintf(old_path, sizeof(old_path), "%s.%d", base_path, i);
        snprintf(new_path, sizeof(new_path), "%s.%d", base_path, i + 1);
        rename(old_path, new_path);
    }

    /* Rotate current log to .1 */
    snprintf(new_path, sizeof(new_path), "%s.1", base_path);
    rename(base_path, new_path);
}

void output_rotate_log(void) {
    if (!g_output_config.log_file || g_log_config.rotation == LOG_ROTATION_NONE) {
        return;
    }

    /* Close current log */
    output_close_log();

    /* Rotate files if configured */
    if (g_log_config.base_path[0] != '\0') {
        rotate_log_files(g_log_config.base_path, g_log_config.max_files);
    }

    /* Reopen log */
    output_open_log(g_output_config.log_path);
}

static void check_log_rotation(size_t bytes_written) {
    if (g_log_config.rotation == LOG_ROTATION_SIZE) {
        g_current_log_size += bytes_written;
        if (g_current_log_size >= g_log_config.max_size) {
            output_rotate_log();
            g_current_log_size = 0;
        }
    }
}

void output_set_log_config(const log_config_t *config) {
    if (config) {
        memcpy(&g_log_config, config, sizeof(g_log_config));
    }
}

int output_open_log_with_config(const char *path, const log_config_t *config) {
    if (config) {
        output_set_log_config(config);
    }

    /* Store base path for rotation */
    if (path) {
        strncpy(g_log_config.base_path, path, sizeof(g_log_config.base_path) - 1);
    }

    return output_open_log(path);
}

/* === PERIODIC SUMMARIES === */

void output_enable_periodic_summary(int interval_seconds) {
    g_output_config.summary_interval = interval_seconds;
    g_output_config.last_summary = time(NULL);
}

void output_disable_periodic_summary(void) {
    g_output_config.summary_interval = 0;
}

int output_get_summary_interval(void) {
    return g_output_config.summary_interval;
}
