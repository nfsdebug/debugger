/**
 * @file config.c
 * @brief User preferences and configuration (libconfig)
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations - avoid circular dependencies */
struct output_config_s;
struct theme_config_s;

/* Global configuration */
static debugger_config_t g_config;

void config_init(debugger_config_t *config) {
    if (config) {
        memcpy(&g_config, config, sizeof(g_config));
    } else {
        /* Initialize with defaults */
        memset(&g_config, 0, sizeof(g_config));

        /* Output settings */
        g_config.output.level = OUTPUT_NORMAL;
        g_config.output.enabled_categories = CAT_ALL;
        g_config.output.use_colors = 1;
        g_config.output.log_file = NULL;
        g_config.output.summary_interval = 0;

        /* Theme settings */
        g_config.theme.use_colors = 1;
        g_config.theme.is_terminal = 1;

        /* Sections defaults */
        g_config.default_expand = EXPAND_NONE;

        /* REPL settings */
        strcpy(g_config.prompt, "dbg> ");
        g_config.history_size = 1000;
        g_config.history_path[0] = '\0';

        /* Log settings */
        g_config.auto_log = 0;
        g_config.log_path[0] = '\0';

        /* Summary interval */
        g_config.summary_interval = 0;
    }
}

#ifdef HAVE_CONFIG
int config_load(const char *path, debugger_config_t *config) {
    config_t cfg;
    config_init_t(&cfg);

    if (!config_read_file(path, &cfg)) {
        return -1;
    }

    /* TODO: Load configuration from file */
    /* For now, just use defaults */

    config_destroy(&cfg);
    return 0;
}

int config_save(const char *path, const debugger_config_t *config) {
    config_t cfg;
    config_init_t(&cfg);
    config_setting_t *root;

    config_write_file(&cfg, path);
    config_destroy(&cfg);

    return 0;
}
#endif

void config_set_output_level(output_level_t level) {
    g_config.output.level = level;
}

void config_set_prompt(const char *prompt) {
    strncpy(g_config.prompt, prompt, sizeof(g_config.prompt) - 1);
}

void config_set_expand(expand_level_t level) {
    g_config.default_expand = level;
}

debugger_config_t *config_get(void) {
    return &g_config;
}
