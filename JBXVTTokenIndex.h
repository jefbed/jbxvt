// Copyright 2017, Jeffrey E. Bedard
#ifndef JBXVT_JBXVTTOKENINDEX_H
#define JBXVT_JBXVTTOKENINDEX_H
enum JBXVTTokenIndex {
	JBXVT_TOKEN_ALN = 1000, // screen alignment test (E-fill)
	JBXVT_TOKEN_ANSI1 = 1001, // ANSI conformance level 1
	JBXVT_TOKEN_ANSI2 = 1002, // ANSI conformance level 2
	JBXVT_TOKEN_ANSI3 = 1003, // ANSI conformance level 3
	JBXVT_TOKEN_APC = 0x9f,
	JBXVT_TOKEN_CHA = 'G', // cursor CHaracter Absolute column
	JBXVT_TOKEN_CHAR = 1004, // single character
	JBXVT_TOKEN_CHT = 0x49, // cursor horizontal tab
	JBXVT_TOKEN_CNL = 'E', // cursor next line
	JBXVT_TOKEN_CPL = 'F', // cursor prev line
	JBXVT_TOKEN_CS_ALT_G1 = '-', // set character set G1
	JBXVT_TOKEN_CS_ALT_G2 = '.', // set character set G2
	JBXVT_TOKEN_CS_ALT_G3 = '/', // set character set G3
	JBXVT_TOKEN_CS_DEF = 1005, // default character set
	JBXVT_TOKEN_CS_G0 = '(', // set character set G0
	JBXVT_TOKEN_CS_G1 = ')', // set character set G1
	JBXVT_TOKEN_CS_G2 = '*', // set character set G2
	JBXVT_TOKEN_CS_G3 = '+', // set character set G3
	JBXVT_TOKEN_CSI = 0x9b,
	JBXVT_TOKEN_CS_UTF8 = 1006, // utf-8 character set
	JBXVT_TOKEN_CUB = 'D', // cursor back
	JBXVT_TOKEN_CUD = 'B', // cursor down
	JBXVT_TOKEN_CUF = 'C', // cursor back
	JBXVT_TOKEN_CUP = 'H', // position cursor
	JBXVT_TOKEN_CUU = 'A', // cursor up
	JBXVT_TOKEN_DA = 'c', // device attributes request
	JBXVT_TOKEN_DCH = 'P', // delete characters
	JBXVT_TOKEN_DCS = 0x90,
	JBXVT_TOKEN_DHLB = 1007, // double height line, bottom half
	JBXVT_TOKEN_DHLT = 1008, // double height line, top half
	JBXVT_TOKEN_DL = 'M', // delete lines
	JBXVT_TOKEN_DSR = 'n', // report status or position
	JBXVT_TOKEN_DWL = 1009, // double width line
	JBXVT_TOKEN_ECH = 'X', // erase CHaracters
	JBXVT_TOKEN_ED = 'J', // erase to start or end of screen
	JBXVT_TOKEN_EL = 'K', // erase to start or end of line
	JBXVT_TOKEN_ELR = 'z', // Enable Locator Reporting
	JBXVT_TOKEN_ENTGM52 = 1010, // enter vt52 graphics mode (ESC F)
	JBXVT_TOKEN_ENTRY = 1011, // cursor crossed window boundary
	JBXVT_TOKEN_EOF = 1012, // end of file
	JBXVT_TOKEN_EPA = 0x97,
	JBXVT_TOKEN_ESC = 033,
	JBXVT_TOKEN_EXPOSE = 1013, // exposure event
	JBXVT_TOKEN_EXTGM52 = 1014, // exit vt52 graphics mode (ESC G)
	JBXVT_TOKEN_FOCUS = 1015, // keyboard focus event
	JBXVT_TOKEN_HOME = 1016, // move cursor to home position
	JBXVT_TOKEN_HPA = '`', // horizontal position absolute
	JBXVT_TOKEN_HPR = 'a', // horizontal position relative
	JBXVT_TOKEN_HTS = 0x88, // horizontal tab stop
	JBXVT_TOKEN_HVP = 'f', // horizontal and vertical position
	JBXVT_TOKEN_ICH = '@', // insert characters
	JBXVT_TOKEN_ID = 0x9a,
	JBXVT_TOKEN_IL = 'L', // insert lines
	JBXVT_TOKEN_IND = 0x84, // index
	JBXVT_TOKEN_LL = 'q', // load LEDs
	JBXVT_TOKEN_MC = 'i', // media copy
	JBXVT_TOKEN_MEMLOCK = 1017, // (HP) lock memory above cursor
	JBXVT_TOKEN_MEMUNLOCK = 1018, // (HP) unlock memory above cursor
	JBXVT_TOKEN_NEL = 0x85, // next line
	JBXVT_TOKEN_NULL = 0, // token to be ignored
	JBXVT_TOKEN_OSC = 0x9d,
	JBXVT_TOKEN_PAM = '=', // keypad to applications mode
	JBXVT_TOKEN_PM = 0x9e, // privacy message (ended by ESC \ (ST))
	JBXVT_TOKEN_PNM = '>', // keypad to numeric mode
	JBXVT_TOKEN_QUERY_SCA = 1019, // DSR cursor attributes
	JBXVT_TOKEN_QUERY_SCL = 1020,
	JBXVT_TOKEN_QUERY_SCUSR = 1021,
	JBXVT_TOKEN_QUERY_SGR = 1022, // DSR SGR style
	JBXVT_TOKEN_QUERY_SLRM = 1023, // DSR soft scroll mode
	JBXVT_TOKEN_QUERY_STBM = 1024, // DSR scroll margins
	JBXVT_TOKEN_RC = '8', // restore cursor position
	JBXVT_TOKEN_REQTPARAM = 'x', // REQuest Terminal PARAMeters
	JBXVT_TOKEN_RESET = 'l', // reset mode
	JBXVT_TOKEN_RESIZE = 1025, // main window resized
	JBXVT_TOKEN_RESTOREPM = '?', // restore private modes
	JBXVT_TOKEN_RI = 0x8d, // reverse index
	JBXVT_TOKEN_RIS = 1026, // reset to initial state
	JBXVT_TOKEN_RQM = 'p', // request DEC private mode
	JBXVT_TOKEN_S7C1T = 1027, // 7-bit controls
	JBXVT_TOKEN_S8C1T = 1028, // 8-bit controls
	JBXVT_TOKEN_SAVEPM = 's', // save private mode values
	JBXVT_TOKEN_SBDOWN = 1029, // scroll bar down
	JBXVT_TOKEN_SBGOTO = 1030, // set scroll bar position
	JBXVT_TOKEN_SBSWITCH = 1031, // toggle scroll bar
	JBXVT_TOKEN_SBUP = 1032, // scroll bar up
	JBXVT_TOKEN_SC = '7', // save cursor position
	JBXVT_TOKEN_SD = 'T', // Scroll Down # lines
	JBXVT_TOKEN_SELCLEAR = 1033, // selection clear request
	JBXVT_TOKEN_SELDRAG = 1034, // drag selection
	JBXVT_TOKEN_SELECT = 1035, // confirm the selection
	JBXVT_TOKEN_SELEXTND = 1036, // extend selection
	JBXVT_TOKEN_SELINSRT = 1037, // insert selection
	JBXVT_TOKEN_SELLINE = 1038, // select a line
	JBXVT_TOKEN_SELNOTIFY = 1039, // selection notify request
	JBXVT_TOKEN_SELREQUEST = 1040, // selection request
	JBXVT_TOKEN_SELSTART = 1041, // start selection
	JBXVT_TOKEN_SELWORD = 1042, // select a word
	JBXVT_TOKEN_SET = 'h', // set mode
	JBXVT_TOKEN_SGR = 'm', // set graphics rendition
	JBXVT_TOKEN_SOS = 0x98,
	JBXVT_TOKEN_SPA = 0x96,
	JBXVT_TOKEN_SS2 = 0x8e, // select G2 for next character only
	JBXVT_TOKEN_SS3 = 0x8f, // select G3 for next character only
	JBXVT_TOKEN_ST = 0x9c, // string terminator
	JBXVT_TOKEN_STBM = 'r', // set top and bottom margins
	JBXVT_TOKEN_STRING = 1043, // string of printable characters
	JBXVT_TOKEN_SU = 'S', // Scroll Up # lines
	JBXVT_TOKEN_SWL = 1044, // single width line
	JBXVT_TOKEN_TBC = 'g', // tab clear
	JBXVT_TOKEN_TXTPAR = 1045, // sequence with a text parameter
	JBXVT_TOKEN_VPA = 'd', // vertical position absolute
	JBXVT_TOKEN_VPR = 'e', // vertical position relative
};
#endif//!JBXVT_JBXVTTOKENINDEX_H