/*  Copyright 2016, Jeffrey E. Bedard
    Copyright 1992, 1997 John Bovey, University of Kent at Canterbury.*/

#include "cmdtok.h"

#include "cursor.h"
#include "jbxvt.h"
#include "libjb/log.h"
#include "libjb/xcb.h"
#include "lookup_key.h"
#include "screen.h"
#include "xevents.h"

#include <errno.h>
#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef OPENBSD
#include <sys/select.h>
#endif//OPENBSD

// Input buffer is empty
#define GCC_NULL 0x100

//  Flags used to control get_com_char();
enum ComCharFlags {BUF_ONLY=1, GET_XEVENTS=2};

// Shortcuts
#define XC jbxvt.X.xcb
#define COM jbxvt.com
#define BUF jbxvt.com.buf
#define CFD COM.fd

static struct JBXVTEvent * ev_alloc(xcb_generic_event_t * restrict e)
{
	struct JBXVTEvent * xe = calloc(1, sizeof(struct JBXVTEvent));
	xe->type = e->response_type;
	return xe;
}

static void put_xevent(struct JBXVTEvent * xe)
{
	struct JBXVTEventQueue * q = &jbxvt.com.events;
	xe->next = q->start;
	xe->prev = NULL;
	*(xe->next ? &xe->next->prev : &q->last) = xe;
}

static void handle_focus(xcb_generic_event_t * restrict e)
{
	xcb_focus_in_event_t * f = (xcb_focus_in_event_t *)e;
	if (f->mode)
		  return;
	switch (f->detail) {
	case XCB_NOTIFY_DETAIL_ANCESTOR:
	case XCB_NOTIFY_DETAIL_INFERIOR:
	case XCB_NOTIFY_DETAIL_NONLINEAR:
		break;
	default:
		return;
	}
	struct JBXVTEvent * xe = ev_alloc(e);
	xe->detail = f->detail;
	put_xevent(xe);
}

#define XESET(a, b) xe->a = e->b
#define XEEQ(a) XESET(a, a)
#define EALLOC(t) struct JBXVTEvent * xe = ev_alloc(ge); t * e = (t*)ge;

static void handle_sel(xcb_generic_event_t * restrict ge)
{
	EALLOC(xcb_selection_request_event_t);
	XEEQ(time); XEEQ(requestor); XEEQ(target); XEEQ(property);
	XESET(window, owner);
	put_xevent(xe);
}

static void handle_client_msg(xcb_generic_event_t * restrict ge)
{
	xcb_client_message_event_t * e = (xcb_client_message_event_t *)ge;
	if (e->format == 32 && e->data.data32[0]
		== (unsigned long)wm_del_win())
		  exit(0);
}

static void handle_expose(xcb_generic_event_t * restrict ge)
{
	EALLOC(xcb_expose_event_t);
	XEEQ(window); XESET(box.x, x); XESET(box.y, y);
	XESET(box.width, width); XESET(box.height, height);
	put_xevent(xe);
}

static void handle_other(xcb_generic_event_t * restrict ge)
{
	EALLOC(xcb_key_press_event_t);
	XESET(window, event); XESET(box.x, event_x); XESET(box.y, event_y);
	XEEQ(state); XEEQ(time); XESET(button, detail);
	put_xevent(xe);
}

static void key_press(xcb_generic_event_t * restrict e)
{
	int_fast16_t count = 0;
	uint8_t * s = lookup_key(e, &count);
	if (count) {
		jbxvt.com.send_nxt = s;
		jbxvt.com.send_count = count;
	}
}

static bool handle_xev(void)
{
	jb_check_x(XC);
	xcb_generic_event_t * event = xcb_poll_for_event(XC);
	if (!event)
		return false;
	switch (event->response_type & ~0x80) {
	case XCB_KEY_PRESS:
		key_press(event);
		break;
	case XCB_FOCUS_IN:
	case XCB_FOCUS_OUT:
		handle_focus(event);
		break;
	case XCB_SELECTION_REQUEST:
	case XCB_SELECTION_NOTIFY:
		handle_sel(event);
		break;
	case XCB_CLIENT_MESSAGE:
		handle_client_msg(event);
		break;
	case XCB_EXPOSE:
	case XCB_GRAPHICS_EXPOSURE:
		handle_expose(event);
		break;
	default:
		handle_other(event);
	}
	free(event);
	return true;
}

static int_fast16_t output_to_command(void)
{
	struct JBXVTCommandData * c = &jbxvt.com;
	const ssize_t count = write(c->fd, c->send_nxt,
		c->send_count);
	if (jb_check(count != -1, "Could not write to command"))
		exit(1);
	c->send_count -= count;
	c->send_nxt += count;
	return count;
}

