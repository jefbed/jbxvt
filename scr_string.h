/*  Copyright 2016, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey,
    University of Kent at Canterbury.*/

#ifndef SCR_STRING_H
#define SCR_STRING_H

#include <stdbool.h>
#include <stdint.h>

/*  Display the string at the current position.
    nlcount is the number of new lines in the string.  */
void scr_string(uint8_t * restrict str, uint8_t len, int8_t nlcount);

//  Tab to the next tab stop.
void scr_tab(void);

// Set tab stops:
// -1 clears all, -2 sets default
void scr_set_tab(int i, const bool value);

#endif//!SCR_STRING_H
