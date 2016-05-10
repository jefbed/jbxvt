#include "scr_erase.h"

#include "cursor.h"
#include "jbxvt.h"
#include "sbar.h"
#include "screen.h"
#include "scroll.h"
#include "selection.h"
#include "xvt.h"

#include <string.h>

//  erase part or the whole of a line
void scr_erase_line(int mode)
{
	int i, x, y, width, height;
	unsigned char *r, *s;

	home_screen();
	y = MARGIN + screen->row * jbxvt.X.font_height;
	height = jbxvt.X.font_height;
	s = screen->text[screen->row];
	r = screen->rend[screen->row];
	switch (mode) {
	    case START :
		x = MARGIN;
		width = (screen->col + 1) * jbxvt.X.font_width;
		memset(s,0,screen->col + 1);
		memset(r,0,screen->col + 1);
		break;
	    case END :
		x = MARGIN + screen->col * jbxvt.X.font_width;
		width = (jbxvt.scr.chars.width - screen->col)
			* jbxvt.X.font_width;
		memset(s + screen->col,0,
			jbxvt.scr.chars.width - screen->col + 1);
		memset(r + screen->col,0,
			jbxvt.scr.chars.width - screen->col);
		break;
	    case ENTIRE :
		x = MARGIN;
		width = jbxvt.scr.chars.width * jbxvt.X.font_width;
		memset(s,0,jbxvt.scr.chars.width + 1);
		memset(r,0,jbxvt.scr.chars.width);
		break;
	    default :
		return;
	}
	/*  patch in the final rendition flag if there is any non-zero
	 *  rendition.
	 */
	r[jbxvt.scr.chars.width] = 0;
	for (i = 0; i < jbxvt.scr.chars.width; i++)
		if (r[i] != 0) {
			r[jbxvt.scr.chars.width] = 1;
			break;
		}
	cursor();
	check_selection(screen->row,screen->row);
	XClearArea(jbxvt.X.dpy,jbxvt.X.win.vt,x,y,width,height,False);
	screen->wrap_next = 0;
	cursor();
}

//  erase part or the whole of the screen
void scr_erase_screen(int mode)
{
	int x, y, width, height;
	int i;

	home_screen();
	screen->wrap_next = 0;
	x = MARGIN;
	width = jbxvt.X.font_width * jbxvt.scr.chars.width;
	switch (mode) {
	    case START :
		y = MARGIN;
		height = screen->row * jbxvt.X.font_height;
		for (i = 0; i < screen->row; i++) {
			memset(screen->text[i],0,jbxvt.scr.chars.width + 1);
			memset(screen->rend[i],0,jbxvt.scr.chars.width + 1);
		}
		check_selection(0,screen->row - 1);
		if (height > 0)
			XClearArea(jbxvt.X.dpy,jbxvt.X.win.vt,
				x,y,width,height,False);
		scr_erase_line(mode);
		break;
	    case END :
		if (screen->row != 0 || screen->col != 0) {
			y = MARGIN + (screen->row + 1) * jbxvt.X.font_height;
			height = (jbxvt.scr.chars.height - screen->row - 1)
				* jbxvt.X.font_height;
			for (i = screen->row + 1; i < jbxvt.scr.chars.height;
				i++) {
				memset(screen->text[i],0,
					jbxvt.scr.chars.width + 1);
				memset(screen->rend[i],0,
					jbxvt.scr.chars.width + 1);
			}
			check_selection(screen->row + 1,
				jbxvt.scr.chars.height - 1);
			if (height > 0)
				XClearArea(jbxvt.X.dpy,jbxvt.X.win.vt,
					x,y,width,height,False);
			scr_erase_line(mode);
			break;
		}
		/*  If we are positioned at the top left hand corner then
		 *  it is effectively a whole screen clear.
		 *  Drop through so that we do not need to duplicate
		 *  the scroll-up code.
		 */
	    case ENTIRE :
		y = MARGIN;
		height = jbxvt.scr.chars.height * jbxvt.X.font_height;
		if (screen == &jbxvt.scr.s1)
			scroll1(jbxvt.scr.chars.height);
		else
			for (i = 0; i < jbxvt.scr.chars.height; i++) {
				memset(screen->text[i],0,
					jbxvt.scr.chars.width + 1);
				memset(screen->rend[i],0,
					jbxvt.scr.chars.width + 1);
			}
		cursor();
		check_selection(0,jbxvt.scr.chars.height - 1);
		XClearArea(jbxvt.X.dpy,jbxvt.X.win.vt,x,y,width,height,False);
		cursor();
		sbar_show(jbxvt.scr.chars.height + jbxvt.scr.sline.top - 1,
			0, jbxvt.scr.chars.height - 1);
		break;
	    default :
		return;
	}
}