static void timer(void)
{
	if (jbxvt.mode.att610)
		return;
	switch(jbxvt.opt.cursor_attr) {
	case 0: // blinking block
	case 1: // blinking block
	case 3: // blinking underline
	case 5: // blinking bar
	case 7: // blinking overline
		draw_cursor();
		break;
	}
}

__attribute__((nonnull))
static void poll_io(fd_set * restrict in_fdset)
{
	FD_SET(jbxvt.com.fd, in_fdset);
	FD_SET(jbxvt.com.xfd, in_fdset);
	fd_set out_fdset;
	FD_ZERO(&out_fdset);
	if (jbxvt.com.send_count > 0)
		FD_SET(jbxvt.com.fd, &out_fdset);
	errno = 0; // Ensure next error message is accurate
	if (jb_check(select(jbxvt.com.width, in_fdset, &out_fdset, NULL,
		&(struct timeval){.tv_usec = 500000}) != -1,
		"Error performing select()"))
		return;
	if (FD_ISSET(jbxvt.com.fd, &out_fdset))
		output_to_command();
	else if (!FD_ISSET(jbxvt.com.xfd, in_fdset))
		timer(); // select timed out
	else
		jb_check_x(jbxvt.X.xcb);
}

static bool get_buffered(int_fast16_t * val, const int_fast8_t flags)
{
	bool r = false;
	if ((r = (COM.stack.top > COM.stack.data)))
		*val = *--COM.stack.top;
	else if ((r = (COM.buf.next < COM.buf.top)))
		*val = *COM.buf.next++;
	else if ((r = (flags & BUF_ONLY)))
		*val = GCC_NULL;
	return r;
}

/*  Return the next input character after first passing any keyboard input
 *  to the command.  If flags & BUF_ONLY is true then only buffered characters are
 *  returned and once the buffer is empty the special value GCC_NULL is
 *  returned.  If flags and GET_XEVENTS is true then GCC_NULL is returned
 *  when an X event arrives.
 */
// This is the most often called function.
#if defined(__i386__) || defined(__amd64__)
__attribute__((hot,regparm(1)))
#else
__attribute__((hot))
#endif
static int_fast16_t get_com_char(const int_fast8_t flags)
{
	int_fast16_t ret = 0;
	if (get_buffered(&ret, flags))
		return ret;
	xcb_flush(XC);
	fd_set in;
input:
	FD_ZERO(&in);
	if (handle_xev() && (flags & GET_XEVENTS))
		return GCC_NULL;
	poll_io(&in);
	if (!FD_ISSET(CFD, &in))
		goto input;
	const uint8_t l = read(CFD, BUF.data, COM_BUF_SIZE);
	if (l < 1)
		return errno == EWOULDBLOCK ? GCC_NULL : EOF;
	BUF.next = BUF.data;
	BUF.top = BUF.data + l;
	return *BUF.next++;
}

//  Return true if the character is one that can be handled by scr_string()
#if defined(__i386__) || defined(__amd64__)
	__attribute__((hot,const,regparm(1)))
#else
	__attribute__((hot,const))
#endif//x86
static inline bool is_string_char(register int_fast16_t c)
{
	return c < 0x7f && (c >= ' ' || c == '\n' || c == '\r' || c == '\t');
}

#if defined(__i386__) || defined(__amd64__)
	__attribute__((nonnull,regparm(1)))
#else
	__attribute__((nonnull))
#endif//x86
static void handle_string_char(int_fast16_t c, struct Token * restrict tk)
{
	uint_fast16_t i = 0;
	tk->nlcount = 0;
	do {
		tk->string[i++] = c;
		c = get_com_char(1);
		if (c == '\n')
			++tk->nlcount;
	} while (is_string_char(c) && i < TKS_MAX);
	tk->length = i;
	tk->string[i] = 0;
	tk->type = TK_STRING;
	if (c != GCC_NULL)
		  put_com_char(c);
}

#if defined(__i386__) || defined(__amd64__)
	__attribute__((nonnull,regparm(1)))
#else
	__attribute__((nonnull))
#endif//x86
static void start_esc(int_fast16_t c, struct Token * restrict tk)
{
	c = get_com_char(0);
	if (c >= '<' && c <= '?') {
		tk->private = c;
		c = get_com_char(0);
	}

	//  read any numerical arguments
	uint_fast16_t i = 0;
	do {
		uint_fast16_t n = 0;
		while (c >= '0' && c <= '9') { // is a number
			// Advance position and convert
			n = n * 10 + c - '0';
			c = get_com_char(0); // next digit
		}
		if (i < TK_MAX_ARGS)
			  tk->arg[i++] = n;
		if (c == TK_ESC)
			  put_com_char(c);
		if (c < ' ')
			  return;
		if (c < '@')
			  c = get_com_char(0);
	} while (c < '@' && c >= ' ');
	if (c == TK_ESC)
		  put_com_char(c);
	tk->nargs = i;
	tk->type = c;
}

