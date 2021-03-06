/*  Copyright 2017, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey, University of Kent at Canterbury.*/
#ifndef JBXVT_CONFIG_H
#define JBXVT_CONFIG_H
// Overall release number of the current version:
#define JBXVT_VERSION "0.9"
#define JBXVT_NORMAL_FONT \
    "lucidasanstypewriter-12"
#define JBXVT_BOLD_FONT \
    "lucidasanstypewriter-bold-12"
#define JBXVT_ITALIC_FONT \
    "lucidasanstypewriter-bold-12"
/* FIXME:  Even though we provide a jbxvt terminfo entry, mouse tracking in
 * vim does not work with it.  Set TERM to xterm before launching vim or fix
 * vim.  */
#ifdef USE_JBXVT_TERMCAP
#if !defined(FREEBSD)
#define JBXVT_ENV_TERM "xterm-jbxvt"
#else//FREEBSD
#define JBXVT_ENV_TERM "xterm"
#endif//!FREEBSD
#define JBXVT_ENV_TERM "xterm-jbxvt"
#else //!USE_JBXVT_TERMCAP
#ifdef USE_JBXVT_XTERM_256
#define JBXVT_ENV_TERM "xterm-256color"
#else//!USE_JBXVT_XTERM_256
#ifdef USE_JBXVT_VT100
#define JBXVT_ENV_TERM "vt100"
#else//!USE_JBXVT_VT100
#define JBXVT_ENV_TERM "xterm"
#endif//USE_JBXVT_VT100
#endif//USE_JBXVT_XTERM_256
#endif//USE_JBXVT_TERMCAP
#define JBXVT_ENV_SHELL "SHELL"
#define JBXVT_FOREGROUND_COLOR "white"
#define JBXVT_BACKGROUND_COLOR "black"
enum {    JBXVT_SCROLLBAR_WIDTH = 8, // width of scroll bar
    JBXVT_MARGIN = 1, // gap between the text and the window edges
    JBXVT_ROWS = 24, // default # rows
    JBXVT_COLUMNS = 80, // default # columns
    JBXVT_MAX_SCROLL = 1000,
    // Set these to fit your display :
    JBXVT_MAX_COLUMNS = 170, // max columns on screen
    JBXVT_MAX_ROWS = 60, // max rows on screen
    JBXVT_PROP_SIZE = 1024, // selection property chunk size
    JBXVT_SEL_KEY_DEL = 2000, // ms for kb input before selection
    JBXVT_DEFAULT_CURSOR_ATTR = 2, // cursor style (solid block)
    // The following value per the following:
    // http://www.vt100.net/docs/vt100-ug/chapter3.html166666
    JBXVT_SOFT_SCROLL_DELAY = 166,
};
#endif//!JBXVT_CONFIG_H
