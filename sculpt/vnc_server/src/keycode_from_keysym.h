/*
 * \brief  Translates X11 keysyms to keycodes
 * \author Johannes Schlatow
 * \date   2021-05-31
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _KEYCODE_FROM_KEYSYM_H_
#define _KEYCODE_FROM_KEYSYM_H_

#include <input/keycodes.h>
#include <rfb/keysym.h>

namespace Vncserver {
	using namespace Genode;
	using namespace Input;

	static Input::Keycode keycode_from_keysym(rfbKeySym keysym) {
		if (keysym >= XK_KP_Enter && keysym <= XK_KP_Equal) {
			static Input::Keycode map[] = {
				KEY_KPENTER,       /* 0x8D */
				KEY_UNKNOWN,       /* 0x8E */
				KEY_UNKNOWN,       /* 0x8F */
				KEY_UNKNOWN,       /* 0x90 */
				KEY_UNKNOWN,       /* 0x91 */
				KEY_UNKNOWN,       /* 0x92 */
				KEY_UNKNOWN,       /* 0x93 */
				KEY_UNKNOWN,       /* 0x94 */
				KEY_HOME,          /* 0x95 */
				KEY_LEFT,          /* 0x96 */
				KEY_UP,            /* 0x97 */
				KEY_RIGHT,         /* 0x98 */
				KEY_DOWN,          /* 0x99 */
				KEY_PAGEUP,        /* 0x9a */
				KEY_PAGEDOWN,      /* 0x9b */
				KEY_END,           /* 0x9c */
				KEY_HOME,          /* 0x9d */
				KEY_INSERT,        /* 0x9e */
				KEY_DELETE,        /* 0x9f */
				KEY_UNKNOWN,       /* 0xa0 */
				KEY_UNKNOWN,       /* 0xa1 */
				KEY_UNKNOWN,       /* 0xa2 */
				KEY_UNKNOWN,       /* 0xa3 */
				KEY_UNKNOWN,       /* 0xa4 */
				KEY_UNKNOWN,       /* 0xa5 */
				KEY_UNKNOWN,       /* 0xa6 */
				KEY_UNKNOWN,       /* 0xa7 */
				KEY_UNKNOWN,       /* 0xa8 */
				KEY_UNKNOWN,       /* 0xa9 */
				KEY_KPASTERISK,    /* 0xaa */
				KEY_KPPLUS,        /* 0xab */
				KEY_KPCOMMA,       /* 0xac */
				KEY_KPMINUS,       /* 0xad */
				KEY_KPDOT,         /* 0xae */
				KEY_KPSLASH,       /* 0xaf */
				KEY_KP0,           /* 0xb0 */
				KEY_KP1,           /* 0xb1 */
				KEY_KP2,           /* 0xb2 */
				KEY_KP3,           /* 0xb3 */
				KEY_KP4,           /* 0xb4 */
				KEY_KP5,           /* 0xb5 */
				KEY_KP6,           /* 0xb6 */
				KEY_KP7,           /* 0xb7 */
				KEY_KP8,           /* 0xb8 */
				KEY_KP9,           /* 0xb9 */
				KEY_UNKNOWN,       /* 0xba */
				KEY_UNKNOWN,       /* 0xbb */
				KEY_UNKNOWN,       /* 0xbc */
				KEY_KPEQUAL        /* 0xbd */
			};
			return map[keysym - XK_KP_Enter];
		}
		else if (keysym >= XK_Home && keysym < XK_Select) {
			static Input::Keycode map[] = {
				KEY_HOME,       /* 0x50 */
				KEY_LEFT,       /* 0x51 */
				KEY_UP,         /* 0x52 */
				KEY_RIGHT,      /* 0x53 */
				KEY_DOWN,       /* 0x54 */
				KEY_PAGEUP,     /* 0x55 */
				KEY_PAGEDOWN,   /* 0x56 */
				KEY_END,        /* 0x57 */
				KEY_HOME,       /* 0x58 */
				KEY_UNKNOWN     /* 0x59 */
			};
			return map[keysym - XK_Home];
		}
		else if (keysym >= XK_F1 && keysym <= XK_F12) {
			static Input::Keycode map[] = {
				KEY_F1,       /* 0xbe */
				KEY_F2,       /* 0xbf */
				KEY_F3,       /* 0xc0 */
				KEY_F4,       /* 0xc1 */
				KEY_F5,       /* 0xc2 */
				KEY_F6,       /* 0xc3 */
				KEY_F7,       /* 0xc4 */
				KEY_F8,       /* 0xc5 */
				KEY_F9,       /* 0xc6 */
				KEY_F10,      /* 0xc7 */
				KEY_F11,      /* 0xc8 */
				KEY_F12,      /* 0xc9 */
			};
			return map[keysym - XK_F1];
		}
		else if (keysym >= XK_Select && keysym <= XK_Break) {
			/* TODO contains select, print, execute, insert, undo, redo, menu,
 			 *      find, cancel, help, break*/
			return KEY_UNKNOWN;
		}
		else if (keysym >= XK_Shift_L && keysym <= XK_Super_R) {
			static Input::Keycode map[] = {
				KEY_LEFTSHIFT,   /* 0xe1 */
				KEY_RIGHTSHIFT,  /* 0xe2 */
				KEY_LEFTCTRL,    /* 0xe3 */
				KEY_RIGHTCTRL,   /* 0xe4 */
				KEY_CAPSLOCK,    /* 0xe5 */
				KEY_UNKNOWN,     /* 0xe6 */
				KEY_LEFTMETA,    /* 0xe7 */
				KEY_RIGHTMETA,   /* 0xe8 */
				KEY_LEFTALT,     /* 0xe9 */
				KEY_RIGHTALT,    /* 0xea */
				KEY_LEFTMETA,    /* 0xeb */
				KEY_RIGHTMETA,   /* 0xec */
			};
			return map[keysym - XK_Shift_L];
		}
		else if (keysym == XK_Delete) {
			return KEY_DELETE;
		}
		else if (keysym == XK_ISO_Level3_Shift) {
			return KEY_RIGHTALT;
		}
		else if (keysym >= XK_BackSpace && keysym <= XK_Escape) {
			static Input::Keycode map[] = {
				KEY_BACKSPACE,  /* 0x08 */
				KEY_TAB,        /* 0x09 */
				KEY_LINEFEED,   /* 0x0A */
				KEY_CLEAR,      /* 0x0B */
				KEY_UNKNOWN,    /* 0x0C */
				KEY_ENTER,      /* 0x0D */
				KEY_UNKNOWN,    /* 0x0E */
				KEY_UNKNOWN,    /* 0x0F */
				KEY_UNKNOWN,    /* 0x10 */
				KEY_UNKNOWN,    /* 0x11 */
				KEY_UNKNOWN,    /* 0x12 */
				KEY_UNKNOWN,    /* 0x13 */
				KEY_SCROLLLOCK, /* 0x14 */
				KEY_SYSRQ,      /* 0x15 */
				KEY_UNKNOWN,    /* 0x16 */
				KEY_UNKNOWN,    /* 0x17 */
				KEY_UNKNOWN,    /* 0x18 */
				KEY_UNKNOWN,    /* 0x19 */
				KEY_UNKNOWN,    /* 0x1a */
				KEY_ESC,        /* 0x1b */
			};
			return map[keysym - XK_BackSpace];
		}

		return KEY_UNKNOWN;
	}

	static bool character_keysym(rfbKeySym keysym) {
		if (keysym >= XK_space && keysym <= XK_ydiaeresis)
			return true;
		return false;
	}
}

#endif /* _KEYCODE_FROM_KEYSYM_H_ */
