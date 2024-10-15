/*
 * \brief   Monitor a ROM and create dynamic label widget
 * \author  Johannes Schlatow
 * \date    2024-10-15
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _ROM_LABEL_H_
#define _ROM_LABEL_H_

#include <base/attached_rom_dataspace.h>
#include <util/reconstructible.h>

/* local includes */
#include "system_info.h"

using namespace Genode;

struct Rom_label : Info::Widget
{
	Attached_rom_dataspace       _rom;
	Signal_handler<Rom_label>    _sigh;
	lv_obj_t                   * _cont;

	Constructible<Info::Label>   _widget { };

	/* Noncopyable */
	Rom_label(Rom_label const &) = delete;
	void operator=(Rom_label const &) = delete;

	void handle_resize() override
	{
		if (_widget.constructed())
			_widget->handle_resize();
	}

	void handle_update()
	{
		_rom.update();

		Libc::with_libc([&] () {
			_widget->set_text(_rom.local_addr<char const>());
		});
	}

	Rom_label(Env & _env, Session_label const & label, lv_obj_t * cont, unsigned font_size)
	: _rom(_env, label.string()),
	  _sigh(_env.ep(), *this, &Rom_label::handle_update),
	  _cont(cont)
	{
		_rom.sigh(_sigh);

		_widget.construct(_cont, _rom.local_addr<char const>(), font_size);
		handle_update();
	}
};


#endif /* _ROM_LABEL_H_ */
