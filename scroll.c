/*  Copyright 2017, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey,
    University of Kent at Canterbury.*/
#include "scroll.h"
#include "JBXVTLine.h"
#include "JBXVTScreen.h"
#include "font.h"
#include "gc.h"
#include "libjb/JBDim.h"
#include "libjb/util.h"
#include "sbar.h"
#include "screen.h"
#include "selection.h"
#include "size.h"
#include "window.h"
#include <stdlib.h>
#include <string.h>
static struct JBXVTLine * saved_lines;
static int16_t scroll_size;
struct JBXVTLine * jbxvt_get_saved_lines(void)
{
    return saved_lines;
}
void jbxvt_clear_scroll_history(void)
{
    free(saved_lines);
    saved_lines = NULL;
    scroll_size = 0;
}
int16_t jbxvt_get_scroll_size(void)
{
    return scroll_size;
}
static void clear_selection_at(const int16_t j)
{
    struct JBDim * e = jbxvt_get_selection_end_points();
    if (e[0].index == j || e[1].index == j)
        jbxvt_clear_selection();
}
static inline void move_line(const int16_t source, const int16_t count,
    struct JBXVTLine * line_array)
{
    const int16_t dest = source + count;
    memcpy(line_array + dest , line_array + source,
        sizeof(struct JBXVTLine));
    clear_selection_at(source);
}
static inline uint8_t get_y(int16_t * y, const int16_t row1,
    const int16_t count, const bool up)
{
    const uint8_t fh = jbxvt_get_font_size().height;
    const int16_t a = row1 * fh;
    y[up ? 1 : 0] = a;
    y[up ? 0 : 1] = count * fh + a;
    return fh;
}
static void copy_visible_area(xcb_connection_t * xc, const int r1,
    const int r2, const int count, const bool up)
{
    int16_t y[2];
    const uint8_t fh = get_y(y, r1, count, up);
    const xcb_window_t vt = jbxvt_get_vt_window(xc);
    xcb_copy_area(xc, vt, vt, jbxvt_get_text_gc(xc), 0,
        y[0], 0, y[1], jbxvt_get_pixel_size().width,
        (r2 - r1 - count) * fh);
}
// Restrict scroll history size to JBXVT_MAX_SCROLL:
static void trim(void)
{
    enum { SZ = sizeof(struct JBXVTLine), LIM = JBXVT_MAX_SCROLL };
    // Only work when scroll_size is twice JBXVT_MAX_SCROLL
    if (scroll_size < LIM << 1)
        return;
    struct JBXVTLine * new = malloc(LIM * SZ), * i;
#ifdef DEBUG
    jb_require(new, "Cannot allocate memory for new scroll"
        " history buffer");
#endif // DEBUG
    const int16_t diff = scroll_size - LIM;
    i = saved_lines + diff;
    memcpy(new, i, LIM * SZ);
    free(saved_lines);
    saved_lines = new;
    scroll_size = LIM;
}
static void add_scroll_history(void)
{
    struct JBXVTScreen * s = jbxvt_get_current_screen();
    enum { SIZE = sizeof(struct JBXVTLine) };
    { // sz scope
        const size_t sz = ((size_t)++scroll_size * SIZE);
        saved_lines = realloc(saved_lines, sz);
    }
#ifdef DEBUG
    jb_require(saved_lines, "Cannot allocate memory"
        " for scroll history");
#endif // DEBUG
    // - 1 for index instead of size
    memcpy(&saved_lines[scroll_size - 1], &s->line[s->cursor.y], SIZE);
    trim();
}
static int16_t copy_lines(const int16_t i, const int16_t j, const int16_t mod,
    const int16_t count)
{
    if (i >= count)
        return j;
    struct JBXVTScreen * s = jbxvt_get_current_screen();
    struct JBXVTLine * dest = s->line + i,
             * src = s->line + j;
    memcpy(dest, src, sizeof(struct JBXVTLine));
    return copy_lines(i + 1, j + mod, mod, count);
}
static inline void clear_line(xcb_connection_t * xc,
    const int16_t y, const int16_t count)
{
    const uint8_t fh = jbxvt_get_font_size().height;
    xcb_clear_area(xc, 0, jbxvt_get_vt_window(xc), 0, y * fh,
        jbxvt_get_pixel_size().width, count * fh);
}
static void clear(const int16_t count, const int16_t offset, const bool is_up)
{
    if (count >= 0) {
        const int16_t j = offset + (is_up ? - count - 1 : count);
        memset(jbxvt_get_current_screen()->line + j, 0,
            sizeof(struct JBXVTLine));
        clear(count - 1, offset, is_up);
    }
}
struct ScrollData {
    xcb_connection_t * connection;
    int16_t begin;
    int16_t end;
    int16_t count;
    bool up;
};
static void sc_common(struct ScrollData * d)
{
    if (d->up) // call this way to only have one branch
        clear(d->count - 1, d->end, true);
    else
        clear(d->count - 1, d->begin, false);
    xcb_connection_t * xc = d->connection;
    copy_visible_area(xc, d->begin, d->end, d->count, d->up);
    clear_line(d->connection, d->up ? (d->end - d->count)
        : d->begin, d->count);
    jbxvt_draw_scrollbar(xc);
}
static void sc_dn(struct ScrollData * d)
{
    struct JBXVTScreen * s = jbxvt_get_current_screen();
    for(int16_t j = copy_lines(0, d->end, -1, d->count);
        j >= d->begin; --j)
        move_line(j, d->count, s->line);
    sc_common(d);
}
static void sc_up(struct ScrollData * d)
{
    struct JBXVTScreen * s = jbxvt_get_current_screen();
    if (s == jbxvt_get_screen_at(0) && d->begin < 1)
        add_scroll_history();
    for(int16_t j = copy_lines(0, d->begin, 1, d->count);
        j < d->end; ++j)
        move_line(j, -d->count, s->line);
    sc_common(d);
}
/*  Scroll count lines from row1 to row2 inclusive.
    row1 should be <= row2.  Scrolling is up for a positive count and
    down for a negative count.  count is limited to a maximum of
    SCROLL lines.  */
void scroll(xcb_connection_t * xc, const int16_t row1,
    const int16_t row2, const int16_t count)
{
    if (count && row1 < row2) {
        struct ScrollData sd;
        sd.connection = xc;
        sd.begin = row1;
        sd.end = row2 + 1;
        sd.count = abs(count);
        sd.up = count > 0;
        if (sd.up)
            sc_up(&sd);
        else
            sc_dn(&sd);
        jbxvt_set_scroll(xc, 0);
    }
}
