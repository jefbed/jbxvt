#include "scr_move.h"

#include "cursor.h"
#include "jbxvt.h"
#include "log.h"
#include "screen.h"
#include "scroll.h"
#include "scr_reset.h"
#include "selection.h"

#include <stdlib.h>

/*  Move the cursor to a new position.  The relative argument is a pair of
 *  flags that specify relative rather than absolute motion.
 */
void scr_move(const int16_t x, const int16_t y, const uint8_t relative)
{
	LOG("scr_move(x:%d, y:%d, relative:%d)", x, y, relative);
	home_screen();
	cursor(CURSOR_DRAW); // clear
	xcb_point_t * restrict c = &jbxvt.scr.current->cursor;
	/* Sanitize non-relative arguments--must be positive.  */
	c->x = relative & COL_RELATIVE ? c->x + x : ((x>=0)?x:0);
	c->y = relative & ROW_RELATIVE ? c->y + y : ((y>=0)?y:0);
	reset_row_col();
	jbxvt.scr.current->wrap_next = 0;
	check_selection(c->y, c->y);
	cursor(CURSOR_DRAW); // draw
#if 0
	Size m = jbxvt.scr.current->margin;
	LOG("c->y: %d, m.bottom: %d, m.top: %d", c->y, m.b, m.t);
	if(c->y == m.bottom - 1) {
		int16_t d = relative & ROW_RELATIVE ? y : abs(c->y - y);
		d = d ? d : 1;
		scroll(m.top, m.bot, d);
	}
#endif
}

