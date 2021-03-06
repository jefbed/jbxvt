/*  Copyright 2017, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey, University of Kent at Canterbury.*/
#include "xevents.h"
#include <unistd.h>
#include "JBXVTPrivateModes.h"
#include "button_events.h"
#include "command.h"
#include "libjb/JBDim.h"
#include "libjb/log.h"
#include "libjb/xcb.h"
#include "lookup_key.h"
#include "mode.h"
#include "mouse.h"
#include "sbar.h"
#include "scr_reset.h"
#include "selection.h"
#include "selex.h"
#include "selreq.h"
#include "window.h"
static unsigned long wm_del_win(xcb_connection_t * xc)
{
    static unsigned long a;
    if (!a) { // Init on first call:
        a = jb_get_atom(xc, "WM_DELETE_WINDOW");
        xcb_change_property(xc, XCB_PROP_MODE_REPLACE,
            jbxvt_get_main_window(xc),
            jb_get_atom(xc, "WM_PROTOCOLS"),
            XCB_ATOM_ATOM, 32, 1, &a);
    }
    return a;
}
static void handle_client_message(xcb_connection_t * xc,
    xcb_client_message_event_t * e)
{
    if (e->format == 32 && e->data.data32[0] == wm_del_win(xc)) {
        LOG("WM_DEL_WIN");
        exit(0);
    } else {
        LOG("not WM_DEL_WIN");
    }
}
static void handle_expose(xcb_connection_t * xc,
    xcb_expose_event_t * e)
{
    if (e->window == jbxvt_get_scrollbar(xc))
        jbxvt_draw_scrollbar(xc);
    else
        jbxvt_reset(xc);
}
static void key_press(xcb_connection_t * xc, void * e)
{
    int_fast16_t count = 0;
    uint8_t * s = jbxvt_lookup_key(xc, e, &count);
    if (s)
        jb_require(write(jbxvt_get_fd(), s,
            (size_t)count) != -1,
            "Could not write to command");
}
static void handle_motion_notify(xcb_connection_t * xc,
    xcb_motion_notify_event_t * e)
{
    const xcb_window_t w = e->event;
    const struct JBDim b = {.x = e->event_x, .y = e->event_y};
    if (w == jbxvt_get_scrollbar(xc)
        && (e->state & XCB_KEY_BUT_MASK_BUTTON_2))
        jbxvt_scroll_to(xc, b.y);
    else if (jbxvt_get_mouse_motion_tracked())
        jbxvt_track_mouse(e->detail, e->state, b, JBXVT_MOTION);
    else if ((e->state & XCB_KEY_BUT_MASK_BUTTON_1)
        && !(e->state & XCB_KEY_BUT_MASK_CONTROL)
        && !jbxvt_get_mouse_tracked())
        jbxvt_extend_selection(xc, b, true);
}
static void handle_focus(const bool in)
{
    if (jbxvt_get_modes()->mouse_focus_evt)
        dprintf(jbxvt_get_fd(), "%s%c]",
            jbxvt_get_csi(), in ? 'I' : 'O');
}
static void handle_selection_notify(xcb_connection_t * xc,
    xcb_selection_notify_event_t * e)
{
    jbxvt_paste_primary(xc, e->time, e->requestor, e->property);
}
static void handle_selection_request(xcb_connection_t * xc,
    xcb_selection_request_event_t * e)
{
    jbxvt_send_selection(xc, e->time, e->requestor, e->target,
        e->property);
}
// Handle X event on queue.  Return true if event was handled.
bool jbxvt_handle_xevents(xcb_connection_t * xc)
{
    jb_check_x(xc);
    static bool init;
    if (!init) {
        // Set up the wm_del_win property here.
        wm_del_win(xc);
    }
    xcb_generic_event_t * event = xcb_poll_for_event(xc);
    if (!event) // nothing to process
        return false;
    bool ret = true;
    switch (event->response_type & ~0x80) {
    // Put things to ignore here:
    case 0: // Unimplemented, undefined, no event
    case 150: // Undefined
    case XCB_KEY_RELEASE: // Unimplemented
    case XCB_MAP_NOTIFY:
    case XCB_NO_EXPOSURE: // Unimplemented
    case XCB_REPARENT_NOTIFY: // handle here to ensure cursor filled.
        ret = false;
        break;
    case XCB_CONFIGURE_NOTIFY:
        jbxvt_resize_window(xc);
        break;
    case XCB_KEY_PRESS:
        key_press(xc, event);
        break;
    case XCB_CLIENT_MESSAGE:
        handle_client_message(xc,
            (xcb_client_message_event_t *)event);
        break;
    case XCB_EXPOSE:
    case XCB_GRAPHICS_EXPOSURE:
        handle_expose(xc, (xcb_expose_event_t *)event);
        break;
    case XCB_ENTER_NOTIFY:
    case XCB_FOCUS_IN:
        handle_focus(true);
        break;
    case XCB_LEAVE_NOTIFY:
    case XCB_FOCUS_OUT:
        handle_focus(false);
        break;
    case XCB_SELECTION_CLEAR:
        jbxvt_clear_selection();
        break;
    case XCB_SELECTION_NOTIFY:
        handle_selection_notify(xc,
            (xcb_selection_notify_event_t *)event);
        break;
    case XCB_SELECTION_REQUEST:
        handle_selection_request(xc,
            (xcb_selection_request_event_t *)event);
        break;
    case XCB_BUTTON_PRESS:
        jbxvt_handle_button_press_event(xc,
            (xcb_button_press_event_t *)event);
        break;
    case XCB_BUTTON_RELEASE:
        jbxvt_handle_button_release_event(xc,
            (xcb_button_release_event_t *)event);
        break;
    case XCB_MOTION_NOTIFY:
        handle_motion_notify(xc,
            (xcb_motion_notify_event_t *)event);
        break;
    default:
        LOG("Unhandled event %d", event->response_type);
    }
    free(event);
    return ret;
}
