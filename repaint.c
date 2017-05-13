/*  Copyright 2017, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey,
    University of Kent at Canterbury.*/
#include "repaint.h"
#include "JBXVTScreen.h"
#include "config.h"
#include "font.h"
#include "gc.h"
#include "libjb/JBDim.h"
#include "paint.h"
#include "sbar.h"
#include "screen.h"
#include "scroll.h"
#include "show_selection.h"
#include "size.h"
#include "window.h"
static uint_fast16_t get_render_length(const rstyle_t * rvec,
	const uint16_t len)
{
	uint_fast16_t i = 0;
	while (i < len && rvec[i] == rvec[0])
		++i;
	return i;
}
// Display the string using the render vector at the screen
// coordinates.
static void paint_rvec_text(xcb_connection_t * restrict xc,
	uint8_t * restrict str, const rstyle_t * restrict rvec, uint16_t len,
	struct JBDim p, const bool dwl)
{
	const struct JBDim f = jbxvt_get_font_size();
	for (int i; len; len -= i, str += i, rvec += i, p.x += i *
		f.width)
		jbxvt_paint(xc, str, *rvec, i = get_render_length(rvec, len),
			p, dwl);
}
static uint8_t * filter(uint8_t * restrict t, register int_fast16_t i)
{
	while (--i >= 0)
		if (t[i] < ' ')
			t[i] = ' ';
	return t;
}
static void paint(xcb_connection_t * xc, struct JBXVTLine * l,
	const struct JBDim p)
{
	const uint16_t w = jbxvt_get_char_size().width;
	paint_rvec_text(xc, filter(l->text, w), l->rend, w, p, l->dwl);
}
static int show_history(xcb_connection_t * restrict xc, const int line,
	const int top, struct JBDim * restrict p,
	const struct JBDim font_size, const struct JBDim char_size)
{
	const uint16_t ss = jbxvt_get_scroll_size();
	const uint8_t h = char_size.height;
	/* Check  top + h vs ss so that the following pointer
	 * arithmetic does not go outside array bounds.  */
	if (top > ss)
		return top;
	/* This is the normal return condition of this recursive
	 * function:  */
	if (line >= h || top < 0)
		return line;
	/* Use screen character height as an offset into the scroll
	 * history buffer, as indicated by variable h.  Use -1 to
	 * convert size ss into an index.  Use top as the iterator.
	 * */
	struct JBXVTLine * l = jbxvt_get_saved_lines() + ss - top - 1;
	paint(xc, l, *p);
	p->y += font_size.height;
	return show_history(xc, line + 1, top - 1, p, font_size, char_size);
}
// Repaint the screen
void jbxvt_repaint(xcb_connection_t * xc)
{
	// First do any 'scrolled off' lines that are visible.
	struct JBDim p = {{0},{0}};
	const struct JBDim chars = jbxvt_get_char_size(),
	      f = jbxvt_get_font_size();
	if (chars.rows >= JBXVT_MAX_ROWS)
		return; // invalid screen size, go no further.
	/* Subtract 1 from scroll offset to get index.  */
	int line = show_history(xc, 0, jbxvt_get_scroll() - 1, &p, f, chars);
	// Initialize onscreen_line here to save p.y:
	const struct JBDim ps = jbxvt_chars_to_pixels(chars);
	xcb_point_t onscreen_line[] = {{.y = p.y - 1},
		{.x = ps.x, .y = p.y - 1}};
	// Do the remainder from the current screen:
	struct JBXVTScreen * s = jbxvt_get_current_screen();
	for (uint_fast16_t i = 0; line < chars.height;
		++line, ++i, p.y += f.h)
		paint(xc, s->line + i, p);
	xcb_poly_line(xc, XCB_COORD_MODE_ORIGIN, jbxvt_get_vt_window(xc),
		jbxvt_get_cursor_gc(xc), 2, onscreen_line);
	jbxvt_show_selection(xc);
}
