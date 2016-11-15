// Copyright 2016, Jeffrey E. Bedard
#ifndef LIBJB_JBDIM_H
#define LIBJB_JBDIM_H
#include <stdint.h>
struct JBDim {
	union {
		uint16_t width, w, columns, cols, col, c;
		int16_t x, top, t, max;
	};
	union {
		uint16_t height, h, row, rows, r;
		int16_t y, bottom, bot, b, min, index;
	};
};
#endif//!LIBJB_JBDIM_H