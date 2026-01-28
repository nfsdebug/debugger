/**
 * @file config.h
 * @brief User preferences and configuration (libconfig)
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef HAVE_CONFIG
#include <libconfig.h>
#endif

#include "display/output.h"
#include "display/theme.h"
#include "display/sections.h"

/* Global configuration */
typedef struct {
    /* Output settings */
    output_config_t output;

    /* Theme settings */
    theme_config_t theme;

    /* Sections defaults */
    expand_level_t default_expand;

    /* REPL settings */
    char prompt[32];
    int history_size;
    char history_path[512];

    /* Log settings */
    int auto_log;
    char log_path[512];

    /* Summary interval (seconds, 0 = disabled) */
    int summary_interval;
} debugger_config_t;

/* Initialize configuration with defaults */
void config_init(debugger_config_t *config);

/* Load configuration from file */
#ifdef HAVE_CONFIG
int config_load(const char *path, debugger_config_t *config);
int config_save(const char *path, const debugger_config_t *config);
#endif

/* Set configuration values */
void config_set_output_level(output_level_t level);
void config_set_prompt(const char *prompt);
void config_set_expand(expand_level_t level);

/* Get global configuration */
debugger_config_t *config_get(void);

#endif /* CONFIG_H */
