// Copyright 2017-2020, Jeffrey E. Bedard
#ifndef JBXVT_SGR_H
#define JBXVT_SGR_H
#include <xcb/xcb.h>
struct JBXVTToken;
void jbxvt_handle_sgr(xcb_connection_t * xc,
    struct JBXVTToken * token)
    __attribute__((hot));
#endif//!JBXVT_SGR_H
