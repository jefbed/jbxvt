/*  Copyright 2016, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey,
    University of Kent at Canterbury.*/
#include "repaint.h"
#include "display.h"
#include "jbxvt.h"
#include "paint.h"
#include "show_selection.h"
#include "window.h"
#define CSZ jbxvt.scr.chars
#define FSZ jbxvt_get_font_size()
/* Display the string using the rendition vector
   at the screen coordinates.  */
static void paint_rvec_text(xcb_connection_t * xc,
	uint8_t * str, uint32_t * rvec,
	int16_t len, struct JBDim p, const bool dwl)
{
	if (!rvec || !str)
		  return;
	while (len > 0) {
		uint32_t r;
		int_fast16_t i;
		/* Find the length for which
		   the current rend val applies.  */
		for (i = 0, r = *rvec; i < len && rvec[i] == r; ++i)
			;
		// draw
		jbxvt_paint(xc, str, r, i, p, dwl);
		// advance to next block
		p.x += i * FSZ.width;
		str += i;
		rvec += i;
		len -= i;
	}
}
static int_fast32_t repaint_generic(xcb_connection_t * xc,
	struct JBDim p, uint_fast16_t len,
	uint8_t * restrict str, uint32_t * rend, const bool dwl)
{
	// check inputs:
	if (!str || !len)
		return p.y + FSZ.height;
	if (rend)
		paint_rvec_text(xc, str, rend + 0, len, p, dwl);
	else
		jbxvt_paint(xc, str, 0, len, p, dwl);
	p.x += len * FSZ.width;
	const uint16_t width = (CSZ.width + 1 - len) * FSZ.width;
	xcb_clear_area(xc, false, jbxvt_get_vt_window(xc), p.x, p.y,
		width, FSZ.height);
	return p.y + FSZ.height;
}
__attribute__((nonnull(1)))
static int_fast16_t show_scroll_history(xcb_connection_t * xc,
	struct JBDim * restrict p, const int_fast16_t line,
	const int_fast16_t i)
{
	if (line > CSZ.h || i < 0)
		return line;
	struct JBXVTSavedLine * sl = &jbxvt.scr.sline.data[i];
	p->y = repaint_generic(xc, *p, sl->size, sl->text,
		sl->rend, sl->dwl);
	return show_scroll_history(xc, p, line + 1, i - 1);
}
__attribute__((nonnull(1)))
static uint_fast16_t filter_string(uint8_t * restrict buf,
	uint8_t * restrict input)
{
	if (!input)
		return 0;
	uint_fast16_t x;
	for (x = 0; x < CSZ.width; ++x)
		buf[x] = input[x] < ' ' ? ' ' : input[x];
	return x;
}
// Repaint the screen
void jbxvt_repaint(xcb_connection_t * xc)
{
	//  First do any 'scrolled off' lines that are visible.
	struct JBDim p = {};
	int_fast32_t line = show_scroll_history(xc,
		&p, 0, jbxvt.scr.offset - 1);
	// Do the remainder from the current screen:
	for (uint_fast16_t i = 0; line < CSZ.height; ++line, ++i) {
		// Allocate enough space to process each column
		uint8_t str[CSZ.width];
		p.y = repaint_generic(xc, p, filter_string(str,
			jbxvt.scr.current->text[i]), str,
			jbxvt.scr.current->rend[i],
			jbxvt.scr.current->dwl[i]);
	}
	jbxvt_show_selection(xc);
}
