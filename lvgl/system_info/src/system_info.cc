/*
 * \brief   LVGL System Info
 * \author  Johannes Schlatow
 * \date    2023-12-05
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/log.h>
#include <base/heap.h>
#include <base/registry.h>
#include <libc/component.h>
#include <util/reconstructible.h>
#include <util/color.h>

/* library includes */
#include <lvgl_support.h>
#include <sys/time.h>
#include <stdlib.h>

/* local includes */
#include "system_info.h"
#include "rom_tabular.h"
#include "rom_label.h"
#include "battery_monitor.h"

using namespace Genode;

template <typename T>
struct Registered_widget : T
{
	Registry<Info::Widget>::Element _element;

	template <typename... ARGS>
	Registered_widget(Registry<Info::Widget> & registry, ARGS&&... args)
	: T(args...), _element(registry, *this) { }
};

struct Main
{
	Env                    &_env;
	Attached_rom_dataspace  _config_rom { _env, "config" };
	Signal_handler<Main>    _sigh       { _env.ep(), *this, &Main::handle_config };
	Heap                    _heap       { _env.ram(), _env.rm() };

	Constructible<Info::Layout> _layout          { };
	Registry<Info::Widget>      _widget_registry { };

	struct Resize_callback : Lvgl::Callback
	{

		Main & main;

		Resize_callback(Main & main) : main(main) { }

		void operator() () override { main.handle_resize(); }

	} _resize_callback { *this };

	struct Timer_callback : Lvgl::Callback
	{

		Main & main;

		Timer_callback(Main & main) : main(main) { }

		void operator() () override { main.update_date_time(); }

	} _timer_callback { *this };

	Lvgl::Config _lvgl_config { .title              = "System Info",
	                            .initial_width      = 800,
	                            .initial_height     = 600,
	                            .allow_resize       = true,
	                            .use_refresh_sync   = true,
	                            .use_alpha          = true,
	                            .use_keyboard       = false,
	                            .use_mouse          = false,
	                            .use_periodic_timer = true,
	                            .periodic_ms        = 5000,
	                            .resize_callback    = &_resize_callback,
	                            .timer_callback     = &_timer_callback,
	};


	void handle_resize()
	{
		Libc::with_libc([&] {
			_widget_registry.for_each([&] (Info::Widget & widget) {
				widget.handle_resize();
			});

			_layout->handle_resize();
		});
	}

	void update_date_time()
	{
		Libc::with_libc([&] {

			struct timespec tp;
			if (clock_gettime(CLOCK_REALTIME, &tp)) {
				warning("Unable to get system time");
				return;
			}

			_widget_registry.for_each([&] (Info::Widget & widget) {
				widget.tick(tp.tv_sec);
			});

		});
	}

	void _handle_rom_nodes(Xml_node const & xml, lv_obj_t * cont, String<32> const & default_tz)
	{
		/* iterate XML nodes and create widgets */
		xml.for_each_sub_node([&] (Xml_node const & node) {
			String<32> tz = node.attribute_value("timezone", default_tz);

			if (node.has_type("clock"))
				new (_heap) Registered_widget<Info::Clock>(_widget_registry,
				                                           cont, tz.string());
			else if (node.has_type("calendar"))
				new (_heap) Registered_widget<Info::Calendar>(_widget_registry,
				                                           cont, tz.string());
			else if (node.has_type("tabular"))
				new (_heap) Registered_widget<Rom_tabular>(_widget_registry,
				                                           _env, node, cont);
			else if (node.has_type("battery"))
				new (_heap) Registered_widget<Battery_monitor>(_widget_registry,
				                                               _env, node, cont,
				                                               _heap);
			else if (node.has_type("tabular"))
				new (_heap) Registered_widget<Rom_tabular>(_widget_registry,
				                                           _env, node, cont);
			else if (node.has_type("label")) {
				String<64> rom_label  = node.attribute_value("rom", String<64>());
				unsigned   label_size = node.attribute_value("size", 28);

				if (rom_label.length())
					new (_heap) Registered_widget<Rom_label>(_widget_registry,
						_env, rom_label, cont, label_size);
				else
					new (_heap) Registered_widget<Info::Label>(
						_widget_registry, cont,
						node.decoded_content<String<128>>().string(),
						label_size);
			}
			else if (node.has_type("track")) {
				Info::Track * track = new (_heap) Registered_widget<Info::Track>(
					_widget_registry, cont,
					node.attribute_value("align", Info::Alignment::MID));
				track->with_container([&] (lv_obj_t * track) {
					_handle_rom_nodes(node, track, tz); });
			}
		});
	}

	void handle_config()
	{
		/* remove all widgets */
		_widget_registry.for_each([&] (Info::Widget & widget) {
			destroy(&_heap, &widget);
		});

		_config_rom.update();

		Xml_node xml = _config_rom.xml();
		Color      color      = xml.attribute_value("background_color",
		                                            Color(17, 85, 136));
		String<32> default_tz = xml.attribute_value("default_timezone",
		                                            Genode::String<32>("UTC-01"));

		Libc::with_libc([&] {
			lv_color_t c = lv_color_make(color.r, color.g, color.b);
			Info::set_background(c, color.a);

			_layout->with_container([&] (lv_obj_t * cont) {
				_handle_rom_nodes(_config_rom.xml(), cont, default_tz);
			});
		});

		update_date_time();
	}

	Main(Env &env) : _env(env)
	{
		Libc::with_libc([&] {
			Lvgl::init(_env, _lvgl_config);
			_layout.construct();
		});

		handle_config();
		_config_rom.sigh(_sigh);
	}
};


void Libc::Component::construct(Libc::Env &env)
{
	static Main main(env);
}
