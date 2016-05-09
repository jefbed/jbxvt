#include "selection.h"

#include "global.h"
#include "jbxvt.h"
#include "save_selection.h"
#include "selcmp.h"
#include "screen.h"
#include "slinest.h"
#include "xsetup.h"
#include "xvt.h"

#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>


// Globals:
unsigned char * selection_text;
int selection_length;		/* length of selection text */

// Static globals:
static enum selunit selection_unit;	/* current unit of selection */

/*  Make the selection currently delimited by the selection end markers.
 */
void scr_make_selection(const Time time)
{
	if (save_selection() < 0)
		return;

	XSetSelectionOwner(jbxvt.X.dpy,XA_PRIMARY,jbxvt.X.win.vt,time);
	if (XGetSelectionOwner(jbxvt.X.dpy,XA_PRIMARY) != jbxvt.X.win.vt)
		error("Could not get primary selection");

	/*  Place in CUT_BUFFER0 for backup.
	 */
	XChangeProperty(jbxvt.X.dpy,DefaultRootWindow(jbxvt.X.dpy),XA_CUT_BUFFER0,
		XA_STRING,8,PropModeReplace,selection_text,selection_length);
}

/*  respond to a request for our current selection.
 */
void scr_send_selection(const int time __attribute__((unused)),
	const int requestor, const int target, const int property)
{
	XEvent event = { .xselection.type = SelectionNotify,
		.xselection.selection = XA_PRIMARY, .xselection.target = XA_STRING,
		.xselection.requestor = requestor, .xselection.time = time };
	if (target == XA_STRING) {
		XChangeProperty(jbxvt.X.dpy,requestor,property,XA_STRING,8,PropModeReplace,
				selection_text,selection_length);
		event.xselection.property = property;
	} else
		event.xselection.property = None;
	XSendEvent(jbxvt.X.dpy,requestor,False,0,&event);
}

//  Clear the current selection.
void scr_clear_selection(void)
{
	if (selection_text != NULL) {
		free(selection_text);
		selection_text = NULL;
		selection_length = 0;
	}
	show_selection(0,cheight - 1,0,cwidth - 1);
	selend1.se_type = selend2.se_type = NOSEL;
}

//  start a selection using the specified unit.
void scr_start_selection(int x, int y, enum selunit unit)
{
	show_selection(0,cheight - 1,0,cwidth - 1);
	int16_t col = (x - MARGIN) / fwidth;
	int16_t row = (y - MARGIN) / fheight;
	selection_unit = unit;
	fix_rc(&row,&col);
	rc_to_selend(row,col,&selanchor);
	selend2 = selend1 = selanchor;
	adjust_selection(&selend2);
	show_selection(0,cheight - 1,0,cwidth - 1);
}


/*  Convert a row and column coordinates into a selection endpoint.
 */
void rc_to_selend(const int16_t row, const int16_t col, struct selst * se)
{
	int i = (row - offset);
	if (i >= 0)
		se->se_type = SCREENSEL;
	else {
		se->se_type = SAVEDSEL;
		i = -1 - i;
	}
	se->se_index = i;
	se->se_col = col;
}

/*  Fix the coordinates so that they are within the screen and do not lie within
 *  empty space.
 */
void fix_rc(int16_t * restrict rowp, int16_t * restrict colp)
{
	int i, len, row, col;
	unsigned char *s;

	col = *colp;
	if (col < 0)
		col = 0;
	if (col > cwidth)
		col = cwidth;
	row = *rowp;
	if (row < 0)
		row = 0;
	if (row >= cheight)
		row = cheight - 1;

	if (selection_unit == CHAR) {
		i = (row - offset);
		if (i >= 0) {
			s = screen->text[i];
			if (col > 0 && s[col - 1] < ' ')
				while (col < cwidth && s[col] < ' ')
					col++;
		} else {
			i = -1 - i;
			len = sline[i]->sl_length;
			s = sline[i]->sl_text;
			if (col > 0 && s[col - 1] < ' ')
				while (col <= len && s[col] < ' ')
					col++;
			if (col > len)
				col = cwidth;
		}
	}
	*colp = col;
	*rowp = row;
}

