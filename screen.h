/*  Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 *  Redistribution and use in source code and/or executable forms, with
 *  or without modification, are permitted provided that the following
 *  condition is met:
 *
 *  Any redistribution must retain the above copyright notice, this
 *  condition and the following disclaimer, either as part of the
 *  program source code included in the redistribution or in human-
 *  readable materials provided with the redistribution.
 *
 *  THIS SOFTWARE IS PROVIDED "AS IS".  Any express or implied
 *  warranties concerning this software are disclaimed by the copyright
 *  holder to the fullest extent permitted by applicable law.  In no
 *  event shall the copyright-holder be liable for any damages of any
 *  kind, however caused and on any theory of liability, arising in any
 *  way out of the use of, or inability to use, this software.
 *
 *  -------------------------------------------------------------------
 *
 *  In other words, do not misrepresent my work as your own work, and
 *  do not sue me if it causes problems.  Feel free to do anything else
 *  you wish with it.
 */

/* @(#)screen.h	1.2 16/11/93 (UKC) */

#ifndef SCREEN_H
#define SCREEN_H

#include "screenst.h"

#include <stdint.h>
#include <X11/Xlib.h>

//  flags for scr_move()
#define COL_RELATIVE	1	/* column movement is relative */
#define ROW_RELATIVE	2	/* row movement is relative */

#define MAX_SCROLL	50	/* max number of lines that can scroll at once */

//  arguments to the screen delete functions
enum { END, START, ENTIRE };

//  rendition style flags:
#define RS_NONE	0		/* Normal */
#define RS_BOLD		1/* Bold face */
#define RS_ULINE	(1<<1)	/* underline */
#define RS_BLINK	(1<<2)	/* blinking */
#define RS_RVID		(1<<3) /* reverse video */
#define RS_ITALIC	(1<<4)
#define RS_LOW		(1<<5) // low intensity
// For colors:
// foreground:
#define RS_F0		(1<<6)
#define RS_F1		(1<<7)
#define RS_F2		(1<<8)
#define RS_F3		(1<<9)
#define RS_F4		(1<<10)
#define RS_F5		(1<<11)
#define RS_F6		(1<<12)
#define RS_F7		(1<<13)
#define RS_FR		(1<<14) // reset
// background:
#define RS_B0		(1<<15)
#define RS_B1		(1<<16)
#define RS_B2		(1<<17)
#define RS_B3		(1<<18)
#define RS_B4		(1<<19)
#define RS_B5		(1<<20)
#define RS_B6		(1<<21)
#define RS_B7		(1<<22)
#define RS_BR		(1<<23) // reset

#define RS_BF		(1<<24) // bright foreground
#define RS_BB		(1<<25) // bright background

// not enough bits for bright backgrounds

// Normal colors:
#define COLOR_0 "black"
#define COLOR_1 "red3"
#define COLOR_2 "green3"
#define COLOR_3 "yellow3"
#define COLOR_4 "blue3"
#define COLOR_5 "magenta3"
#define COLOR_6 "cyan3"
#define COLOR_7 "grey90"

// Bright colors:
#define BCOLOR_0 "black"
#define BCOLOR_1 "red3"
#define BCOLOR_2 "green3"
#define BCOLOR_3 "yellow3"
#define BCOLOR_4 "blue3"
#define BCOLOR_5 "magenta3"
#define BCOLOR_6 "cyan3"
#define BCOLOR_7 "grey90"

void home_screen(void);
int16_t is_string_char(int16_t c);
void scr_backspace(void);
void scr_bell(void);
void scr_change_rendition(const uint32_t style);
void scr_change_screen(const uint8_t direction);
void scr_delete_lines(int);

//  Return the width and height of the screen.
void scr_get_size(uint16_t * restrict width_p,
	uint16_t * restrict height_p);

void scr_index(void);
void scr_init(const unsigned int saved_lines);
void scr_insert_lines(int);
void scr_move_by(const int16_t y);
void scr_move_to(int16_t y);
void scr_paste_primary(int,Window,Atom);
void scr_report_display(void);
void scr_report_position(void);
void scr_restore_cursor(void);
void scr_rindex(void);
void scr_save_cursor(void);
void scr_set_margins(int16_t top, int16_t bottom);

#endif//!SCREEN_H