#if defined(__i386__) || defined(__amd64__)
	__attribute__((nonnull,regparm(1)))
#else
	__attribute__((nonnull))
#endif//x86
static void end_esc(int_fast16_t c, struct Token * restrict tk)
{
	c = get_com_char(0);
	uint_fast16_t n = 0;
	while (c >= '0' && c <= '9') {
		n = n * 10 + c - '0';
		c = get_com_char(0);
	}
	tk->arg[0] = n;
	tk->nargs = 1;
	c = get_com_char(0);
	register uint_fast16_t i = 0;
	while ((c & 0177) >= ' ' && i < TKS_MAX) {
		if (c >= ' ')
			  tk->string[i++] = c;
		c = get_com_char(0);
	}
	tk->length = i;
	tk->string[i] = 0;
	tk->type = TK_TXTPAR;
}

static void check_st(struct Token * t)
{
	int_fast16_t c = get_com_char(0);
	if (c != TK_ST)
		t->type = TK_NULL;
}

static void start_dcs(struct Token * t)
{
	int_fast16_t c = get_com_char(0);
	switch (c) {
	case '0':
	case '1':
		LOG("FIXME: User defined keys are unimplemented.");
		return;
	case '$':
		c = get_com_char(0);
		if (c != 'q')
			return;
		// RQSS:  Request status string
		c = get_com_char(0); // next char
		switch (c) {
		case '"':
			c = get_com_char(0); // next
			switch (c) {
#define CASE_Q(ch, tk) case ch:t->type=TK_QUERY_##tk;check_st(t);break;
			CASE_Q('p', SCA);
			CASE_Q('q', SCL);
			}
			break;
		CASE_Q('m', SLRM);
		CASE_Q('r', STBM);
		CASE_Q('s', SLRM);
		case ' ':
			c = get_com_char(0);
			if (c != 'q')
				return;
			t->type = TK_QUERY_SCUSR;
			check_st(t);
			break;
		}
		break;
	case '+':
		c = get_com_char(0);
		switch (c) {
		case 'p':
		case 'q':
			LOG("FIXME: termcap support unimplemented");
		}
		break;
	case 0x1b:
		get_com_char(0);
		get_com_char(0);
		put_com_char('-');
		break;
	default:
		LOG("Unhandled DCS, starting with 0x%x", (int)c);
	}
}

#if defined(__i386__) || defined(__amd64__)
	__attribute__((nonnull,regparm(1)))
#else
	__attribute__((nonnull))
