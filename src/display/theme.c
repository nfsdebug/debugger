/**
 * @file theme.c
 * @brief Color detection and ANSI codes
 */

#include "theme.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Global theme configuration */
static theme_config_t g_theme = {
    .use_colors = 0,
    .is_terminal = 0
};

/* ANSI color codes */
static const char *ansi_colors[] = {
    "\033[0m",       /* RESET */
    "\033[30m",      /* BLACK */
    "\033[31m",      /* RED */
    "\033[32m",      /* GREEN */
    "\033[33m",      /* YELLOW */
    "\033[34m",      /* BLUE */
    "\033[35m",      /* MAGENTA */
    "\033[36m",      /* CYAN */
    "\033[37m",      /* WHITE */
};

/* ANSI style codes */
static const char *ansi_bold = "\033[1m";
static const char *ansi_dim = "\033[2m";
static const char *ansi_underline = "\033[4m";
static const char *ansi_blink = "\033[5m";
static const char *ansi_reverse = "\033[7m";

/* Box styles */
const box_style_t box_ascii = {
    .h = "-",
    .v = "|",
    .tl = "+",
    .tr = "+",
    .bl = "+",
    .br = "+",
    .branch_t = "|-",
    .branch_l = "`-"
};

const box_style_t box_compact = {
    .h = "",
    .v = "",
    .tl = "",
    .tr = "",
    .bl = "",
    .br = "",
    .branch_t = "| ",
    .branch_l = "` "
};

void theme_init(theme_config_t *config) {
    if (config) {
        memcpy(&g_theme, config, sizeof(g_theme));
    } else {
        /* Auto-detect */
        g_theme.is_terminal = isatty(STDOUT_FILENO);
        g_theme.use_colors = g_theme.is_terminal;
    }
}

void theme_set_colors(int enable) {
    g_theme.use_colors = enable;
}

const char *theme_color(color_t fg) {
    if (!g_theme.use_colors) {
        return "";
    }
    if (fg >= 0 && fg < sizeof(ansi_colors) / sizeof(ansi_colors[0])) {
        return ansi_colors[fg];
    }
    return "";
}

const char *theme_color_bg(color_t bg) {
    /* Not implemented for now */
    (void)bg;
    return "";
}

const char *theme_style(style_t style) {
    if (!g_theme.use_colors) {
        return "";
    }

    switch (style) {
        case STYLE_BOLD:      return ansi_bold;
        case STYLE_DIM:       return ansi_dim;
        case STYLE_UNDERLINE: return ansi_underline;
        case STYLE_BLINK:     return ansi_blink;
        case STYLE_REVERSE:   return ansi_reverse;
        default: return "";
    }
}

const char *theme_reset(void) {
    if (!g_theme.use_colors) {
        return "";
    }
    return ansi_colors[COLOR_RESET];
}

void theme_print_box(const box_style_t *style, const char *title, int width) {
    if (width < 4) return;

    /* Top border */
    printf("%s", style->tl);
    for (int i = 1; i < width - 1; i++) {
        printf("%s", style->h);
    }
    printf("%s\n", style->tr);

    /* Title line */
    if (title && *title) {
        printf("%s %s%s\n", style->v, title, style->v);
    }
}

void theme_print_separator(const char *ch, int length) {
    for (int i = 0; i < length; i++) {
        printf("%s", ch ? ch : "-");
    }
    printf("\n");
}
