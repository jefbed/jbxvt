/*  Copyright 2016, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey,
    University of Kent at Canterbury.*/

#include "scr_string.h"

#include "config.h"
#include "cursor.h"
#include "jbxvt.h"
#include "libjb/log.h"
#include "libjb/util.h"
#include "paint.h"
#include "repaint.h"
#include "sbar.h"
#include "screen.h"
#include "scroll.h"
#include "scr_move.h"
#include "selection.h"

#include <string.h>
#include <unistd.h>

#define STRING_DEBUG
#ifndef STRING_DEBUG
#undef LOG
#define LOG(...)
#endif//!STRING_DEBUG

static bool tab_stops[JBXVT_MAX_COLS];

// Set tab stops:
// -1 clears all, -2 sets default
void jbxvt_set_tab(int16_t i, const bool value)
{
	if (i == -1) // clear all
		memset(&tab_stops, 0, JBXVT_MAX_COLS);
	else if (i == -2) // establish tab stop every 8 columns
		for (i = 0; i < JBXVT_MAX_COLS; ++i)
			tab_stops[i] = (i % 8 == 0);
	else if (i >= 0) // assign
		tab_stops[i] = value;
}

//  Tab to the next tab_stop.
void jbxvt_tab(void)
{
	LOG("jbxvt_tab()");
	jbxvt_set_scroll(0);
	struct JBDim c = jbxvt.scr.current->cursor;
	jbxvt.scr.current->text[c.y][c.x] = ' ';
	const uint16_t w = jbxvt.scr.chars.w - 1;
	while (!tab_stops[++c.x] && c.x < w)
		jbxvt.scr.current->text[c.y][c.x] = ' ';
	jbxvt.scr.current->cursor.x = c.x;
}

void jbxvt_cht(int16_t v)
{
	while (v-- > 0)
		jbxvt_tab();
}

static void handle_new_lines(int8_t nlcount)
{
	const int16_t y = jbxvt.scr.current->cursor.y;
	struct JBDim * m = &jbxvt.scr.current->margin;
	nlcount = y > m->b ? 0 : nlcount - m->b - y;
	JB_LIMIT(nlcount, y - m->top, 0);
	nlcount = MIN(nlcount, JBXVT_MAX_SCROLL);
	scroll(m->top, m->bottom, nlcount);
	jbxvt.scr.current->cursor.y -= nlcount;
}

static void decsclm(void)
{
	// Time value per the following:
	// http://www.vt100.net/docs/vt100-ug/chapter3.html166666
	if (jbxvt.mode.decsclm)
		usleep(166666);
}

static void wrap(void)
{
	jbxvt.scr.current->wrap_next = false;
	const struct JBDim m = jbxvt.scr.current->margin;
	const int16_t y = jbxvt.scr.current->cursor.y;
	jbxvt.scr.current->wrap[y] = true;
	if (y >= m.b) {
		decsclm();
		scroll(m.top, m.bottom, 1);
	} else
		++jbxvt.scr.current->cursor.y;
}

#if defined(__i386__) || defined(__amd64__)
       __attribute__((regparm(2)))
#endif//x86
static void handle_insert(const uint8_t n, const struct JBDim p)
{
	LOG("handle_insert(n=%d, p={%d, %d})", n, p.x, p.y);
	const struct JBDim c = jbxvt.scr.current->cursor;
	uint8_t * restrict s = jbxvt.scr.current->text[c.y];
	uint32_t * restrict r = jbxvt.scr.current->rend[c.y];
	const uint16_t sz = jbxvt.scr.chars.w - c.x;
	memmove(s + c.x + n, s + c.x, sz);
	memmove(r + c.x + n, r + c.x, sz << 2);
#define FSZ jbxvt.X.font.size
	const uint16_t n_width = n * FSZ.width;
	const uint16_t width = sz * FSZ.width - n_width;
	const int16_t x = p.x + n_width;
	xcb_copy_area(jbxvt.X.xcb, jbxvt.X.win.vt, jbxvt.X.win.vt,
		jbxvt.X.gc.tx, p.x, p.y, x, p.y, width, FSZ.height);
#undef FSZ
}