/*  Convert the selection into a row and column.
 */
void selend_to_rc(int16_t * restrict rowp, int16_t * restrict colp,
	struct selst * restrict se)
{
	if (se->se_type == NOSEL)
		return;

	*colp = se->se_col;
	if (se->se_type == SCREENSEL)
		*rowp = se->se_index + offset;
	else
		*rowp = offset - se->se_index - 1;
}

/*  Convert a section of displayed text line into a text string suitable for pasting.
 *  *lenp is the length of the input string, i1 is index of the first character to
 *  convert and i2 is the last.  The length of the returned string is returned
 *  in *lenp;
 */
unsigned char * convert_line(unsigned char * restrict str,
	int * restrict lenp, int i1, int i2)
{
	static unsigned char buf[MAX_WIDTH + 3];
	unsigned char *s;
	int i;
	int newline;

	newline = (i2 + 1 == cwidth) && (str[*lenp] == 0);
	if (i2 >= *lenp)
		i2 = *lenp - 1;
	if (i2 - i1 >= MAX_WIDTH)
		i2 = i1 + MAX_WIDTH;
	while (i2 >= i1 && str[i2] == 0)
		i2--;
	s = buf;
	for (i = i1; i <= i2; i++) {
		if (str[i] >= ' ')
			*s++ = str[i];
		else if (str[i] == '\t') {
			*s++ = '\t';
			while (i < i2 && str[i + 1] == 0)
				i++;
		} else
			*s++ = ' ';
	}
	if (newline)
		*s++ = '\n';
	*s = 0;
	*lenp = s - buf;
	return (buf);
}

/*  Adjust the selection to a word or line boundary. If the include endpoint is
 *  non NULL then the selection is forced to be large enough to include it.
 */
void adjust_selection(struct selst * restrict include)
{
	struct selst *se1, *se2;

	if (selection_unit == CHAR)
		return;

	if (selcmp(&selend1,&selend2) <= 0) {
		se1 = &selend1;
		se2 = &selend2;
	} else {
		se2 = &selend1;
		se1 = &selend2;
	}
	if (selection_unit == WORD) {
		int16_t i = se1->se_col;
		unsigned char * s = se1->se_type == SCREENSEL
			? screen->text[se1->se_index]
			: sline[se1->se_index]->sl_text;
		static int char_class[256] = {
			32,   1,   1,   1,   1,   1,   1,   1,
			1,  32,   1,   1,   1,   1,   1,   1,
			1,   1,   1,   1,   1,   1,   1,   1,
			1,   1,   1,   1,   1,   1,   1,   1,
			32,  33,  34,  35,  36,  37,  38,  39,
			40,  41,  42,  43,  44,  45,  46,  47,
			48,  48,  48,  48,  48,  48,  48,  48,
			48,  48,  58,  59,  60,  61,  62,  63,
			64,  48,  48,  48,  48,  48,  48,  48,
			48,  48,  48,  48,  48,  48,  48,  48,
			48,  48,  48,  48,  48,  48,  48,  48,
			48,  48,  48,  91,  92,  93,  94,  48,
			96,  48,  48,  48,  48,  48,  48,  48,
			48,  48,  48,  48,  48,  48,  48,  48,
			48,  48,  48,  48,  48,  48,  48,  48,
			48,  48,  48, 123, 124, 125, 126,   1,
			1,   1,   1,   1,   1,   1,   1,   1,
			1,   1,   1,   1,   1,   1,   1,   1,
			1,   1,   1,   1,   1,   1,   1,   1,
			1,   1,   1,   1,   1,   1,   1,   1,
			160, 161, 162, 163, 164, 165, 166, 167,
			168, 169, 170, 171, 172, 173, 174, 175,
			176, 177, 178, 179, 180, 181, 182, 183,
			184, 185, 186, 187, 188, 189, 190, 191,
			192, 193, 194, 195, 196, 197, 198, 199,
			200, 201, 202, 203, 204, 205, 206, 207,
			208, 209, 210, 211, 212, 213, 214, 215,
			216, 217, 218, 219, 220, 221, 222, 223,
			224, 225, 226, 227, 228, 229, 230, 231,
			232, 233, 234, 235, 236, 237, 238, 239,
			240, 241, 242, 243, 244, 245, 246, 247,
			248, 249, 250, 251, 252, 253, 254, 255
		};

		while (i > 0 && char_class[s[i]] == char_class[s[i-1]])
			  i--;
		se1->se_col = i;
		i = se2->se_col;
		if (se2 == include || selcmp(se2,&selanchor) == 0)
			  i++;
		int16_t len;
		if (se2->se_type == SCREENSEL) {
			s = screen->text[se2->se_index];
			len = cwidth;
		} else {
			s = sline[se2->se_index]->sl_text;
			len = sline[se2->se_index]->sl_length;
		}
		while (i < len && char_class[s[i]] == char_class[s[i-1]])
			  i++;
		se2->se_col = (i > len) ? cwidth : i;
	} else if (selection_unit == LINE) {
		se1->se_col = 0;
		se2->se_col = cwidth;
	}
}

