/**
 * @file theme.c
 * @brief Color detection and ANSI codes
 */

#include "theme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Global theme configuration */
static theme_config_t g_theme = {0};

/* Buffer for combined format codes */
static char g_format_buffer[32];

/* ANSI color codes - 16 colors (8 normal + 8 bright) */
static const char *ansi_colors[] = {
    "\033[0m",              /* RESET */
    /* Normal (30-37) */
    "\033[30m",             /* BLACK */
    "\033[31m",             /* RED */
    "\033[32m",             /* GREEN */
    "\033[33m",             /* YELLOW */
    "\033[34m",             /* BLUE */
    "\033[35m",             /* MAGENTA */
    "\033[36m",             /* CYAN */
    "\033[37m",             /* WHITE */
    /* Bright (90-97) */
    "\033[90m",             /* BRIGHT_BLACK */
    "\033[91m",             /* BRIGHT_RED */
    "\033[92m",             /* BRIGHT_GREEN */
    "\033[93m",             /* BRIGHT_YELLOW */
    "\033[94m",             /* BRIGHT_BLUE */
    "\033[95m",             /* BRIGHT_MAGENTA */
    "\033[96m",             /* BRIGHT_CYAN */
    "\033[97m",             /* BRIGHT_WHITE */
};

/* ANSI style codes */
static const char *ansi_bold = "\033[1m";
static const char *ansi_dim = "\033[2m";
static const char *ansi_underline = "\033[4m";
static const char *ansi_blink = "\033[5m";
static const char *ansi_reverse = "\033[7m";
static const char *ansi_hidden = "\033[8m";
static const char *ansi_strikethrough = "\033[9m";

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

static int detect_color_support(void) {
    /* 1. Check NO_COLOR environment variable (standard) */
    if (getenv("NO_COLOR") != NULL) {
        return 0;
    }

    /* 2. Check if output is a terminal */
    if (!isatty(STDOUT_FILENO)) {
        return 0;
    }

    /* 3. Check COLORTERM variable */
    const char *colorterm = getenv("COLORTERM");
    if (colorterm != NULL) {
        /* truecolor, 24bit, gnome-terminal, etc. */
        if (strstr(colorterm, "truecolor") || strstr(colorterm, "24bit")) {
            return 1;
        }
    }

    /* 4. Check TERM variable */
    const char *term = getenv("TERM");
    if (term == NULL) {
        return 0;
    }

    /* Check for color support in TERM */
    if (strstr(term, "xterm") || strstr(term, "screen") ||
        strstr(term, "tmux") || strstr(term, "rxvt") ||
        strstr(term, "vt100") || strstr(term, "ansi") ||
        strstr(term, "color") || strstr(term, "linux")) {
        return 1;
    }

    /* 5. Check for dumb terminal */
    if (strcmp(term, "dumb") == 0) {
        return 0;
    }

    /* Default: assume color support if we're a TTY */
    return 1;
}

static int detect_256color(void) {
    const char *term = getenv("TERM");
    if (!term) return 0;

    return strstr(term, "256color") != NULL;
}

static int detect_truecolor(void) {
    const char *colorterm = getenv("COLORTERM");
    if (!colorterm) return 0;

    return strstr(colorterm, "truecolor") != NULL ||
           strstr(colorterm, "24bit") != NULL;
}

void theme_init(const theme_config_t *config) {
    if (config) {
        memcpy(&g_theme, config, sizeof(g_theme));
    } else {
        /* Auto-detect everything */
        g_theme.is_terminal = isatty(STDOUT_FILENO);
        g_theme.use_colors = detect_color_support();
        g_theme.supports_256color = detect_256color();
        g_theme.supports_truecolor = detect_truecolor();
        g_theme.color_mode = g_theme.use_colors ? COLOR_AUTO : COLOR_NEVER;
    }
}

void theme_set_color_mode(color_mode_t mode) {
    g_theme.color_mode = mode;
    switch (mode) {
        case COLOR_NEVER:
            g_theme.use_colors = 0;
            break;
        case COLOR_ALWAYS:
            g_theme.use_colors = 1;
            break;
        case COLOR_AUTO:
            g_theme.use_colors = detect_color_support();
            break;
    }
}

