/*
 * \brief   Monitor a battery ROM and create bar widgets
 * \author  Johannes Schlatow
 * \date    2024-01-02
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _BATTERY_MONITOR_H_
#define _BATTERY_MONITOR_H_

#include <base/attached_rom_dataspace.h>
#include <util/reconstructible.h>
#include <util/list_model.h>

/* local includes */
#include "system_info.h"

using namespace Genode;

struct Battery_monitor : Info::Widget
{
	Attached_rom_dataspace           _rom;
	Signal_handler<Battery_monitor>  _sigh;
	lv_obj_t                       * _cont;
	Allocator                      & _alloc;

	struct Battery : List_model<Battery>::Element
	{
		using Name = String<32>;

		Name      name      {   };
		unsigned  max_value { 0 };
		unsigned  value     { 0 };

		Constructible<Info::Bar> widget { };

		static bool type_matches(Xml_node const & node) {
			return node.has_type("sb"); }

		bool matches(Xml_node const & node)
		{
			bool result = false;
			node.with_sub_node("name", [&] (Xml_node const & n) {
				if (n.decoded_content<Name>() == name)
					result = true;
			}, [&] () { });

			return result;
		}

		Battery(Xml_node const & node, lv_obj_t * cont)
		{
			node.with_sub_node("name", [&] (Xml_node const & n) {
				name = n.decoded_content<Name>();
			}, [&] () { });

			widget.construct(cont, name.string(), 0, 100);

			update(node);
		}

		void update(Xml_node const & node)
		{
			node.with_sub_node("last_full_capacity", [&] (Xml_node const & n) {
				max_value = n.attribute_value("value", 0);
			}, [&] () { });

			node.with_sub_node("remaining_capacity", [&] (Xml_node const & n) {
				value = n.attribute_value("value", 0);
			}, [&] () { });

			if (widget.constructed())
				widget->value(value * 100 / max_value);
		}

		void handle_resize()
		{
			if (widget.constructed())
				widget->handle_resize();
		}

	};

	List_model<Battery>          _widgets { };

	/* Noncopyable */
	Battery_monitor(Battery_monitor const &) = delete;
	void operator=(Battery_monitor const &) = delete;

	void handle_resize() override
	{
		_widgets.for_each([&] (Battery & bat) {
			bat.handle_resize();
		});
	}

	void update_from_xml(Xml_node const & node)
	{
		_widgets.update_from_xml(node,

			/* create fn */
			[&] (Xml_node const &node) -> Battery & {
				return *new (_alloc) Battery(node, _cont);
			},

			/* destroy fn */
			[&] (Battery &b) { destroy(_alloc, &b); },

			/* update fn */
			[&] (Battery &b, Xml_node const &node) { b.update(node); }
		);
	}

	void handle_update()
	{
		_rom.update();

		try {
			Libc::with_libc([&] () {
				update_from_xml(_rom.xml());
			});
		} catch (...) {
			error("Unable to complete update of battery ROM");
		}
	}

	Battery_monitor(Env & _env, Xml_node const & node, lv_obj_t * cont, Allocator & alloc)
	: _rom(_env, node.attribute_value("rom", Genode::String<32> { }).string()),
	  _sigh(_env.ep(), *this, &Battery_monitor::handle_update),
	  _cont(lv_obj_create(cont)),
	  _alloc(alloc)
	{
		/* set up a flex container to host all detected batteries */
		lv_obj_align(_cont, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_size(_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_obj_set_style_flex_flow(_cont, LV_FLEX_FLOW_COLUMN_WRAP, LV_PART_MAIN);
		lv_obj_set_style_flex_main_place(_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN);
		lv_obj_set_style_flex_cross_place(_cont, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);
		lv_obj_set_style_layout(_cont, LV_LAYOUT_FLEX, LV_PART_MAIN);
		lv_obj_set_style_bg_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_border_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_pad_all(_cont, 0, LV_PART_MAIN);

		_rom.sigh(_sigh);
		handle_update();
	}

	~Battery_monitor()
	{
		update_from_xml("<empty/>");
		lv_obj_del(_cont);
	}
};


#endif /* _ROM_MONITOR_H_ */
