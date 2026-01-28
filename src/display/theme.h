/**
 * @file theme.h
 * @brief Color detection and ANSI codes
 */

#ifndef THEME_H
#define THEME_H

/* Color codes */
typedef enum {
    COLOR_RESET = 0,
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
} color_t;

/* Style modifiers */
typedef enum {
    STYLE_BOLD      = (1 << 0),
    STYLE_DIM       = (1 << 1),
    STYLE_UNDERLINE = (1 << 2),
    STYLE_BLINK     = (1 << 3),
    STYLE_REVERSE   = (1 << 4)
} style_t;

/* Theme configuration */
typedef struct {
    int use_colors;
    int is_terminal;
} theme_config_t;

/* Initialize theme (detect terminal capabilities) */
void theme_init(theme_config_t *config);

/* Enable/disable colors */
void theme_set_colors(int enable);

/* Colored output functions */
const char *theme_color(color_t fg);
const char *theme_color_bg(color_t bg);
const char *theme_style(style_t style);
const char *theme_reset(void);

/* Box drawing characters (ASCII) */
typedef struct {
    const char *h;       /* horizontal */
    const char *v;       /* vertical */
    const char *tl;      /* top-left */
    const char *tr;      /* top-right */
    const char *bl;      /* bottom-left */
    const char *br;      /* bottom-right */
    const char *branch_t; /* tree branch (not last) */
    const char *branch_l; /* tree branch (last) */
} box_style_t;

/* Predefined styles */
extern const box_style_t box_ascii;
extern const box_style_t box_compact;

/* Helpers */
void theme_print_box(const box_style_t *style, const char *title, int width);
void theme_print_separator(const char *ch, int length);

#endif /* THEME_H */
