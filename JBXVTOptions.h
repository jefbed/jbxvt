// Copyright 2016, Jeffrey E. Bedard
#ifndef JBXVT_OPTIONS_H
#define JBXVT_OPTIONS_H
#include "color.h"
#include "font.h"
#include "libjb/size.h"
struct JBXVTOptions {
	struct JBXVTColorOptions color;
	struct JBXVTFontOptions font;
	struct JBDim size;
	int screen;
	bool show_scrollbar;
};
#endif//!JBXVT_OPTIONS_H