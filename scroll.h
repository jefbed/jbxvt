/*  Copyright 2016, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey, University of Kent at Canterbury.*/
#ifndef JBXVT_SCROLL_H
#define JBXVT_SCROLL_H
#include <stdint.h>
#include <xcb/xcb.h>
void scroll(xcb_connection_t * xc, const uint8_t row1,
	const uint8_t row2, const int16_t count);
void jbxvt_scroll_primary_screen(const int16_t count);
#endif//!JBXVT_SCROLL_H
