#include "save_selection.h"

#include "global.h"
#include "screen.h"
#include "selcmp.h"
#include "selection.h"
#include "slinest.h"

#include <stdlib.h>
#include <string.h>

/*  Convert the currently marked screen selection as a text string and save it
 *  as the current saved selection.  0 is returned for a success, -1 for a failure.
 */
int8_t save_selection(void)
{
	unsigned char *str, *s;
	int i, len, total, col1, col2;
	struct selst *se1, *se2;
	struct slinest *sl;

	if (selend1.se_type == NOSEL || selend2.se_type == NOSEL)
		return(-1);
	if (selend1.se_type == selend2.se_type
				&& selend1.se_index == selend2.se_index
				&& selend1.se_col == selend2.se_col)
		return(-1);

	if (selection_text != NULL)
		free(selection_text);

	/*  Set se1 and se2 to point to the first and second selection endpoints.
	 */
	if (selcmp(&selend1,&selend2) <= 0) {
		se1 = &selend1;
		se2 = &selend2;
	} else {
		se2 = &selend1;
		se1 = &selend2;
	}
	str = (unsigned char *)malloc(total = 1);
	if (se1->se_type == SAVEDSEL) {
		col1 = se1->se_col;
		for (i = se1->se_index; i >= 0; i--) {
			sl = sline[i];
			if (se2->se_type == SAVEDSEL && se2->se_index == i) {
				col2 = se2->se_col - 1;
				i = 0;			/* force loop exit */
			} else
				col2 = cwidth - 1;
			len = sl->sl_length;
			s = convert_line(sl->sl_text,&len,col1,col2);
			str = (unsigned char *)realloc(str,total + len);
			if (str == NULL)
				abort();
			strncpy((char *)str + total - 1,(char *)s,len);
			total += len;
			col1 = 0;
		}
	}
	if (se2->se_type == SCREENSEL) {
		if (se1->se_type == SCREENSEL) {
			i = se1->se_index;
			col1 = se1->se_col;
		} else {
			i = 0;
			col1 = 0;
		}
		for (; i <= se2->se_index; i++) {
			col2 = i == se2->se_index ? se2->se_col : cwidth;
			if (--col2 < 0)
				break;
			len = cwidth;
			s = convert_line(screen->text[i],&len,col1,col2);
			str = (unsigned char *)realloc(str,total + len);
			if (str == NULL)
				abort();
			strncpy((char *)str + total - 1,(char *)s,len);
			total += len;
			col1 = 0;
		}
	}
	str[total - 1] = 0;
	selection_text = str;
	selection_length = total - 1;
	return(0);
}


