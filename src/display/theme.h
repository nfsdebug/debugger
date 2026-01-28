/**
 * @file theme.h
 * @brief Color detection and ANSI codes
 */

#ifndef THEME_H
#define THEME_H

/* Color codes - 16 colors (8 normal + 8 bright) */
typedef enum {
    COLOR_RESET = 0,
    /* Normal colors (30-37) */
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    /* Bright colors (90-97) */
    COLOR_BRIGHT_BLACK,
    COLOR_BRIGHT_RED,
    COLOR_BRIGHT_GREEN,
    COLOR_BRIGHT_YELLOW,
    COLOR_BRIGHT_BLUE,
    COLOR_BRIGHT_MAGENTA,
    COLOR_BRIGHT_CYAN,
    COLOR_BRIGHT_WHITE,
    COLOR_COUNT
} color_t;

/* Style modifiers (can be combined with OR) */
typedef enum {
    STYLE_BOLD      = (1 << 0),
    STYLE_DIM       = (1 << 1),
    STYLE_UNDERLINE = (1 << 2),
    STYLE_BLINK     = (1 << 3),
    STYLE_REVERSE   = (1 << 4),
    STYLE_HIDDEN    = (1 << 5),
    STYLE_STRIKETHROUGH = (1 << 6)
} style_t;

/* Color detection result */
typedef enum {
    COLOR_NEVER,    /* Colors disabled (NO_COLOR set, or redirected) */
    COLOR_AUTO,     /* Auto-detected based on terminal */
    COLOR_ALWAYS    /* Force colors */
} color_mode_t;

/* Theme configuration */
typedef struct {
    color_mode_t color_mode;
    int use_colors;
    int is_terminal;
    int supports_256color;
    int supports_truecolor;
} theme_config_t;

/* Initialize theme (detect terminal capabilities) */
void theme_init(theme_config_t *config);

/* Set color mode */
void theme_set_color_mode(color_mode_t mode);

/* Enable/disable colors */
void theme_set_colors(int enable);

/* Get current theme state */
int theme_using_colors(void);
int theme_is_terminal(void);

/* Build combined style string (color + modifiers) */
/* Result is valid until next call to theme_format() */
const char *theme_format(color_t fg, style_t style);

/* Colored output functions */
const char *theme_color(color_t fg);
const char *theme_color_bg(color_t bg);
const char *theme_style(style_t style);
const char *theme_reset(void);

/* Color mapping for categories */
color_t theme_color_for_category(int category);

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
