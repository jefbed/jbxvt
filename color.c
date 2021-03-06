// Copyright 2017, Jeffrey E. Bedard
#include "color.h"
#include "JBXVTOptions.h"
#include "gc.h"
#include "libjb/macros.h"
#include "window.h"
#include "xcb_screen.h"
static struct {
    pixel_t bg, fg, current_fg, current_bg;
} color_data;
void jbxvt_init_colors(xcb_connection_t * xc,
    struct JBXVTOptions * opt)
{
    color_data.fg = jbxvt_set_fg(xc, opt->foreground_color);
    color_data.bg = jbxvt_set_fg(xc, opt->background_color);
}
void jbxvt_set_reverse_video(xcb_connection_t * xc)
{
    jb_set_fg(xc, jbxvt_get_text_gc(xc), color_data.current_bg);
    jb_set_bg(xc, jbxvt_get_text_gc(xc), color_data.current_fg);
}
void jbxvt_reverse_screen_colors(xcb_connection_t * xc)
{
    JB_SWAP(pixel_t, color_data.fg, color_data.bg);
    JB_SWAP(pixel_t, color_data.current_fg, color_data.current_bg);
    xcb_change_gc(xc, jbxvt_get_text_gc(xc), XCB_GC_FOREGROUND
        | XCB_GC_BACKGROUND, (uint32_t[]){
        color_data.fg, color_data.bg});
    xcb_change_window_attributes(xc, jbxvt_get_vt_window(xc),
        XCB_CW_BACK_PIXEL, &color_data.bg);
}
#define GET_PIXEL(fgbg) return color_data.current_##fgbg = jb_set_##fgbg(\
    xc, jbxvt_get_text_gc(xc), p);
// not pure, has side-effects
pixel_t jbxvt_set_fg_pixel(xcb_connection_t * xc, const pixel_t p)
{
    GET_PIXEL(fg);
}
// not pure, has side-effects
pixel_t jbxvt_set_bg_pixel(xcb_connection_t * xc, const pixel_t p)
{
    GET_PIXEL(bg);
}
static pixel_t set_x(xcb_connection_t * xc, const char * color,
    const pixel_t backup, pixel_t (*func)(xcb_connection_t *,
    const pixel_t))
{
    return func(xc, color ? jb_get_pixel(xc,
        jbxvt_get_colormap(xc), color) : backup);
}
pixel_t jbxvt_get_fg(void)
{
    return color_data.fg;
}
pixel_t jbxvt_get_bg(void)
{
    return color_data.bg;
}
pixel_t jbxvt_set_fg(xcb_connection_t * xc, const char * color)
{
    return set_x(xc, color, color_data.fg, &jbxvt_set_fg_pixel);
}
pixel_t jbxvt_set_bg(xcb_connection_t * xc, const char * color)
{
    return set_x(xc, color, color_data.bg, &jbxvt_set_bg_pixel);
}
