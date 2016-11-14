// Copyright 2016, Jeffrey E. Bedard
#ifndef JBXVT_MOUSE_H
#define JBXVT_MOUSE_H
#include "libjb/JBDim.h"
#include <stdbool.h>
enum { JBXVT_RELEASE = 1, JBXVT_MOTION = 2};
bool jbxvt_get_mouse_tracked(void);
bool jbxvt_get_mouse_motion_tracked(void);
void jbxvt_track_mouse(uint8_t b, uint32_t state, struct JBDim p,
	const uint8_t flags);
#endif//!JBXVT_MOUSE_H
