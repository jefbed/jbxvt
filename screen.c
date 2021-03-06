/*  Copyright 2017, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey,
    University of Kent at Canterbury.  */
#include "screen.h"
#include <string.h>
#include "JBXVTPrivateModes.h"
#include "cursor.h"
#include "erase.h"
#include "libjb/log.h"
#include "mode.h"
#include "move.h"
#include "repaint.h"
#include "sbar.h"
#include "scr_reset.h"
#include "scroll.h"
#include "size.h"
static uint8_t screen_index; // current screen
static void init_screen(struct JBXVTScreen * scr)
{
    scr->margin.bottom = jbxvt_get_char_size().height - 1;
    scr->wrap_next = false;
    scr->margin.top = 0;
}
static struct JBXVTScreen * jbxvt_get_screens(void)
{
    static struct JBXVTScreen screens[2];
    static bool init_done;
    if (!init_done) {
        init_screen(screens);
        init_screen(screens + 1);
        init_done = true;
    }
    return screens;
}
// returns indexed screen, with i's validity sanitized
struct JBXVTScreen * jbxvt_get_screen_at(const uint8_t i)
{
    // default to 0 if i is not 1
    return jbxvt_get_screens() + (i == 1 ? 1 : 0);
}
// Returns a pointer to the current screen
struct JBXVTScreen * jbxvt_get_current_screen(void)
{
    // this, in effect, validates screen_index
    return jbxvt_get_screen_at(screen_index);
}
// Returns the specified row of the current screen
struct JBXVTLine * jbxvt_get_line(const uint8_t row)
{
    return jbxvt_get_current_screen()->line + row;
}
// Returns a pointer to the current screen's margin data
struct JBDim * jbxvt_get_margin(void)
{
    return &jbxvt_get_current_screen()->margin;
}
static void set_area_to_e(const int y, const struct JBDim c)
{
    if (y < c.h) {
        struct JBXVTScreen * current
            = jbxvt_get_current_screen();
        memset(current->line[y].text, 'E', c.w);
        memset(current->line[y].rend, 0, (size_t)(c.w << 2));
        set_area_to_e(y + 1, c);
    }
}
static inline void home(xcb_connection_t * xc)
{
    jbxvt_move(xc, 0, 0, 0);
}
// Set all chars to 'E'
void jbxvt_efill(xcb_connection_t * xc)
{
    LOG("jbxvt_efill");
    // Move to cursor home in order for all characters to appear.
    home(xc);
    set_area_to_e(0, jbxvt_get_char_size());
    jbxvt_repaint(xc);
}
//  Change between the alternate and the main screens
void jbxvt_change_screen(xcb_connection_t * xc, const bool mode_high)
{
    LOG("jbxvt_change_screen(xc, mode_high: %d)", mode_high);
    screen_index = mode_high ? 1 : 0;
    jbxvt_get_modes()->charsel = 0;
    /*  Do not call jbxvt_erase_screen(JBXVT_ERASE_ALL) here--It causes
        corruption of the saved line data in this context.  Test case:
        $ ls; vi
        q
        ls
        <scroll up>
     */
    jbxvt_reset(xc);
    home(xc);
    jbxvt_erase_screen(xc, JBXVT_ERASE_AFTER);
    jbxvt_clear_saved_lines(xc);
}
// Scroll from top to current bottom margin count lines, moving cursor
void jbxvt_index_from(xcb_connection_t * xc,
    const int8_t count, const int16_t top)
{
    jbxvt_set_scroll(xc, 0);
    jbxvt_draw_cursor(xc);
    scroll(xc, top, jbxvt_get_margin()->b, count);
    jbxvt_draw_cursor(xc);
}
