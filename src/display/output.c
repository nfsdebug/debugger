/**
 * @file output.c
 * @brief Output system with levels (quiet/normal/verbose)
 */

#include "output.h"
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
    .log_file = NULL,
    .log_path = "",
    .summary_interval = 0,
    .last_summary = 0
};

void output_init(output_config_t *config) {
    if (config) {
        memcpy(&g_output_config, config, sizeof(g_output_config));
    }

    /* Auto-detect if we're a terminal */
    g_output_config.use_colors = isatty(STDOUT_FILENO);
}

void output_set_level(output_level_t level) {
    g_output_config.level = level;
}

void output_enable_category(output_category_t cat) {
    g_output_config.enabled_categories |= cat;
}

void output_disable_category(output_category_t cat) {
    g_output_config.enabled_categories &= ~cat;
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

    /* Format the message */
    va_list args;
    va_start(args, fmt);

    /* Print to console */
    vprintf(fmt, args);

    /* Print to log file if open */
    if (g_output_config.log_file) {
        vfprintf(g_output_config.log_file, fmt, args);
        fflush(g_output_config.log_file);
    }

    va_end(args);

    /* Check if we should print a summary (long-running) */
    if (g_output_config.summary_interval > 0) {
        time_t now = time(NULL);
        if (now - g_output_config.last_summary >= g_output_config.summary_interval) {
            output_summary();
            g_output_config.last_summary = now;
        }
    }
}

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

void output_summary(void) {
    /* TODO: Implement periodic summary */
    printf("[SUMMARY] Runtime: %ld events: 0\n", (long)time(NULL));
}
