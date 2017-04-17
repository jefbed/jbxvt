// Copyright 2017, Jeffrey E. Bedard
//  Tab to the next tab stop.
#ifndef JBXVT_TAB_H
#define JBXVT_TAB_H
#include <stdbool.h>
#include <stdint.h>
#include <xcb/xcb.h>
struct JBXVTToken;
void jbxvt_tab(xcb_connection_t * xc);
// Do v tabs
void jbxvt_cht(xcb_connection_t * xc, int16_t v);
// Handle DECTBC:
void jbxvt_handle_JBXVT_TOKEN_TBC(void * xc, struct JBXVTToken * token);
// Set tab stops:
// -1 clears all, -2 sets default
void jbxvt_set_tab(int16_t i, const bool value);
#endif//!JBXVT_TAB_H