#endif//x86
static void handle_esc(int_fast16_t c, struct Token * restrict tk)
{
	c = get_com_char(0);
	switch(c) {
	case '[': // CSI
		start_esc(c, tk);
		break;
	case ']': // OSC
		end_esc(c, tk);
		break;
	case ' ':
		c = get_com_char(0);
		switch (c) {
#define CASE_A(ch, tok, a) case ch: tk->type = tok, tk->arg[0] = a;\
			tk->nargs=1; break;
#define CASE_T(ch, tok) case ch: tk->type = tok; break;
#define CASE_M(ch, mod, v) case ch: jbxvt.mode.mod = v; break;
		CASE_T('F', TK_S7C1T);
		CASE_T('G', TK_S8C1T);
		CASE_T('L', TK_ANSI1);
		CASE_T('M', TK_ANSI2);
		CASE_T('N', TK_ANSI3);
		}
		break;
	case '#':
		c = get_com_char(0);
		switch(c) {
		CASE_T('3', TK_DHLT);
		CASE_T('4', TK_DHLB);
		CASE_T('5', TK_SWL);
		CASE_T('6', TK_DWL);
		CASE_T('8', TK_ALN);
		}
		break;

	case '(': // G0 charset
	CASE_A(')', c, get_com_char(0));

	case '%': // UTF charset switch
		c = get_com_char(0);
		switch (c) {
		CASE_T('@', TK_CS_DEF);
		CASE_T('G', TK_CS_UTF8);
		}
		break;
	CASE_T('6', TK_RI); // BI: back index
	CASE_T('9', TK_IND); // FI: forward index
	case '7': // SC: save cursor
	case '8': // RC: restore cursor
	case '=': // PAM: keypad to application mode
	case '>': // PNM: keypad to numeric mode
		tk->type = c;
		break;
	CASE_T('^', TK_PM); // PM: Privacy message (ended by ESC \)
	CASE_T('\\', TK_ST);
	CASE_M('<', decanm, true); // exit vt52 mode
	case 'A': // vt52 cursor up
	case 'B': // vt52 cursor down
	case 'C': // vt52 cursor left
		tk->type = c;
		break;
	CASE_T('D', jbxvt.mode.decanm ? TK_IND : TK_CUF);
	CASE_T('c', TK_RIS); // Reset to Initial State
	CASE_M('e', dectcem, true); // enable cursor (vt52 GEMDOS)
	CASE_M('f', dectcem, false); // disable cursor (vt52 GEMDOS)
	CASE_T('E', TK_NEL);
	CASE_T('F', TK_ENTGM52); // Enter VT52 graphics mode
	CASE_T('G', TK_EXTGM52); // Leave VT52 graphics mode
	CASE_T('H', jbxvt.mode.decanm ? TK_HTS : TK_HOME);
	CASE_A('l', jbxvt.mode.decanm ? TK_MEMLOCK : TK_EL, 2);
	case 'I':
		scr_index_from(-1, jbxvt.scr.current->cursor.y);
		tk->type = TK_CUU;
		break;
	CASE_A('J', TK_EL, 1); // vt52 erase to end of line
	CASE_T('j', TK_SC); // save cursor (vt52g)
	CASE_T('K', TK_ED); // vt42 erase to end of screen
	CASE_T('k', TK_RC); // restore cursor (vt52g)
	CASE_T('L', TK_IL); // insert line (vt52)
	CASE_T('M', jbxvt.mode.decanm ? TK_RI : TK_DL);
	CASE_T('N', TK_SS2);
	CASE_T('O', TK_SS3);
	CASE_A('o', TK_EL, 1); // clear to start of line (vt52g)
	case 'P':
		start_dcs(tk);
		break;
	CASE_M('p', decscnm, true); // reverse video mode (vt52g)
	CASE_M('q', decscnm, false); // normal video (vt52g)
	CASE_T('V', TK_SPA);
	CASE_M('v', decawm, true); // wrap on
	CASE_T('W', TK_EPA);
	CASE_M('w', decawm, false); // wrap off
	CASE_T('X', TK_SOS);
	case 'Y':
		tk->type = TK_CUP;
		// -32 to decode, + 1 to be vt100 compatible
		tk->arg[1] = get_com_char(0) - 31;
		tk->arg[0] = get_com_char(0) - 31;
		tk->nargs = 2;
	case 'Z':
		if (jbxvt.mode.decanm) // vt100+ mode
			tk->type = TK_ID;
		else // I am a VT52
			cprintf("\033/Z");
		break;
	}
}

static void default_token(struct Token * restrict tk, int_fast16_t c)
{
	switch(c) { // handle 8-bit controls
	case TK_APC:
	case TK_CSI:
	case TK_DCS:
	case TK_EPA:
	case TK_HTS:
	case TK_ID:
	case TK_IND:
	case TK_NEL:
	case TK_OSC:
	case TK_PM:
	case TK_RI:
	case TK_SOS:
	case TK_SPA:
	case TK_SS2:
	case TK_SS3:
	case TK_ST:
		tk->type = c;
		break;
	case 0xe2:
		c = get_com_char(c);
		switch (c) {
		case 0x94:
			c = get_com_char(c);
			switch (c) {
			case 0x80:
				put_com_char('-');
				break;
			case 0x82:
				put_com_char('|');
				break;
			case 0xac:
				put_com_char('+');
				break;
			default:
				LOG("0x%x", (unsigned int)c);
				put_com_char(c);
			}
			break;
		case 0x96:
		case 0x80:
			put_com_char('-');
			break;
		default:
			LOG("0xe2 0x%x", (unsigned int)c);
			put_com_char(c);
		}
		break;
	default:
		if (is_string_char(c))
			handle_string_char(c, tk);
		else {
			tk->type = TK_CHAR;
			tk->tk_char = c;
		}
	}
}

//  Return an input token
void get_token(struct Token * restrict tk)
{
	memset(tk, 0, sizeof(struct Token));
	// set token per event:
	if(handle_xevents(tk))
		  return;
	const int_fast16_t c = get_com_char(GET_XEVENTS);
	switch (c) {
	CASE_T(GCC_NULL, TK_NULL);
	CASE_T(EOF, TK_EOF);
	case TK_ESC:
		handle_esc(c, tk);
		break;
	case TK_CSI: // 8-bit CSI
		// Catch this here, since 7-bit CSI is parsed above.
		LOG("CC_CSI");
		start_esc(c, tk);
		break;
	case TK_DCS: // 8-bit DCS
		start_dcs(tk);
		break;
	default:
		default_token(tk, c);
	}
}

