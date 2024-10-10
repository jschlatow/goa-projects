/*
 * \brief   Monitor a ROM and create table widget
 * \author  Johannes Schlatow
 * \date    2023-12-07
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _ROM_MONITOR_H_
#define _ROM_MONITOR_H_

#include <base/attached_rom_dataspace.h>
#include <util/reconstructible.h>

/* local includes */
#include "system_info.h"

using namespace Genode;

struct Rom_monitor : Info::Widget
{
	Attached_rom_dataspace       _rom;
	Signal_handler<Rom_monitor>  _sigh;
	Xml_node                     _xml;
	lv_obj_t                   * _cont;

	Constructible<Info::Tabular> _widget { };

	/* Noncopyable */
	Rom_monitor(Rom_monitor const &) = delete;
	void operator=(Rom_monitor const &) = delete;

	void _handle_node(Xml_node const & node, Xml_node const & rom_node)
	{
		if (!node.has_attribute("type")) return;

		Xml_node::Type type = node.attribute_value("type", Xml_node::Type());

		rom_node.for_each_sub_node(type.string(), [&] (Xml_node const & data_node) {
			bool matches { true };

			node.for_each_attribute([&] (Xml_attribute const & attr) {
				if (!matches) return;

				using Name = Xml_attribute::Name;
				Name const & name = attr.name();

				/* skip type attribute */
				if (name == "type") return;

				String<64> value { };
				attr.value<64>(value);

				String<64> attr_value = data_node.attribute_value(name.string(), String<64> { });
				if (attr_value != value)
					matches = false;
			});

			if (matches)
				_parse_xml(node, data_node);
		});
	}

	void _handle_row(Xml_node const & row, Xml_node const & rom_node)
	{
		using Name = String<64>;

		if (!row.has_attribute("attribute"))
			return;

		Name attr_name = row.attribute_value("attribute", Name());

		Name label = row.attribute_value("label", attr_name);

		_widget->add_row(label.string(),
		                 rom_node.attribute_value(attr_name.string(), Name()).string(),
		                 row.attribute_value("highlight", false));
	}

	void _parse_xml(Xml_node const & xml, Xml_node const & rom_node)
	{
		xml.for_each_sub_node([&] (Xml_node const & node) {
			if (node.has_type("node"))
				_handle_node(node, rom_node);
			else if (node.has_type("row"))
				_handle_row(node, rom_node);
		});
	}

	void handle_resize() override
	{
		if (_widget.constructed())
			_widget->handle_resize();
	}

	void handle_update()
	{
		static Genode::String<64> default_message { "no data" };

		_rom.update();

		Libc::with_libc([&] () {
			_widget->clear();

			Xml_node const & rom_node = _rom.xml();
			_parse_xml(_xml, rom_node);

			if (_widget->empty())
				_widget->add_merged_row(_xml.attribute_value("alt", default_message).string());
		});
	}

	Rom_monitor(Env & _env, Xml_node const & node, lv_obj_t * cont)
	: _rom(_env, node.attribute_value("rom", Genode::String<64> { }).string()),
	  _sigh(_env.ep(), *this, &Rom_monitor::handle_update),
	  _xml(node),
	  _cont(cont)
	{
		_rom.sigh(_sigh);

		_widget.construct(_cont);
		handle_update();
	}
};


#endif /* _ROM_MONITOR_H_ */
