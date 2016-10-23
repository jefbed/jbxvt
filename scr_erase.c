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
#define DEBUG_ERASE
#ifndef DEBUG_ERASE
#undef LOG
#define LOG(...)
#endif//DEBUG_ERASE
#define FSZ jbxvt.X.f.size
static void del(uint16_t col, uint16_t width)
{
	const uint16_t y = jbxvt.scr.current->cursor.y;
	struct JBXVTScreen * s = jbxvt.scr.current;
	if (col + width > jbxvt.scr.chars.width) // keep in screen
		width = jbxvt.scr.chars.width - col;
	memset(s->text[y] + col, 0, width);
	memset(s->rend[y] + col, 0, width << 2);
	xcb_clear_area(jbxvt.X.xcb, 0, jbxvt.X.win.vt, col * FSZ.w,
		y * FSZ.h, width * FSZ.w, FSZ.h);
	xcb_flush(jbxvt.X.xcb);
	s->wrap[y] = false;
	s->dwl[y] = false;
}
//  erase part or the whole of a line
void jbxvt_erase_line(const int8_t mode)
{
	jbxvt_set_scroll(0);
	const uint16_t x = jbxvt.scr.current->cursor.x;
	switch (mode) {
	case JBXVT_ERASE_ALL:
		del(0, jbxvt.scr.chars.width);
		break;
	case JBXVT_ERASE_BEFORE:
		del(0, x);
		break;
	case JBXVT_ERASE_AFTER:
	default:
		del(x, jbxvt.scr.chars.width - x);
	}
	jbxvt_draw_cursor();
}
//  erase part or the whole of the screen
void jbxvt_erase_screen(const int8_t mode)
{
	LOG("jbxvt_erase_screen(mode=%d)", mode);
	uint16_t start, end;
	switch (mode) {
		// offset by 1 to not include current line, handled later
	case JBXVT_ERASE_AFTER:
		start = jbxvt.scr.current->cursor.y + 1;
		end = jbxvt.scr.chars.h;
		break;
	case JBXVT_ERASE_BEFORE:
		start = 0;
		end = jbxvt.scr.current->cursor.y - 1;
		break;
	case JBXVT_ERASE_SAVED:
		jbxvt_clear_saved_lines();
		return;
	default: // JBXVT_ERASE_ALL
		start = 0;
		end = jbxvt.scr.chars.h - 1;
		break;
	}
	/* Save cursor y locally instead of using save/restore cursor
	   functions in order to avoid side-effects on applications
	   using a saved cursor position.  */
	const uint16_t old_y = jbxvt.scr.current->cursor.y;
	for (uint_fast16_t l = start; l <= end; ++l) {
		jbxvt.scr.current->cursor.y = l;
		jbxvt_erase_line(JBXVT_ERASE_ALL); // entire
		jbxvt_draw_cursor();
	}
	jbxvt.scr.current->cursor.y = old_y;
	// clear start of, end of, or entire current line, per mode
	jbxvt_erase_line(mode);
}
