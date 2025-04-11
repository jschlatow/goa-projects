/*
 * \brief  Vnc server using libvnc
 * \author Johannes Schlatow
 * \date   2021-05-26
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


#include <base/env.h>
#include <libc/component.h>
#include <base/log.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <capture_session/connection.h>
#include <timer_session/connection.h>

#include "vnc_output.h"

namespace Vncserver {

	using namespace Genode;

	template <typename>
	class  Rate_limiter;
	struct Main;
}


template <typename HANDLER>
class Vncserver::Rate_limiter : public Output_control
{
	private:

		using Handler_method   = void (HANDLER::*)();
		using One_shot_timeout = Timer::One_shot_timeout<Rate_limiter>;

		Env                        & _env;
		Capture::Connection        & _capture;
		HANDLER                    & _handler;
		Handler_method const         _handler_method;
		Timer::Connection            _timer   { _env };
		One_shot_timeout             _timeout { _timer, *this, &Rate_limiter::_handle_timeout };
		Signal_handler<Rate_limiter> _wakeup_handler {
			_env.ep(), *this, &Rate_limiter::_handle_wakeup };

		Duration _period     { Milliseconds { 40 } };  /* max. 25 fps */
		Duration _last_frame { Microseconds { 0 } };

		bool     _idle       { true };

		void _handle_timeout(Duration)
		{
			_last_frame = _timer.curr_time();
			(_handler.*_handler_method)();
			_capture.capture_stopped();
		}

		void _handle_wakeup()
		{
			if (_idle) return;

			Duration now      = _timer.curr_time();
			Duration earliest = _last_frame;
			earliest.add(_period.trunc_to_plain_ms());
			earliest.add(Milliseconds { 1 }); /* tolerance */
			if (earliest.less_than(now)) {
				/* call handler_method only if last call was > _period in the past */
				_last_frame = now;
				(_handler.*_handler_method)();
				_capture.capture_stopped();
			} else if (!_timeout.scheduled()) {
				/* postpone the call of handler_method */
				auto diff = earliest.trunc_to_plain_us().value - now.trunc_to_plain_us().value;
				_timeout.schedule({ Microseconds { diff } });
			}
		}

	public:

		Rate_limiter(Env & env, Capture::Connection & capture,
		             HANDLER & handler, Handler_method method)
		: _env(env), _capture(capture),
		  _handler(handler),
		  _handler_method(method)
		{
			_capture.wakeup_sigh(_wakeup_handler);
			_capture.capture_stopped();
		}

		void update_period(Milliseconds period) { _period = Duration({period}); }

		/**
		* Output_control interface
		*/

		void enable() override
		{
			_idle = false;
			_last_frame = _timer.curr_time();
			(_handler.*_handler_method)();
			_capture.capture_stopped();
		}

		void disable() override
		{
			_timeout.discard();
			_idle = true;
		}
};


struct Vncserver::Main
{
	Libc::Env &_env;

	using Pixel          = Capture::Pixel;
	using Area           = Capture::Area;
	using Affected_rects = Capture::Session::Affected_rects;

	Attached_rom_dataspace    _config { _env, "config" };

	Heap                      _heap   { _env.ram(), _env.rm() };

	static Capture::Point _point_from_xml(Xml_node const &node)
	{
		return Capture::Point(node.attribute_value("xpos", 0L),
		                      node.attribute_value("ypos", 0L));
	}

	static Area _area_from_xml(Xml_node const &node, Area default_area)
	{
		return Area(node.attribute_value("width",  default_area.w),
		            node.attribute_value("height", default_area.h));
	}

	static Area _safe_screen_size(Area default_area)
	{
		if (default_area.w > 0 && default_area.h > 0)
			return default_area;
		else
			return Area(320, 240);
	}

	Capture::Connection  _capture      { _env };
	Rate_limiter<Main>   _rate_limiter { _env, _capture, *this, &Main::_handle_capture };
	Output               _output       { _env,
	                                     _heap,
	                                     _safe_screen_size(_capture.screen_size()),
	                                     _rate_limiter };

	struct Capture_input
	{
		Env &_env;

		Capture::Connection &_capture;

		Area const _area;

		bool _capture_buffer_init = ( _capture.buffer({ .px = _area,
		                                                .mm = { },
		                                                .viewport = _area}), true );

		Attached_dataspace _capture_ds { _env.rm(), _capture.dataspace() };

		Texture<Pixel> const _texture { _capture_ds.local_addr<Pixel>(), nullptr, _area };

		Capture::Point _at { };

		Capture_input(Env &env,
		              Capture::Connection &capture,
		              Area area,
		              Xml_node const &config)
		:
			_env(env),
			_capture(capture),
			_area(area),
			_at(_point_from_xml(config))
		{ }

		Affected_rects capture() { return _capture.capture_at(_at); }

		template <typename FN>
		void with_texture(FN const &fn) const
		{
			fn(_texture);
		}
	};

	Constructible<Capture_input> _capture_input { };

	void _handle_capture()
	{
		if (!_capture_input.constructed())
			return;

		_capture_input->with_texture([&] (Texture<Pixel> const &texture) {

			_output.with_surface([&] (Surface<Pixel> &surface) {
				Affected_rects const affected = _capture_input->capture();

				affected.for_each_rect([&] (Capture::Rect const rect) {

					surface.clip(rect);

					Blit_painter::paint(surface, texture, Capture::Point(0, 0));
				});

				affected.for_each_rect([&] (Capture::Rect const rect) {
					_output.refresh(rect);
				});
			});
		});

	}

	void _handle_resize()
	{
		Xml_node const &config = _config.xml();

		Area area = _area_from_xml(config, _safe_screen_size(_capture.screen_size()));

		_output.resize(area);

		_capture_input.construct(_env, _capture, area, config);
	}

	void _handle_config()
	{
		_config.update();
		_output.handle_config(_config.xml());

		_handle_resize();

		unsigned long const period_ms = _config.xml().attribute_value("period_ms", 0U);

		if (period_ms == 0)
			warning("missing or invalid 'period_ms' config attribute");

		_rate_limiter.update_period(Milliseconds { period_ms });
	}

	Signal_handler<Main> _resize_handler { _env.ep(), *this, &Main::_handle_resize };
	Signal_handler<Main> _config_handler { _env.ep(), *this, &Main::_handle_config };

	Main(Libc::Env &env) : _env(env)
	{
		_config.sigh(_config_handler);
		_capture.screen_size_sigh(_resize_handler);

		_handle_config();
	}
};


void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] () {
		static Vncserver::Main main(env);
	});
}
