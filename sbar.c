/*  Copyright 2017, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey,
    University of Kent at Canterbury.*/
#include "sbar.h"
#include "config.h"
#include "cursor.h"
#include "gc.h"
#include "libjb/JBDim.h"
#include "libjb/macros.h"
#include "repaint.h"
#include "scroll.h"
#include "size.h"
#include "window.h"
#include "xcb_id_getter.h"
static int16_t sbar_offset; // how far up scrollbar is positioned
static bool sbar_visible;
XCB_ID_GETTER(jbxvt_get_scrollbar);
int16_t jbxvt_get_scroll(void)
{
    return sbar_offset;
}
bool jbxvt_get_scrollbar_visible(void)
{
    return sbar_visible;
}
__attribute__((pure))
static int16_t get_sz(const uint16_t margin)
{
    const uint16_t ph = jbxvt_get_pixel_size().h;
    return ph - ph * (sbar_offset + margin)
        / (jbxvt_get_scroll_size() + jbxvt_get_char_size().h);
}
// Draw the scrollbar.
void jbxvt_draw_scrollbar(xcb_connection_t * xc)
{
    xcb_clear_area(xc, 0, jbxvt_get_scrollbar(xc), 0, 0,
        JBXVT_SCROLLBAR_WIDTH, jbxvt_get_pixel_size().h);
    const int16_t top = get_sz(jbxvt_get_char_size().h);
    xcb_poly_fill_rectangle(xc, jbxvt_get_scrollbar(xc),
        jbxvt_get_text_gc(xc), 1, &(xcb_rectangle_t){0, top,
        JBXVT_SCROLLBAR_WIDTH, get_sz(0) - top});
}
//  Change the value of the scrolled screen offset and repaint the screen
void jbxvt_set_scroll(xcb_connection_t * xc, int16_t n)
{
    JB_LIMIT(n, jbxvt_get_scroll_size(), 0);
    if (n != sbar_offset) {
        sbar_offset = n;
        jbxvt_repaint(xc);
        jbxvt_draw_cursor(xc);
        jbxvt_draw_scrollbar(xc);
    }
}
// Scroll to the specified y position (in pixels)
void jbxvt_scroll_to(xcb_connection_t * xc, const int16_t y)
{
    const uint8_t ch = jbxvt_get_char_size().h;
    const uint16_t ph = jbxvt_get_pixel_size().h;
    jbxvt_set_scroll(xc, (ch + jbxvt_get_scroll_size())
        * (ph - y) / ph - ch);
}
void jbxvt_clear_saved_lines(xcb_connection_t * xc)
{
    jbxvt_clear_scroll_history();
    jbxvt_set_scroll(xc, 0);
}
void jbxvt_toggle_scrollbar(xcb_connection_t * xc)
{
    xcb_configure_window(xc, jbxvt_get_vt_window(xc),
        XCB_CONFIG_WINDOW_X, &(uint32_t){(sbar_visible^=true)
        ? JBXVT_SCROLLBAR_WIDTH : 0});
}
