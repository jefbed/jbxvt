/*  Copyright 2016, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey, University of Kent at Canterbury.*/

#include "scr_reset.h"

#include "config.h"
#include "cursor.h"
#include "jbxvt.h"
#include "libjb/log.h"
#include "repaint.h"
#include "sbar.h"
#include "scr_move.h"
#include "scroll.h"

#include <gc.h>
#include <unistd.h>

// Shortcuts
#define S jbxvt.scr
#define S0 jbxvt.scr.s[0]
#define S1 jbxvt.scr.s[1]
#define P S.pixels
#define X jbxvt.X
#define FSZ X.f.size

static void init_screen_elements(VTScreen * restrict scr,
	uint8_t ** restrict text, uint32_t ** restrict rend)
{
	scr->margin.bottom = S.chars.height - 1;
	scr->wrap_next = false;
	scr->rend = rend;
	scr->text = text;
	scr->margin.top = 0;
}

__attribute__((pure))
static struct JBSize8 get_cdim(const struct JBSize16 d)
{
	// Take advantage of division rounding down for mod
	return (struct JBSize8) {.w = (d.w - MARGIN)/FSZ.w,
		.h = (d.h - MARGIN)/FSZ.h};
}

static void set(const uint8_t i, uint8_t ** t, uint32_t ** r)
{
	S.s[i].text = t;
	S.s[i].rend = r;
}

static void init(void)
{
	uint8_t **s0, **s1;
	uint32_t **r0, **r1;
	uint16_t sz = JBXVT_MAX_ROWS * sizeof(void *);
#define ALLOC(v) {v = GC_MALLOC(sz);}
	ALLOC(s0); ALLOC(s1); ALLOC(r0); ALLOC(r1);
	for (int_fast16_t y = JBXVT_MAX_ROWS - 1; y >= 0; --y) {
		sz = JBXVT_MAX_COLS;
		ALLOC(s0[y]); ALLOC(s1[y]);
		sz <<= 2;
		ALLOC(r0[y]); ALLOC(r1[y]);
	}
	set(0, s0, r0);
	set(1, s1, r1);
	sz = JBXVT_MAX_ROWS;
	ALLOC(S0.wrap);
	ALLOC(S1.wrap);
}

static inline void fix_margins(const struct JBSize8 c)
{
	/* On screen resize, check if old margin was on the bottom line.
	   If so, set the bottom margin to the new bottom line.  */
	if (c.height == S.chars.height)
		  return;
	VTScreen * restrict s = S.current;
	if (s->margin.b >= c.h)
		  s->margin.b = c.h - 1;
}
static void clear(void)
{
	xcb_clear_area(X.xcb, false, X.win.vt, 0, 0, P.w, P.h);
	change_offset(0);
	xcb_flush(X.xcb);
}

static void decscnm(void)
{
	static bool last_was_rv;
	const bool rv = jbxvt.mode.decscnm;
	if (last_was_rv == rv) // Already has either mode set
		return;
	else
		last_was_rv = rv;
	LOG("decscnm()");
	struct JBXVTXPixels * p = &X.color;
	SWAP(pixel_t, p->fg, p->bg);
	SWAP(pixel_t, p->current_fg, p->current_bg);
	xcb_change_gc(X.xcb, X.gc.tx, XCB_GC_FOREGROUND
		| XCB_GC_BACKGROUND, (uint32_t[]){p->fg, p->bg});
	xcb_change_window_attributes(X.xcb, X.win.vt,
		XCB_CW_BACK_PIXEL, &p->bg);
	usleep(100000);
}

/*  Reset the screen - called whenever the screen
    needs to be repaired completely.  */
void scr_reset(void)
{
	LOG("scr_reset()");
	decscnm();
	struct JBSize8 c = get_cdim(P);
	fix_margins(c);
	static bool created;
	if (!created) {
		init();
		created = true;
	}
	uint8_t **s0 = S0.text, **s1 = S1.text;
	uint32_t **r0 = S0.rend, **r1 = S1.rend;
	VTScreen * scr = S.current;
	int16_t * y = &scr->cursor.y;
	if (likely(scr == &S.s[0] && S0.text) && *y >= c.h) {
		scroll1(*y - c.h + 1);
		*y = c.h - 1;
	}
	init_screen_elements(&S.s[0], s0, r0);
	init_screen_elements(&S.s[1], s1, r1);
	scr_start_selection((xcb_point_t){}, SEL_CHAR);
	// Constrain dimensions:
	c.w = MIN(c.w, JBXVT_MAX_COLS);
	c.h = MIN(c.h, JBXVT_MAX_ROWS);
	tty_set_size(c.w, c.h);
	S.chars = c;
	reset_row_col();
	--c.h; --c.w;
	sbar_show(c.h + S.sline.top, S.offset,
		S.offset + c.h);
	clear();
	decscnm();
	xcb_flush(X.xcb);
	repaint();
	draw_cursor();
}

