/*  Copyright 2017, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey, University of Kent at Canterbury.*/
#ifndef JBXVT_ESC_H
#define JBXVT_ESC_H
#include <stdint.h>
#include <xcb/xcb.h>
struct JBXVTToken;
void jbxvt_csi(xcb_connection_t * xc,
    int16_t c, struct JBXVTToken * tk);
void jbxvt_end_cs(xcb_connection_t * xc,
    int16_t c, struct JBXVTToken * tk);
void jbxvt_esc(xcb_connection_t * xc,
    int16_t c, struct JBXVTToken * tk);
#endif//!JBXVT_ESC_H
