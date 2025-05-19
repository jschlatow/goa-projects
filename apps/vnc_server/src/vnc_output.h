/*
 * \brief  Vnc framebuffer output
 * \author Johannes Schlatow
 * \date   2021-05-28
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VNC_OUTPUT_H_
#define _VNC_OUTPUT_H_

/* Genode includes */
#include <libc/component.h>
#include <capture_session/connection.h>
#include <event_session/connection.h>
#include <util/reconstructible.h>
#include <util/interface.h>

/* 3rd-party includes */
#include <libc/args.h>
#include <rfb/rfb.h>

/* local include */
#include "keycode_from_keysym.h"

namespace Vncserver {
	using namespace Genode;

	struct Output_control;
	class  Output;
}


struct Vncserver::Output_control : Interface
{
	virtual void enable()  = 0;
	virtual void disable() = 0;
};


class Vncserver::Output
{
	public:
		struct Libvnc_error   : Exception { };

		using Pixel          = Capture::Pixel;
		using Area           = Capture::Area;

	private:
		/*
		 * Noncopyable
		 */
		Output(Output const &);
		Output &operator= (Output const &);

		struct Button_state
		{
			enum {
				BUTTON_1 = 0x01,
				BUTTON_2 = 0x02,
				BUTTON_3 = 0x04,
				WHEEL_UP = 0x08,
				WHEEL_DN = 0x10,
				WHEEL_LEFT  = 0x20,
				WHEEL_RIGHT = 0x40,
				MASK = 0x7F
			};

			int _value;

			Button_state(int value)
			: _value(value & MASK)
			{ }

			Button_state &operator = (int value)
			{
				_value = value & MASK;
				return *this;
			}

			bool operator != (Button_state const &rhs)
			{ return _value != rhs._value; }

			bool changed(Button_state const &state, int button) const
			{ return (state._value ^ _value) & button & MASK; }

			bool pressed(int button) const
			{ return _value & button & MASK; }
		};

		Libc::Env       &_env;
		Allocator       &_alloc;
		Area             _area;
		Output_control & _output_control;

		Event::Connection _event_connection { _env };

		Constructible<Attached_rom_dataspace> _passwd_rom { };

		/* libvnc state */
		rfbScreenInfoPtr _screen = nullptr;
		Button_state _old_buttons { 0 };

		void _realloc_buffer()
		{
			const size_t size = _area.w * _area.h * sizeof(Pixel);

			char* oldfb = _screen->frameBuffer;
			char* newfb = new (_alloc) char[size];

			if (oldfb) {
				rfbNewFramebuffer(_screen, newfb,
				                  _area.w, _area.h,
				                  8, 3, sizeof(Pixel));
				destroy(_alloc, oldfb);
			}
			else
				_screen->frameBuffer = newfb;

			/* overwrite server pixel format */
			_screen->serverFormat.redShift   = Pixel::r_shift;
			_screen->serverFormat.greenShift = Pixel::g_shift;
			_screen->serverFormat.blueShift  = Pixel::b_shift;
		}

		template <typename T>
		T *_addr()
		{
			return static_cast<T *>((void*)_screen->frameBuffer);
		}

	public:

		Output(Libc::Env &env, Allocator &alloc, Area area, Output_control & ctrl)
		: _env(env), _alloc(alloc), _area(area), _output_control(ctrl)
		{
			/* rfbGetScreen takes options from argc/argv */
			int argc    = 0;
			char **argv = nullptr;
			char **envp = nullptr;
			populate_args_and_env(_env, argc, argv, envp);

			_screen = rfbGetScreen(&argc, argv,
			                       _area.w, _area.h,
			                       8, 3, sizeof(Pixel));
			if (!_screen)
				throw Libvnc_error();

			_screen->frameBuffer    = nullptr;
			_screen->alwaysShared   = FALSE;
			_screen->neverShared    = TRUE;

			/**
			 * disconnect current client if new client connects so that we
			 * always accept a connection even if the current client has not
			 * disconnected properly
			 */
			_screen->dontDisconnect = FALSE;

			/* set password callback */
			_screen->passwordCheck = [] (rfbClientPtr cl, const char* response, int len) {
				return (static_cast<Output*>(cl->screen->screenData))
					->check_password(cl, response, len);
			};

			/* store context for event callbacks in screenData member */
			_screen->screenData = static_cast<void*>(this);

			/* set ptr event callback */
			_screen->ptrAddEvent = [] (int buttons, int x, int y, rfbClientPtr cl) {
				(static_cast<Output*>(cl->screen->screenData))
					->handle_ptr(buttons, x, y);
				rfbDefaultPtrAddEvent(buttons, x, y, cl);
			};

			/* set keyboard event callback */
			_screen->kbdAddEvent = [] (rfbBool down, rfbKeySym key, rfbClientPtr cl) {
				(static_cast<Output*>(cl->screen->screenData))
					->handle_keysym(down, key);
			};

			/* set client hook */
			_screen->newClientHook = [] (rfbClientPtr cl) {
				(static_cast<Output*>(cl->screen->screenData))
					->handle_connect();

				cl->clientGoneHook = [] (rfbClientPtr cl) {
					(static_cast<Output*>(cl->screen->screenData))
						->handle_disconnect();
				};
				return RFB_CLIENT_ACCEPT;
			};

			/* allocate buffer */
			_realloc_buffer();

			/* initialize the server */
			rfbInitServer(_screen);

			/* start background loop (using pthread) */
			rfbRunEventLoop(_screen, -1, TRUE);
		}

