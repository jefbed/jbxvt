/*  Copyright 2016, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey, University of Kent at Canterbury.*/

#include "scr_erase.h"

#include "config.h"
#include "cursor.h"
#include "jbxvt.h"
#include "libjb/log.h"
#include "sbar.h"
#include "screen.h"
#include "scroll.h"
#include "scr_reset.h"
#include "selection.h"

#include <string.h>

//#define DEBUG_ERASE
#ifndef DEBUG_ERASE
#undef LOG
#define LOG(...)
#endif

#define SCR jbxvt.scr.current
#define CUR SCR->cursor
#define CSZ jbxvt.scr.chars
#define PSZ jbxvt.scr.pixels
#define FSZ jbxvt.X.f.size
#define FH FSZ.h

static void zero(const int16_t line, const uint16_t sz, const int16_t col)
{
	memset(SCR->text[line] + col, 0, sz);
	memset(SCR->rend[line] + col, 0, sz << 2);
	SCR->wrap[line] = false;

}

static void erase_range(xcb_rectangle_t * restrict h,
	const uint16_t sz, const uint16_t col)
{
	h->x = col * FSZ.w;
	h->width = sz * FSZ.w;
	zero(CUR.y, sz, col);
}

static int16_t get_col(const uint8_t mode)
{
	return mode == 0 ? CUR.x : 0;
}

static int16_t get_sz(const uint8_t mode)
{
	switch(mode) {
	case 0:
		return CSZ.w - CUR.x;
	case 2:
		return CSZ.w;
	case 1:
	default:
		return CUR.x;
	}
}

//  erase part or the whole of a line
void scr_erase_line(const int8_t mode)
{
	LOG("scr_erase_line(%d)", mode);
	change_offset(0);
	VTScreen * scr = jbxvt.scr.current;
	struct JBDim c = scr->cursor;
	const uint8_t fh = jbxvt.X.f.size.height;
	xcb_rectangle_t g = { .y = c.y * fh };
	erase_range(&g, get_sz(mode), get_col(mode));
	draw_cursor(); //clear
	check_selection(c.y, c.y);
	xcb_clear_area(jbxvt.X.xcb, 0, jbxvt.X.win.vt,
		g.x, g.y, g.width, fh);
	scr->wrap_next = false;
	draw_cursor();
	xcb_flush(jbxvt.X.xcb);
}

static void common_scr_erase(const xcb_rectangle_t r,
	const int16_t row1, const int16_t row2, const int8_t mode)
{
	check_selection(row1, row2);
	xcb_clear_area(jbxvt.X.xcb, 0, jbxvt.X.win.vt, r.x, r.y,
		r.width, r.height);
	scr_erase_line(mode);
}

//  erase part or the whole of the screen
void scr_erase_screen(const int8_t mode)
{
	LOG("scr_erase_screen(%d)", mode);
	change_offset(0);
	VTScreen * s = jbxvt.scr.current;
	s->wrap_next = 0;
	xcb_rectangle_t r = {.width = PSZ.width};
	const struct JBDim cur = s->cursor;
	switch (mode) {
	case 1:
		LOG("START");
		r.height = cur.y * FH;
		for (uint8_t i = 0; i < cur.y; ++i)
			zero(i, CSZ.w, 0);
		common_scr_erase(r, 0, cur.y - 1, mode);
		break;
	case 0:
		LOG("END");
		if (cur.y || cur.x) {
			const int16_t c1 = cur.y + 1;
			r.y += c1 * FH;
			r.height = (CSZ.height - cur.y - 1) * FH;
			for (uint8_t i = c1; i < CSZ.height; ++i)
				zero(i, CSZ.w, 0);
			common_scr_erase(r, c1, CSZ.height - 1, mode);
			break;
		}
		/*  If we are positioned at the top left hand corner then
		 *  it is effectively a whole screen clear.
		 *  Drop through so that we do not need to duplicate
		 *  the scroll-up code.  */
	case 2:
	case 3: // for linux console compatibility
		LOG("ENTIRE");
		r.height = CSZ.h - 1;
		scroll(0, r.height, r.height);
		common_scr_erase(r, 0, r.height, mode);
	}
}