void theme_set_colors(int enable) {
    g_theme.use_colors = enable;
    g_theme.color_mode = enable ? COLOR_ALWAYS : COLOR_NEVER;
}

int theme_using_colors(void) {
    return g_theme.use_colors;
}

int theme_is_terminal(void) {
    return g_theme.is_terminal;
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
        case STYLE_HIDDEN:    return ansi_hidden;
        case STYLE_STRIKETHROUGH: return ansi_strikethrough;
        default: return "";
    }
}

const char *theme_format(color_t fg, style_t style) {
    if (!g_theme.use_colors || fg == COLOR_RESET) {
        return "";
    }

    /* Build combined ANSI code: \033[fg;stylem */
    /* Example: \033[31;1m for bright red bold */
    g_format_buffer[0] = '\033';
    g_format_buffer[1] = '[';

    int pos = 2;

    /* Add foreground color */
    if (fg > 0 && fg < COLOR_COUNT) {
        /* Find the index in ansi_colors */
        /* Skip the reset code at index 0 */
        const char *code = ansi_colors[fg];
        /* Skip the ESC[ prefix and get the number */
        code += 2;  /* Skip "\033[" */

        while (*code != 'm' && *code != '\0' && pos < 30) {
            g_format_buffer[pos++] = *code++;
        }

        /* Add separator if we have styles */
        if (style != 0) {
            g_format_buffer[pos++] = ';';
        }
    }

    /* Add style codes */
    if (style & STYLE_BOLD) {
        const char *s = ansi_bold + 2;
        while (*s != 'm' && *s != '\0' && pos < 30) {
            g_format_buffer[pos++] = *s++;
        }
        if (style & ~STYLE_BOLD) g_format_buffer[pos++] = ';';
    }
    if (style & STYLE_DIM) {
        const char *s = ansi_dim + 2;
        while (*s != 'm' && *s != '\0' && pos < 30) {
            g_format_buffer[pos++] = *s++;
        }
        if (style & ~(STYLE_BOLD | STYLE_DIM)) g_format_buffer[pos++] = ';';
    }
    if (style & STYLE_UNDERLINE) {
        const char *s = ansi_underline + 2;
        while (*s != 'm' && *s != '\0' && pos < 30) {
            g_format_buffer[pos++] = *s++;
        }
        if (style & ~(STYLE_BOLD | STYLE_DIM | STYLE_UNDERLINE)) g_format_buffer[pos++] = ';';
    }
    if (style & STYLE_BLINK) {
        const char *s = ansi_blink + 2;
        while (*s != 'm' && *s != '\0' && pos < 30) {
            g_format_buffer[pos++] = *s++;
        }
        if (style & ~(STYLE_BOLD | STYLE_DIM | STYLE_UNDERLINE | STYLE_BLINK)) g_format_buffer[pos++] = ';';
    }
    if (style & STYLE_REVERSE) {
        const char *s = ansi_reverse + 2;
        while (*s != 'm' && *s != '\0' && pos < 30) {
            g_format_buffer[pos++] = *s++;
        }
    }

    g_format_buffer[pos++] = 'm';
    g_format_buffer[pos] = '\0';

    return g_format_buffer;
}

/* Category to color mapping */
color_t theme_color_for_category(int category) {
    switch (category) {
        case (1 << 0): /* CAT_PROCESS */
            return COLOR_CYAN;
        case (1 << 1): /* CAT_SIGNAL */
            return COLOR_RED;
        case (1 << 2): /* CAT_BACKTRACE */
            return COLOR_YELLOW;
        case (1 << 3): /* CAT_REGISTERS */
            return COLOR_GREEN;
        case (1 << 4): /* CAT_MEMORY */
            return COLOR_BLUE;
        case (1 << 5): /* CAT_BREAKPOINT */
            return COLOR_MAGENTA;
        default:
            return COLOR_WHITE;
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
