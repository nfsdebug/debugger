/**
 * @file config.c
 * @brief User preferences and configuration
 */

#include "config.h"
#include "../display/output.h"
#include "../display/theme.h"
#include "../display/sections.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        g_config.output.show_prefix = 1;
        g_config.output.show_timestamp = 0;
        g_config.output.log_file = NULL;
        g_config.output.summary_interval = 0;

        /* Theme settings */
        g_config.theme.use_colors = 1;
        g_config.theme.is_terminal = 1;
        g_config.theme.color_mode = COLOR_AUTO;
        g_config.theme.supports_256color = 0;
        g_config.theme.supports_truecolor = 0;

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

        /* Color mode */
        g_config.use_colors = 1;
        g_config.show_timestamps = 0;
    }
}

void config_apply(const debugger_config_t *config) {
    if (!config) return;

    /* Apply output settings */
    output_set_level(config->output.level);
    output_set_show_prefix(config->output.show_prefix);
    output_set_show_timestamp(config->output.show_timestamp);

    /* Apply theme settings */
    theme_init(&config->theme);

    /* Apply sections settings */
    sections_set_global_expand(config->default_expand);

    /* Open log file if configured */
    if (config->auto_log && config->log_path[0] != '\0') {
        output_open_log(config->log_path);
    }
}

void config_reset(void) {
    config_init(NULL);
    config_apply(&g_config);
}

void config_set_output_level(output_level_t level) {
    g_config.output.level = level;
    output_set_level(level);
}

void config_set_prompt(const char *prompt) {
    if (prompt) {
        strncpy(g_config.prompt, prompt, sizeof(g_config.prompt) - 1);
    }
}

void config_set_expand(expand_level_t level) {
    g_config.default_expand = level;
    sections_set_global_expand(level);
}

void config_set_color_mode(int enable) {
    g_config.use_colors = enable;
    g_config.theme.use_colors = enable;
    theme_set_colors(enable);
}

void config_set_timestamps(int enable) {
    g_config.show_timestamps = enable;
    g_config.output.show_timestamp = enable;
    output_set_show_timestamp(enable);
}

debugger_config_t *config_get(void) {
    return &g_config;
}

#ifdef HAVE_CONFIG
int config_load(const char *path, debugger_config_t *config) {
    config_t cfg;
    config_init_t(&cfg);

    if (!config_read_file(path, &cfg)) {
        fprintf(stderr, "Failed to load config from %s\n", path);
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