		~Output()
		{
			if (_screen->frameBuffer)
				destroy(_alloc, _screen->frameBuffer);

			rfbScreenCleanup(_screen);
		}

		void resize(Area area)
		{
			_area = area;
			Libc::with_libc([&] () {
				_realloc_buffer();
			});
		}

		void handle_config(Xml_node const & config_xml)
		{
			if (config_xml.attribute_value("requires_password", false))
				_screen->authPasswdData = (void *)"passwd";
			else
				_screen->authPasswdData = NULL;
		}

		rfbBool check_password(rfbClientPtr cl, const char* response, int len)
		{
			using Password = Genode::String<64>;

			/* create a new ROM connection with label specified by authPasswdData */
			try {
				_passwd_rom.construct(_env, (char *)cl->screen->authPasswdData);
			} catch (...) {
				return FALSE;
			}

			/* invalid or empty dataspace -> no password */
			if (!_passwd_rom->valid() || _passwd_rom->size() == 0)
				return FALSE;

			Password passwd(_passwd_rom->local_addr<const char>());

			if (!passwd.valid())
				return FALSE;

			/* remove newline from tail */
			char * raw_passwd = (char *)passwd.string();
			if (raw_passwd[passwd.length()-2] == '\n')
				raw_passwd[passwd.length()-2] = '\0';

			/* encrypt passwd and store in authChallenge */
			rfbEncryptBytes(cl->authChallenge, raw_passwd);

			/* destruct ROM connection since we do not want to store password in memory */
			_passwd_rom.destruct();

			/* compare result with response */
			if (Genode::memcmp(cl->authChallenge, response, len) != 0) {
				rfbErr("Password authentication failed\n");
				return FALSE;
			}

			return TRUE;
		}

		void handle_connect()    { _output_control.enable();  }
		void handle_disconnect() { _output_control.disable(); }

		void handle_ptr(int buttons, int x, int y)
		{
			if (x < 0 || y < 0 || (unsigned)x >= _area.w || (unsigned)y >= _area.h)
				return;

			Button_state cur_buttons(buttons);

			_event_connection.with_batch([&] (Event::Session_client::Batch &batch) {
				batch.submit(Input::Absolute_motion{x, y});

				/* determine changed button states */
				if (_old_buttons != cur_buttons) {
					if (_old_buttons.changed(cur_buttons, Button_state::BUTTON_1)) {
						if (cur_buttons.pressed(Button_state::BUTTON_1))
							batch.submit(Input::Press  {Input::BTN_LEFT});
						else
							batch.submit(Input::Release{Input::BTN_LEFT});
					}
					if (_old_buttons.changed(cur_buttons, Button_state::BUTTON_2)) {
						if (cur_buttons.pressed(Button_state::BUTTON_2))
							batch.submit(Input::Press  {Input::BTN_MIDDLE});
						else
							batch.submit(Input::Release{Input::BTN_MIDDLE});
					}
					if (_old_buttons.changed(cur_buttons, Button_state::BUTTON_3)) {
						if (cur_buttons.pressed(Button_state::BUTTON_3))
							batch.submit(Input::Press  {Input::BTN_RIGHT});
						else
							batch.submit(Input::Release{Input::BTN_RIGHT});
					}
					if (_old_buttons.changed(cur_buttons, Button_state::WHEEL_UP)) {
						if (cur_buttons.pressed(Button_state::WHEEL_UP))
							batch.submit(Input::Wheel{0, 1});
					}
					if (_old_buttons.changed(cur_buttons, Button_state::WHEEL_DN)) {
						if (cur_buttons.pressed(Button_state::WHEEL_DN))
							batch.submit(Input::Wheel{0,-1});
					}
					if (_old_buttons.changed(cur_buttons, Button_state::WHEEL_LEFT)) {
						if (cur_buttons.pressed(Button_state::WHEEL_LEFT))
							batch.submit(Input::Wheel{-1,0});
					}
					if (_old_buttons.changed(cur_buttons, Button_state::WHEEL_RIGHT)) {
						if (cur_buttons.pressed(Button_state::WHEEL_RIGHT))
							batch.submit(Input::Wheel{1,0});
					}
				}
			});

			_old_buttons = buttons;
		}

		void handle_keysym(bool down, rfbKeySym keysym)
		{
			_event_connection.with_batch([&] (Event::Session_client::Batch &batch) {
				bool           pass_char = character_keysym(keysym);
				Input::Keycode keycode   = Input::KEY_UNKNOWN;

				if (!pass_char) {
					keycode = keycode_from_keysym(keysym);
					if (keycode == Input::KEY_UNKNOWN) return;
				}

				if (down) {
					if (pass_char)
						batch.submit(Input::Press_char{keycode, Codepoint{keysym}});
					else
						batch.submit(Input::Press     {keycode});
				}
				else
					batch.submit(Input::Release{keycode});
			});
		}

		void refresh(Capture::Rect const rect)
		{
			Libc::with_libc([&] () {
				rfbMarkRectAsModified(_screen, rect.x1(), rect.y1(),
				                               rect.x2(), rect.y2());
			});
		}

		Area area() const { return _area; }

		template <typename FN>
		void with_surface(FN const &fn)
		{
			Surface<Pixel> surface(_addr<Pixel>(), _area);

			fn(surface);
		}
};

#endif /* _VNC_OUTPUT_H_ */
