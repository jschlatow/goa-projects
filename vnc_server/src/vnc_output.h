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

/* 3rd-party includes */
#include <libc/args.h>
#include <rfb/rfb.h>

/* local include */
#include "keycode_from_keysym.h"

namespace Vncserver {
	using namespace Genode;

	struct Timer_ctrl;
	class Output;
}

struct Vncserver::Timer_ctrl
{
	Timer::Connection &_timer;
	unsigned long      _period_ms { 0 };
	bool               _enabled   { false };

	void _start() { _timer.trigger_periodic(1000*_period_ms); }
	void _stop()  { _timer.trigger_periodic(0); }

	Timer_ctrl(Timer::Connection &timer)
	: _timer(timer)
	{ }

	void update_period(unsigned long period_ms)
	{
		_period_ms = period_ms;
		if (_enabled) _start();
	}

	void enable()
	{
		_enabled = true;
		_start();
	}

	void disable()
	{
		_enabled = false;
		_stop();
	}
};


class Vncserver::Output
{
	public:
		struct Libvnc_error   : Exception { };

		using Pixel          = Capture::Pixel;
		using Area           = Capture::Area;
		using Affected_rects = Capture::Session::Affected_rects;

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
				BUTTON_4 = 0x08,
				BUTTON_5 = 0x10,
				BUTTON_6 = 0x80,
				MASK = 0x8F
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

		Libc::Env  &_env;
		Allocator  &_alloc;
		Timer_ctrl &_timer_ctrl;
		Area        _area;

		Event::Connection _event_connection { _env };

		/* libvnc state */
		rfbScreenInfoPtr _screen = nullptr;
		Button_state _old_buttons { 0 };

		void _realloc_buffer()
		{
			const size_t size = _area.w() * _area.h() * sizeof(Pixel);

			char* oldfb = _screen->frameBuffer;
			char* newfb = new (_alloc) char[size];

			if (oldfb) {
				rfbNewFramebuffer(_screen, newfb,
					               _area.w(), _area.h(),
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

		Output(Libc::Env &env, Allocator &alloc, Area area, Timer_ctrl &timer)
		: _env(env), _alloc(alloc), _timer_ctrl(timer), _area(area)
		{
			/* rfbGetScreen takes options from argc/argv */
			int argc    = 0;
			char **argv = nullptr;
			char **envp = nullptr;
			populate_args_and_env(_env, argc, argv, envp);

			_screen = rfbGetScreen(&argc, argv,
				                    _area.w(), _area.h(),
				                    8, 3, sizeof(Pixel));
			if (!_screen)
				throw Libvnc_error();

			_screen->frameBuffer  = nullptr;
			_screen->neverShared  = TRUE;

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
				if (cl->screen->clientHead->next)
					return RFB_CLIENT_REFUSE;

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

		void handle_connect()    { _timer_ctrl.enable();  }
		void handle_disconnect() { _timer_ctrl.disable(); }

		void handle_ptr(int buttons, int x, int y)
		{
			if (x < 0 || y < 0 || (unsigned)x >= _area.w() || (unsigned)y >= _area.h())
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
					if (_old_buttons.changed(cur_buttons, Button_state::BUTTON_4)) {
						/* wheel up */
						if (cur_buttons.pressed(Button_state::BUTTON_4))
							warning("Wheel events not implemented");
							/* FIXME batch.submit(Input::Wheel{0, 1}); */
					}
					if (_old_buttons.changed(cur_buttons, Button_state::BUTTON_5)) {
						/* wheel down */
						if (cur_buttons.pressed(Button_state::BUTTON_5))
							warning("Wheel events not implemented");
							/* FIXME batch.submit(Input::Wheel{0,-1}); */
					}
					if (_old_buttons.changed(cur_buttons, Button_state::BUTTON_6)) {
						if (cur_buttons.pressed(Button_state::BUTTON_6))
							batch.submit(Input::Press  {Input::BTN_SIDE});
						else
							batch.submit(Input::Release{Input::BTN_SIDE});
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
