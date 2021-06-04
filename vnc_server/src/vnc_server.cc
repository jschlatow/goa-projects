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
/*#include <base/registry.h>*/
#include <capture_session/connection.h>
#include <timer_session/connection.h>

#include "vnc_output.h"

namespace Vncserver {

	using namespace Genode;

	struct Main;
}

struct Vncserver::Main
{
	Libc::Env &_env;

	using Pixel          = Capture::Pixel;
	using Area           = Capture::Area;
	using Affected_rects = Capture::Session::Affected_rects;

	Attached_rom_dataspace    _config { _env, "config" };

	Heap                      _heap   { _env.ram(), _env.rm() };

	static Capture::Point _point_from_xml(Xml_node node)
	{
		return Capture::Point(node.attribute_value("xpos", 0L),
		                  node.attribute_value("ypos", 0L));
	}

	static Area _area_from_xml(Xml_node node, Area default_area)
	{
		return Area(node.attribute_value("width",  default_area.w()),
			         node.attribute_value("height", default_area.h()));
	}

	Capture::Connection  _capture    { _env };
	Timer::Connection    _timer      { _env };
	Timer_ctrl           _timer_ctrl { _timer };
	Output               _output     { _env,
	                                   _heap,
	                                   _capture.screen_size(),
	                                   _timer_ctrl };

	struct Capture_input
	{
		Env &_env;

		Capture::Connection &_capture;

		Area const _area;

		bool _capture_buffer_init = ( _capture.buffer(_area), true );

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

	Signal_handler<Main> _timer_handler { _env.ep(), *this, &Main::_handle_timer };

	void _handle_timer()
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
		Xml_node const config = _config.xml();

		Area area = _area_from_xml(config, _capture.screen_size());

		_output.resize(area);

		_capture_input.construct(_env, _capture, area, config);
	}

	void _handle_config()
	{
		_config.update();
		_handle_resize();

		unsigned long const period_ms = _config.xml().attribute_value("period_ms", 0U);

		if (period_ms == 0)
			warning("missing or invalid 'period_ms' config attribute");

		_timer_ctrl.update_period(period_ms);
	}

	Signal_handler<Main> _resize_handler { _env.ep(), *this, &Main::_handle_resize };
	Signal_handler<Main> _config_handler { _env.ep(), *this, &Main::_handle_config };

	Main(Libc::Env &env) : _env(env)
	{
		_timer.sigh(_timer_handler);
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
