#include "change_selection.h"


#include "jbxvt.h"
#include "screen.h"
#include "selcmp.h"
#include "selection.h"
#include "xvt.h"

/*  Repaint the displayed selection to reflect the new value.  ose1 and ose2
 *  are assumed to represent the currently displayed selection endpoints.
 */
void change_selection(struct selst * restrict ose1,
	struct selst * restrict ose2)
{
	int16_t rs, cs, re, ce, n;
	int row;
	int row1, row2;
	int x1, x2, y;
	struct selst *se, *se1, *se2;

	if (selcmp(ose1,ose2) > 0) {
		se = ose1;
		ose1 = ose2;
		ose2 = se;
	}
	if (selcmp(&jbxvt.sel.end1,&jbxvt.sel.end2) <= 0) {
		se1 = &jbxvt.sel.end1;
		se2 = &jbxvt.sel.end2;
	} else {
		se1 = &jbxvt.sel.end2;
		se2 = &jbxvt.sel.end1;
	}

	if ((n = selcmp(se1,ose1)) != 0) {

		/* repaint the start.
		 */
		if (n < 0) {
			selend_to_rc(&rs,&cs,se1);
			selend_to_rc(&re,&ce,ose1);
		} else {
			selend_to_rc(&rs,&cs,ose1);
			selend_to_rc(&re,&ce,se1);
		}
		row1 = rs < 0 ? 0 : rs;
		row2 = re >= cheight ? cheight - 1 : re;

		/*  Invert the changed area
		 */
		for (row = row1; row <= row2; row++) {
			y = MARGIN + row * jbxvt.X.font_height;
			x1 = MARGIN + (row == rs ? cs * jbxvt.X.font_width : 0);
			x2 = MARGIN + ((row == re) ? ce : cwidth) * jbxvt.X.font_width;
			XFillRectangle(jbxvt.X.dpy,jbxvt.X.win.vt,jbxvt.X.gc.hl,x1,y,x2 - x1,jbxvt.X.font_height);
		}
	}
	if ((n = selcmp(se2,ose2)) != 0) {

		/* repaint the end.
		 */
		if (n < 0) {
			selend_to_rc(&rs,&cs,se2);
			selend_to_rc(&re,&ce,ose2);
		} else {
			selend_to_rc(&rs,&cs,ose2);
			selend_to_rc(&re,&ce,se2);
		}
		row1 = rs < 0 ? 0 : rs;
		row2 = re >= cheight ? cheight - 1 : re;

		/*  Invert the changed area
		 */
		for (row = row1; row <= row2; row++) {
			y = MARGIN + row * jbxvt.X.font_height;
			x1 = MARGIN + (row == rs ? cs * jbxvt.X.font_width : 0);
			x2 = MARGIN + ((row == re) ? ce : cwidth) * jbxvt.X.font_width;
			XFillRectangle(jbxvt.X.dpy,jbxvt.X.win.vt,jbxvt.X.gc.hl,x1,y,x2 - x1,jbxvt.X.font_height);
		}
	}
}


