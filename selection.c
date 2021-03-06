/*  Copyright 2017, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey, University of Kent at Canterbury.*/
#include "selection.h"
#include "JBXVTSelectionData.h"
#include "libjb/log.h"
#include "libjb/macros.h"
#include "libjb/xcb.h"
#include "save_selection.h"
#include "selend.h"
#include "show_selection.h"
#include "size.h"
#include "window.h"
static struct JBXVTSelectionData selection_data;
#define SE selection_data.end
// Return selection end points: first, second, and anchor
struct JBDim * jbxvt_get_selection_end_points(void) {
    return selection_data.end;
}
enum JBXVTSelectionUnit jbxvt_get_selection_unit(void) {
    return selection_data.unit;
}
bool jbxvt_is_selected(void) {
    return selection_data.on_screen;
}
//  Return the atom corresponding to "CLIPBOARD"
xcb_atom_t jbxvt_get_clipboard(xcb_connection_t * xc) {
    static xcb_atom_t a;
    return a ? a : (a = jb_get_atom(xc, "CLIPBOARD"));
}
static inline void set_selection_property(xcb_connection_t * xc,
        const xcb_atom_t property) {
    jbxvt_set_property(xc, property, selection_data.length,
            selection_data.text);
}
//  Make the selection currently delimited by the selection end markers.
void jbxvt_make_selection(xcb_connection_t * xc) {
    jbxvt_save_selection(&selection_data);
    /* Set all properties which may possibly be requested.  */
    if (selection_data.text) { // don't set NULL data
        set_selection_property(xc, XCB_ATOM_PRIMARY);
        set_selection_property(xc, XCB_ATOM_SECONDARY);
        set_selection_property(xc, jbxvt_get_clipboard(xc));
    }
    xcb_set_selection_owner(xc, jbxvt_get_main_window(xc),
            XCB_ATOM_PRIMARY, XCB_CURRENT_TIME);
}
static void change_property(xcb_connection_t * xc,
        const xcb_window_t requestor, const xcb_atom_t property,
        const xcb_atom_t type) {
    xcb_change_property(xc, XCB_PROP_MODE_REPLACE, requestor,
            property, type, 8, selection_data.length,
            selection_data.text);
}
//  Respond to a request for our current selection.
void jbxvt_send_selection(xcb_connection_t * xc,
        const xcb_time_t time, const uint32_t requestor,
        const uint32_t type, const uint32_t property) {
#ifdef JBXVT_DEBUG_SELECTION
    LOG("jbxvt_send_selection, %d, %d, %d, %d", (int)time,
            requestor, type, property);
#endif // JBXVT_DEBUG_SELECTION
    if (selection_data.text) { // verify data non-null
        change_property(xc, requestor, property, type);
        xcb_selection_notify_event_t e = {
            .response_type = XCB_SELECTION_NOTIFY,
            .selection = XCB_ATOM_PRIMARY, .target = type,
            .requestor = requestor, .time = time, .property
                = property == XCB_NONE
                ? type : property}; // per ICCCM
        xcb_send_event(xc, true, requestor,
                XCB_SELECTION_NOTIFY, (char *)&e);
    }
}
//  Clear the current selection.
void jbxvt_clear_selection(void) {
    if (selection_data.text)
        free(selection_data.text);
    selection_data.text = NULL;
    selection_data.length = 0;
    selection_data.on_screen = false;
}
//  start a selection using the specified unit.
void jbxvt_start_selection(xcb_connection_t * xc,
        struct JBDim p, enum JBXVTSelectionUnit unit) {
    jbxvt_show_selection(xc); // clear previous
    jbxvt_clear_selection(); // free previous selection
    p = jbxvt_pixels_to_chars(p);
    jbxvt_rc_to_selend(p.y, p.x, &SE[2]);
    selection_data.unit = unit;
    selection_data.on_screen = true;
    SE[0] = SE[1] = SE[2];
    jbxvt_adjust_selection(&SE[1]);
    jbxvt_show_selection(xc);
}
/*  Determine if the current selection overlaps row1 through row2.
    If it does then remove it from the screen.  */
void jbxvt_check_selection(xcb_connection_t * xc,
        const int16_t row1, const int16_t row2) {
    if (selection_data.on_screen) {
        int16_t r1 = SE[0].index, r2 = SE[1].index;
        if (r1 > r2)
            JB_SWAP(int16_t, r1, r2);
        if (row2 >= r1 && row1 <= r2) {
            jbxvt_show_selection(xc);
            selection_data.on_screen = false;
        }
    }
}
