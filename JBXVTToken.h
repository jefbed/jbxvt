/*  Copyright 2017-2020, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey, University of Kent at Canterbury.*/
#ifndef JBXVT_JBXVTTOKEN_H
#define JBXVT_JBXVTTOKEN_H
#include <stdint.h>
#include "JBXVTTokenIndex.h"
enum { JBXVT_TOKEN_MAX_LENGTH = 63, JBXVT_TOKEN_MAX_ARGS = 8 };
/*  Structure used to represent a piece of input from the program
 *  or an interesting X event.  */
struct JBXVTToken{
    // numeric arguments:
    int16_t arg[JBXVT_TOKEN_MAX_ARGS];
    // text for string tokens:
    uint8_t string[JBXVT_TOKEN_MAX_LENGTH + 1];
    enum JBXVTTokenIndex type;
    union {
        // non-zero for private control sequences:
        uint8_t private;
        // number of new lines in a string token:
        uint8_t nlcount;
    };
    // length of a string in a string token:
    uint8_t length;
    // number of numeric arguments:
    uint8_t nargs;
    // single unprintable character:
    uint8_t tk_char;
};
#endif//!JBXVT_JBXVTTOKEN_H
