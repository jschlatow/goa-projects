/*
 * \brief   LVGL support library
 * \author  Josef Soentgen
 * \date    2022-10-19
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__LVGL_SUPPORT_H_
#define _INCLUDE__LVGL_SUPPORT_H_

/* Genode includes */
#include <base/env.h>

namespace Lvgl {

	struct Callback
	{
		virtual void operator() () = 0;
		virtual ~Callback() { }
	};

	struct Config
	{
		char const *title;

		unsigned initial_width;
		unsigned initial_height;

		/* frame buffer */
		bool allow_resize;
		bool use_refresh_sync;

		/* input */
		bool use_keyboard;
		bool use_mouse;

		/* timely event handling */
		bool use_periodic_timer;
		unsigned periodic_ms;

		Callback * resize_callback;
		Callback * timer_callback;
	};

	/**
	 * Initialize LVGL library and configure subsystems
	 */
	void init(Genode::Env &, Config);

	/**
	 * Called to advance LVGL's event loop
	 */
	void tick(unsigned);

} /* namespace Lvgl */

#endif /* _INCLUDE__LVGL_SUPPORT_H_ */
