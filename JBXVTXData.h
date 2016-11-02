// Copyright 2016, Jeffrey E. Bedard
#ifndef JBXVT_JBXVTXDATA_H
#define JBXVT_JBXVTXDATA_H
#include "libjb/xcb.h"
struct JBXVTXPixels {
	pixel_t bg, fg, current_fg, current_bg;
};
struct JBXVTXData {
	struct JBXVTXPixels color;
};
#endif//!JBXVT_JBXVTXDATA_H
