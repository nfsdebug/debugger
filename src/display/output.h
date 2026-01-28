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

/* Statistics for summaries */
typedef struct {
    unsigned long signals_received;
    unsigned long breakpoints_hit;
    unsigned long steps_executed;
    unsigned long memory_reads;
    unsigned long memory_writes;
    unsigned long errors;
    time_t start_time;
    time_t last_update;
} output_stats_t;

/* Global output configuration */
typedef struct {
    output_level_t level;
    output_category_t enabled_categories;
    int use_colors;
    int show_prefix;        /* Show category prefixes like [PROCESS] */
    int show_timestamp;     /* Show timestamps */
    FILE *log_file;
    char log_path[512];
    int summary_interval;   /* seconds, 0 = disabled */
    time_t last_summary;
    output_stats_t stats;
} output_config_t;

/* Initialize output system */
void output_init(output_config_t *config);

/* Set output level */
void output_set_level(output_level_t level);

/* Get current output level */
output_level_t output_get_level(void);

/* Enable/disable categories */
void output_enable_category(output_category_t cat);
void output_disable_category(output_category_t cat);

/* Prefix settings */
void output_set_show_prefix(int show);
void output_set_show_timestamp(int show);

/* Core output function (printf-like with level and category) */
void output_print(output_level_t level, output_category_t cat,
                  const char *fmt, ...);

/* Convenience macros */
#define output_quiet(cat, fmt, ...)   output_print(OUTPUT_QUIET, cat, fmt, ##__VA_ARGS__)
#define output_normal(cat, fmt, ...)  output_print(OUTPUT_NORMAL, cat, fmt, ##__VA_ARGS__)
#define output_verbose(cat, fmt, ...) output_print(OUTPUT_VERBOSE, cat, fmt, ##__VA_ARGS__)
#define output_debug(cat, fmt, ...)   output_print(OUTPUT_DEBUG, cat, fmt, ##__VA_ARGS__)

/* Specialized output with automatic formatting */
void output_signal(int signo, const char *msg);
void output_process_info(const char *fmt, ...);
void output_error(const char *fmt, ...);

/* Statistics */
void output_stats_reset(void);
void output_stats_inc_signal(void);
void output_stats_inc_breakpoint(void);
void output_stats_inc_step(void);
void output_stats_inc_memory_read(void);
void output_stats_inc_memory_write(void);
void output_stats_inc_error(void);

/* Logging */
int output_open_log(const char *path);
void output_close_log(void);
void output_summary(void);

/* Log rotation */
typedef enum {
    LOG_ROTATION_NONE,     /* No rotation */
    LOG_ROTATION_SIZE,     /* Rotate by size */
    LOG_ROTATION_TIME      /* Rotate by time */
} log_rotation_t;

/* Log configuration */
typedef struct {
    log_rotation_t rotation;
    size_t max_size;       /* Max size before rotation (bytes) */
    int max_files;         /* Max number of rotated files to keep */
    int rotate_interval;   /* Rotation interval in seconds (for TIME rotation) */
    char base_path[512];   /* Base path for log files */
} log_config_t;

/* Advanced logging */
int output_open_log_with_config(const char *path, const log_config_t *config);
void output_set_log_config(const log_config_t *config);
void output_rotate_log(void);

/* Periodic summaries */
void output_enable_periodic_summary(int interval_seconds);
void output_disable_periodic_summary(void);
int output_get_summary_interval(void);

/* Category name for prefixes */
const char *output_category_name(output_category_t cat);

#endif /* OUTPUT_H */