/*  Determine if the current selection overlaps row1-row2 and if it does then
 *  remove it from the screen.
 */
void check_selection(int row1, int row2)
{
	int r1, r2, x;

	if (selend1.se_type == NOSEL || selend2.se_type == NOSEL)
		return;

	r1 = selend1.se_type == SCREENSEL ? selend1.se_index : -1;
	r2 = selend2.se_type == SCREENSEL ? selend2.se_index : -1;
	if (r1 > r2) {
		x = r1;
		r1 = r2;
		r2 = x;
	}
	if (row2 < r1 || row1 > r2)
		return;
	show_selection(0,cheight - 1,0,cwidth - 1);
	selend2.se_type = NOSEL;
}

/*  Paint any part of the selection that is between rows row1 and row2 inclusive
 *  and between cols col1 and col2 inclusive.
 */
void show_selection(int row1, int row2, int col1, int col2)
{
	int sr, sc, er, ec;
	int x1, x2, y, row;

	if (selend1.se_type == NOSEL || selend2.se_type == NOSEL)
		return;
	if (selcmp(&selend1,&selend2) == 0)
		return;
	int16_t r1, c1, r2, c2;
	selend_to_rc(&r1,&c1,&selend1);
	selend_to_rc(&r2,&c2,&selend2);
	col2++;

	/*  Obtain initial and final endpoints for the selection.
	 */
	if (r1 < r2 || (r1 == r2 && c1 <= c2)) {
		sr = r1;
		sc = c1;
		er = r2;
		ec = c2;
	} else {
		sr = r2;
		sc = c2;
		er = r1;
		ec = c1;
	}
	if (sr < row1) {
		sr = row1;
		sc = col1;
	}
	if (sc < col1)
		sc = col1;
	if (er > row2) {
		er = row2;
		ec = col2;
	}
	if (ec > col2)
		ec = col2;

	if (sr > er)
		return;

	//  Paint in the reverse video:
	for (row = sr; row <= er; row++) {
		y = MARGIN + row * fheight;
		x1 = MARGIN + (row == sr ? sc : col1) * fwidth;
		x2 = MARGIN + ((row == er) ? ec : col2) * fwidth;
		if (x2 > x1)
			XFillRectangle(jbxvt.X.dpy,jbxvt.X.win.vt,jbxvt.X.gc.hl,
				x1,y,x2 - x1,fheight);
	}
}