static void parse_special_charset(uint8_t * restrict str,
	const uint8_t len)
{
	LOG("CHARSET_SG0");
	for (int_fast16_t i = len ; i >= 0; --i) {
		uint8_t * ch = &str[i];
		switch (*ch) {
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 't':
		case 'u':
			*ch = '+';
			break;
		case 'q':
			*ch = '-';
			break;
		case 'x':
			*ch = '|';
			break;
		}
	}
}

static void fix_margins(struct JBDim* restrict m,
	const int16_t cursor_y)
{
	m->b = MAX(m->b, cursor_y);
	const uint8_t h = jbxvt.scr.chars.height - 1;
	m->b = MIN(m->b, h);
}

static void fix_cursor(struct JBXVTScreen * restrict c)
{
	JB_LIMIT(c->cursor.y, jbxvt.scr.chars.height - 1, 0);
	JB_LIMIT(c->cursor.x, jbxvt.scr.chars.width - 1, 0);
	fix_margins(&c->margin, c->cursor.y);
}

static bool test_action_char(const uint8_t c,
	struct JBXVTScreen * restrict s)
{
	switch(c) {
	case '\r':
		s->cursor.x = 0;
		s->wrap_next = false;
		return true;
	case '\n':
		wrap();
		return true;
	case '\t':
		jbxvt_tab();
		return true;
	}
	return false;
}

static void save_render_style(const int_fast16_t n,
	struct JBXVTScreen * restrict s)
{
	const struct JBDim c = s->cursor;
	for (int_fast16_t i = n - 1; i >= 0; --i)
		  s->rend[c.y][c.x + i] = jbxvt.scr.rstyle;
}

static void check_wrap(struct JBXVTScreen * restrict s)
{
	const uint16_t w = jbxvt.scr.chars.w;
	if (s->cursor.x >= w)
		s->wrap_next = !jbxvt.mode.decawm;
}

/*  Display the string at the current position.
    nlcount is the number of new lines in the string.  */
void jbxvt_string(uint8_t * restrict str, uint8_t len, int8_t nlcount)
{
	LOG("jbxvt_string(%s, len: %d, nlcount: %d)", str, len, nlcount);
	jbxvt_set_scroll(0);
	jbxvt_draw_cursor();
	if (nlcount > 0)
		  handle_new_lines(nlcount);
	struct JBDim p;
	fix_cursor(&jbxvt.scr.s[0]);
	fix_cursor(&jbxvt.scr.s[1]);
	bool double_width_sent = false;
	while (len) {
		if (test_action_char(*str, jbxvt.scr.current)) {
			--len;
			++str;
			continue;
		}
		struct JBDim * c = &jbxvt.scr.current->cursor;
		if (jbxvt.scr.current->wrap_next) {
			wrap();
			c->x = 0;
		}
		jbxvt_check_selection(c->y, c->y);
		p = jbxvt_get_pixel_size(jbxvt.scr.current->cursor);
		if (unlikely(jbxvt.mode.insert))
			handle_insert(1, p);
		uint8_t * t = jbxvt.scr.current->text[c->y];
		if (!t) return;
		t += c->x;
		if (jbxvt.mode.charset[jbxvt.mode.charsel] > CHARSET_ASCII)
			parse_special_charset(str, len);
		// Render the string:
		if (!jbxvt.scr.current->decpm) {
#ifdef STRING_DEBUG
			if (jbxvt.scr.current->dwl[c->y])
				LOG("\t\tDOUBLE_WIDTH_LINE");
#endif
			paint_rstyle_text(str, jbxvt.scr.rstyle, 1, p,
				jbxvt.scr.current->dwl[c->y]);
			// Save scroll history:
			*t = *str;
		}
		save_render_style(1, jbxvt.scr.current);
		--len;
		++str;
		++c->x;
		check_wrap(jbxvt.scr.current);
	}
	jbxvt_draw_cursor();
	if (double_width_sent)
		jbxvt_repaint();
}


