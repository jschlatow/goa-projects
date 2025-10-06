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

#ifndef _ROM_TABULAR_H_
#define _ROM_TABULAR_H_

#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <util/reconstructible.h>

/* local includes */
#include "system_info.h"

using namespace Genode;

struct Rom_tabular : Info::Widget
{
	Attached_rom_dataspace       _rom;
	Signal_handler<Rom_tabular>  _sigh;
	Buffered_node                _config;
	lv_obj_t                   * _cont;

	Constructible<Info::Tabular> _widget { };

	/* Noncopyable */
	Rom_tabular(Rom_tabular const &) = delete;
	void operator=(Rom_tabular const &) = delete;

	void _handle_node(Node const & node, Node const & rom_node)
	{
		if (!node.has_attribute("type")) return;

		Node::Type type = node.attribute_value("type", Node::Type());

		rom_node.for_each_sub_node(type.string(), [&] (Node const & data_node) {
			bool matches { true };

			node.for_each_attribute([&] (Node::Attribute const & attr) {
				if (!matches) return;

				using Name = Node::Attribute::Name;
				Name const & name = attr.name;

				/* skip type attribute */
				if (name == "type") return;

				data_node.attribute_value(name.string(),
				                          String<64> { }).with_span([&] (Span const &span) {
					if (!span.equals(attr.value))
						matches = false;
				});
			});

			if (matches)
				_parse_node(node, data_node);
		});
	}

	void _handle_row(Node const & row, Node const & rom_node)
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

	void _parse_node(Node const & node, Node const & rom_node)
	{
		node.for_each_sub_node([&] (Node const & subnode) {
			if (subnode.has_type("node"))
				_handle_node(subnode, rom_node);
			else if (subnode.has_type("row"))
				_handle_row(subnode, rom_node);
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

			Node const & rom_node = _rom.node();
			_parse_node(_config, rom_node);

			if (_widget->empty())
				_widget->add_merged_row(_config.attribute_value("alt", default_message).string());
		});
	}

	Rom_tabular(Env & _env, Allocator & _alloc, Node const & node, lv_obj_t * cont)
	: _rom(_env, node.attribute_value("rom", Genode::String<64> { }).string()),
	  _sigh(_env.ep(), *this, &Rom_tabular::handle_update),
	  _config(_alloc, node),
	  _cont(cont)
	{
		_rom.sigh(_sigh);

		_widget.construct(_cont);
		handle_update();
	}
};


#endif /* _ROM_TABULAR_H_ */
