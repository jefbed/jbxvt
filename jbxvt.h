#ifndef JBXVT_H
#define JBXVT_H

#include "command.h"
#include "selst.h"
#include "slinest.h"

#include <stdbool.h>
#include <stdint.h>
#include <X11/Xlib.h>

struct JBXVT {
	struct {
		uint8_t font_height, font_width;
		Display * dpy;
		XFontStruct *font;
		struct {
			Window vt, sb, main;
		} win;
		struct {
			GC tx, ne, hl, cu, sb;
		} gc;
		struct {
			Colormap map;
			unsigned long bg, fg, cursor;
		} color;
	} X;
	struct {
		int32_t offset; // current vert saved line
		uint32_t rstyle; // render style
		struct {
			struct slinest **data; // saved lines
			uint16_t max; // max # of saved lines
			uint16_t top; /* high water mark
					       of saved scroll lines */
		} sline;
	} scr;
	struct {
		struct selst end1, end2, // selection endpoints
			     anchor; //selection anchor
	} sel;
	struct {
		bool save_rstyle:1;
	} opt;
	struct {
		unsigned char * send_nxt; // next char to be sent
		int send_count; // # chars waiting to be sent
		int fd; // file descriptor connected to the command
		int8_t width; // # file descriptors being used
		struct {
			unsigned char data[COM_BUF_SIZE];
			unsigned char *next, *top;
		} buf;
		struct {
			unsigned char data[COM_PUSH_MAX];
			unsigned char *top;
		} stack;
	} com;
};

extern struct JBXVT jbxvt; // in xvt.c

#endif//!JBXVT_H
