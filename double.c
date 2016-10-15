// Copyright 2016, Jeffrey E. Bedard
#include "double.h"

#include "cursor.h"
#include "jbxvt.h"
#include "repaint.h"

#include <stdlib.h>

void jbxvt_set_double_width_line(const bool is_dwl)
{
	jbxvt.scr.current->dwl[jbxvt.scr.current->cursor.y] = is_dwl;
	jbxvt_repaint(); // in case set mid-line
	jbxvt_draw_cursor(); // clear stale cursor block
}

// Generate a double-width string.  Free the result!
uint8_t * jbxvt_get_double_width_string(uint8_t * in_str, uint16_t * len)
{
	const uint16_t l = *len;
	uint8_t * o = malloc(*len <<= 1);
	uint8_t * j = o;
	for (uint_fast16_t i = 0; i < l; ++i, j += 2) {
		j[0] = in_str[i];
		j[1] = ' ';
	}
	return o;
}
