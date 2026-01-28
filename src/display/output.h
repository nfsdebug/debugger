/**
 * @file output.h
 * @brief Output system with levels (quiet/normal/verbose)
 */

#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdio.h>
#include <time.h>

/* Output levels */
typedef enum {
    OUTPUT_QUIET = 0,      /* Errors and signals only */
    OUTPUT_NORMAL = 1,     /* Essential information */
    OUTPUT_VERBOSE = 2,    /* All details */
    OUTPUT_DEBUG = 3       /* Debug information */
} output_level_t;

/* Output categories (for filtering) */
typedef enum {
    CAT_PROCESS    = (1 << 0),
    CAT_SIGNAL     = (1 << 1),
    CAT_BACKTRACE  = (1 << 2),
    CAT_REGISTERS  = (1 << 3),
    CAT_MEMORY     = (1 << 4),
    CAT_BREAKPOINT = (1 << 5),
    CAT_ALL        = 0xFF
} output_category_t;

/* Global output configuration */
typedef struct {
    output_level_t level;
    output_category_t enabled_categories;
    int use_colors;
    FILE *log_file;
    char log_path[512];
    int summary_interval;  /* seconds, 0 = disabled */
    time_t last_summary;
} output_config_t;

/* Initialize output system */
void output_init(output_config_t *config);

/* Set output level */
void output_set_level(output_level_t level);

/* Enable/disable categories */
void output_enable_category(output_category_t cat);
void output_disable_category(output_category_t cat);

/* Core output function (printf-like with level and category) */
void output_print(output_level_t level, output_category_t cat,
                  const char *fmt, ...);

/* Convenience macros */
#define output_quiet(cat, fmt, ...)   output_print(OUTPUT_QUIET, cat, fmt, ##__VA_ARGS__)
#define output_normal(cat, fmt, ...)  output_print(OUTPUT_NORMAL, cat, fmt, ##__VA_ARGS__)
#define output_verbose(cat, fmt, ...) output_print(OUTPUT_VERBOSE, cat, fmt, ##__VA_ARGS__)
#define output_debug(cat, fmt, ...)   output_print(OUTPUT_DEBUG, cat, fmt, ##__VA_ARGS__)

/* Logging */
int output_open_log(const char *path);
void output_close_log(void);
void output_summary(void);

#endif /* OUTPUT_H */
