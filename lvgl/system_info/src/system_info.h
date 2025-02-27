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

#ifndef _SYSTEM_INFO_H_
#define _SYSTEM_INFO_H_

#include <time.h>
#include <lvgl.h>

namespace Info {

	struct Widget;
	struct Label;
	struct Clock;
	struct Bar;
	struct Calendar;
	struct Layout;
	struct Track;
	struct Tabular;

	void set_background(lv_color_t, unsigned);

	enum Alignment { TOP, MID, BOTTOM };

	inline size_t ascii_to(char const *s, Alignment &result)
	{
		using Genode::strcmp;
		if (!strcmp(s, "top",    3)) { result = Alignment::TOP;    return 3; }
		if (!strcmp(s, "mid",    3)) { result = Alignment::MID;    return 3; }
		if (!strcmp(s, "center", 6)) { result = Alignment::MID;    return 6; }
		if (!strcmp(s, "bottom", 6)) { result = Alignment::BOTTOM; return 6; }

		return 0;
	}

	inline unsigned column_size() {
		if (LV_HOR_RES < 800)
			return 250;
		else if (LV_HOR_RES < 1200)
			return 300;
		else if (LV_HOR_RES < 1920)
			return 400;
		else
			return 600;
	}

	inline unsigned scaled_font(unsigned size) {
		unsigned base_size       { 12 };
		unsigned current_scaling { size / base_size };
		if (current_scaling == 0) current_scaling = 1;

		unsigned offset { 16 };
		if (LV_HOR_RES < 800)
			offset = 0;
		if (LV_HOR_RES < 1200)
			offset = 2;
		else if (LV_HOR_RES < 1920)
			offset = 6;

		return size + (offset * current_scaling);
	}

	inline const lv_font_t * font_by_size(unsigned size) {
		const lv_font_t * fonts[] = { &lv_font_montserrat_8,
		                              &lv_font_montserrat_10,
		                              &lv_font_montserrat_12,
		                              &lv_font_montserrat_14,
		                              &lv_font_montserrat_16,
		                              &lv_font_montserrat_18,
		                              &lv_font_montserrat_20,
		                              &lv_font_montserrat_22,
		                              &lv_font_montserrat_24,
		                              &lv_font_montserrat_26,
		                              &lv_font_montserrat_28,
		                              &lv_font_montserrat_30,
		                              &lv_font_montserrat_32,
		                              &lv_font_montserrat_34,
		                              &lv_font_montserrat_36,
		                              &lv_font_montserrat_38,
		                              &lv_font_montserrat_40,
		                              &lv_font_montserrat_42,
		                              &lv_font_montserrat_44,
		                              &lv_font_montserrat_46,
		                              &lv_font_montserrat_48,
		                              &lv_font_montserrat_60,
		                              &lv_font_montserrat_60,
		                              &lv_font_montserrat_60,
		                              &lv_font_montserrat_60,
		                              &lv_font_montserrat_60,
		                              &lv_font_montserrat_60,
		                              &lv_font_montserrat_72,
		                              &lv_font_montserrat_72,
		                              &lv_font_montserrat_72,
		                              &lv_font_montserrat_72,
		                              &lv_font_montserrat_72,
		                              &lv_font_montserrat_72 };

		unsigned sanitized_fontsize = size;
		if (sanitized_fontsize < 8)
			sanitized_fontsize = 8;
		else if (sanitized_fontsize > 72)
			sanitized_fontsize = 72;

		unsigned font_index = (sanitized_fontsize / 2) - 4;
		return fonts[font_index];
	}
}


struct Info::Widget
{
	virtual void tick(time_t) { }
	virtual void handle_resize() { }
	virtual ~Widget() { }
};


struct Info::Label : Widget
{
	lv_style_t _style        { };
	lv_style_t _style_shadow { };
	lv_obj_t * _cont         { };
	lv_obj_t * _label        { };
	lv_obj_t * _shadow       { };
	char       _text[128]    { };
	unsigned   _fontsize;

	/* Noncopyable */
	Label(Label const &) = delete;
	void operator= (Label const &) = delete;

	void handle_resize() override
	{
		unsigned font_size        = scaled_font(_fontsize);
		const    lv_font_t * font = font_by_size(font_size);
		unsigned offset           = font_size/10;

		lv_style_set_text_font(&_style, font);
		lv_style_set_text_font(&_style_shadow, font);
		lv_obj_align_to(_shadow, _label, LV_ALIGN_TOP_LEFT, offset, offset);
	}

	void set_text(char const * text)
	{
		strncpy(_text, text, sizeof(_text)-1);

		lv_label_set_text(_label,  _text);
		lv_label_set_text(_shadow, lv_label_get_text(_label));
	}

	Label(lv_obj_t * parent, char const *text, unsigned fontsize)
	: _fontsize(fontsize)
	{
		/* set up a container */
		_cont = lv_obj_create(parent);
		lv_obj_align(_cont, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_size(_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_obj_set_style_bg_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_border_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_pad_all(_cont, 0, LV_PART_MAIN);

		/* set style */
		lv_style_init(&_style);
		lv_style_set_pad_bottom(&_style, 20);

		/* set shadow style */
		lv_style_init(&_style_shadow);
		lv_style_set_text_opa(&_style_shadow, LV_OPA_50);
		lv_style_set_text_color(&_style_shadow, lv_color_black());

		/*Create a label for the shadow first (it's in the background)*/
		_shadow = lv_label_create(_cont);
		lv_obj_add_style(_shadow, &_style_shadow, 0);

		/*Create the main label*/
		_label = lv_label_create(_cont);
		lv_obj_add_style(_label, &_style, 0);

		/*Position the main label*/
		lv_obj_align(_label, LV_ALIGN_CENTER, 0, 0);

		set_text(text);

		handle_resize();
	}

	~Label() override { lv_obj_del(_cont); }
};


struct Info::Clock : Widget
{
	lv_style_t _style        { };
	lv_style_t _style_shadow { };
	lv_obj_t * _cont         { };
	lv_obj_t * _label        { };
	lv_obj_t * _shadow       { };
	char       _tz[32]       { };

	/* Noncopyable */
	Clock(Clock const &) = delete;
	void operator= (Clock const &) = delete;

	void tick(time_t epoch) override
	{
		setenv("TZ", _tz, 1);

		struct tm * tm = localtime(&epoch);

		lv_label_set_text_fmt(_label,  "%02d:%02d", (unsigned)tm->tm_hour, (unsigned)tm->tm_min);
		lv_label_set_text_fmt(_shadow, lv_label_get_text(_label));
	}

	void handle_resize() override
	{
		unsigned font_size        { scaled_font(32) };
		const    lv_font_t * font { font_by_size(font_size) };

		lv_style_set_text_font(&_style, font);
		lv_style_set_text_font(&_style_shadow, font);
		lv_obj_align_to(_shadow, _label, LV_ALIGN_TOP_LEFT, font_size/10, font_size/10);
	}

	Clock(lv_obj_t * parent, char const *tz)
	{
		strncpy(_tz, tz, sizeof(_tz)-1);

		/* set up a container */
		_cont = lv_obj_create(parent);
		lv_obj_align(_cont, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_size(_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_obj_set_style_bg_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_border_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_pad_all(_cont, 0, LV_PART_MAIN);

		/* set style */
		lv_style_init(&_style);
		lv_style_set_pad_bottom(&_style, 20);

		/* set shadow style */
		lv_style_init(&_style_shadow);
		lv_style_set_text_opa(&_style_shadow, LV_OPA_50);
		lv_style_set_text_color(&_style_shadow, lv_color_black());

		/*Create a label for the shadow first (it's in the background)*/
		_shadow = lv_label_create(_cont);
		lv_obj_add_style(_shadow, &_style_shadow, 0);

		/*Create the main label*/
		_label = lv_label_create(_cont);
		lv_obj_add_style(_label, &_style, 0);

		/*Position the main label*/
		lv_obj_align(_label, LV_ALIGN_CENTER, 0, 0);

		handle_resize();
	}

	~Clock() override { lv_obj_del(_cont); }
};


struct Info::Bar : Widget
{
	lv_style_t _style        { };
	lv_style_t _style_shadow { };
	lv_obj_t * _cont         { };
	lv_obj_t * _bar          { };
	lv_obj_t * _label        { };
	lv_obj_t * _shadow       { };
	char       _title[32]    { };

	enum {
		INCREASING = LV_STATE_USER_1,
		DECREASING = LV_STATE_USER_2
	};

	/* Noncopyable */
	Bar(Bar const &) = delete;
	void operator= (Bar const &) = delete;

	void value(unsigned val)
	{
		lv_bar_set_value(_bar, val, LV_ANIM_OFF);
	}

	void rate(int r)
	{
		lv_obj_clear_state(_bar, INCREASING);
		lv_obj_clear_state(_bar, DECREASING);

		if (r > 0)
			lv_obj_add_state(_bar, INCREASING);
		else if (r < 0)
			lv_obj_add_state(_bar, DECREASING);
	}

	void handle_resize() override
	{
		unsigned font_size          { scaled_font(14) };
		const    lv_font_t * font   { font_by_size(font_size) };

		lv_obj_set_size(_bar, column_size()-2*font_size, font_size+4);
		lv_style_set_pad_bottom(&_style, font_size+6);

		lv_style_set_text_font(&_style, font);
		lv_style_set_text_font(&_style_shadow, font);
		lv_obj_align_to(_shadow, _label, LV_ALIGN_TOP_LEFT, font_size/5, font_size/5);
	}

	static void _draw_part_event_cb(lv_event_t * e)
	{
		lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
		if(dsc->part != LV_PART_INDICATOR) return;

		lv_obj_t * obj = lv_event_get_target(e);

		lv_draw_label_dsc_t label_dsc;
		lv_draw_label_dsc_init(&label_dsc);
		label_dsc.font = font_by_size(scaled_font(10));

		char state = '\0';
		if (lv_obj_has_state(obj, INCREASING))
			state = '+';
		else if (lv_obj_has_state(obj, DECREASING))
			state = '-';

		char buf[8];
		lv_snprintf(buf, sizeof(buf), "%d%%%c", (int)lv_bar_get_value(obj), state);

		lv_point_t txt_size;
		lv_txt_get_size(&txt_size, buf, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX,
		                label_dsc.flag);

		lv_area_t txt_area;
		/*If the indicator is long enough put the text inside on the right*/
		if(lv_area_get_width(dsc->draw_area) > txt_size.x + 20) {
			txt_area.x2 = dsc->draw_area->x2 - 5;
			txt_area.x1 = txt_area.x2 - txt_size.x + 1;
			label_dsc.color = lv_color_white();
		}
		/*If the indicator is still short put the text out of it on the right*/
		else {
			txt_area.x1 = dsc->draw_area->x2 + 5;
			txt_area.x2 = txt_area.x1 + txt_size.x - 1;
			label_dsc.color = lv_color_black();
		}

		txt_area.y1 = dsc->draw_area->y1 + (lv_area_get_height(dsc->draw_area) - txt_size.y) / 2;
		txt_area.y2 = txt_area.y1 + txt_size.y - 1;

		lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
	}

	Bar(lv_obj_t * parent, char const *title, unsigned min=0, unsigned max=100)
	{
		strncpy(_title, title, sizeof(_title)-1);

		/* set up a container */
		_cont = lv_obj_create(parent);
		lv_obj_align(_cont, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_size(_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
		lv_obj_set_style_bg_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_border_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_pad_all(_cont, 0, LV_PART_MAIN);

		static lv_style_t style_indic;

		lv_style_init(&style_indic);
		lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
		lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
		lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_GREEN));
		lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_HOR);

		_bar = lv_bar_create(_cont);
		lv_obj_add_style(_bar, &style_indic, LV_PART_INDICATOR);
		lv_obj_center(_bar);
		lv_bar_set_range(_bar, min, max);
		lv_obj_add_event_cb(_bar, _draw_part_event_cb, LV_EVENT_DRAW_PART_END, NULL);

		lv_obj_align(_bar, LV_ALIGN_BOTTOM_MID, 0, 0);

		/* set title shadow style */
		lv_style_init(&_style_shadow);
		lv_style_set_text_opa(&_style_shadow, LV_OPA_50);
		lv_style_set_text_color(&_style_shadow, lv_color_black());

		/* Create title shadow */
		_shadow = lv_label_create(_cont);
		lv_obj_add_style(_shadow, &_style_shadow, 0);

		/* set title style */
		lv_style_init(&_style);

		/* Create title label */
		_label = lv_label_create(_cont);
		lv_obj_add_style(_label, &_style, 0);
		lv_obj_align(_label, LV_ALIGN_TOP_MID, 0, 0);

		lv_label_set_text(_label,  _title);
		lv_label_set_text(_shadow, _title);

		handle_resize();
	}

	~Bar() override { lv_obj_del(_cont); }
};


struct Info::Calendar : Widget
{
	lv_obj_t * _cont { };
	lv_obj_t * _cal { };
	int        _last_day { 0 };

	lv_style_t _date_style   { };
	lv_style_t _date_style_shadow { };
	lv_obj_t * _date_label   { };
	lv_obj_t * _date_shadow  { };

	char       _tz[32]       { };

	/* Noncopyable */
	Calendar(Calendar const &) = delete;
	void operator= (Calendar const &) = delete;

	void handle_resize() override
	{
		unsigned font_size        { scaled_font(10) };
		const    lv_font_t * font { font_by_size(font_size) };
		lv_obj_set_size(_cal, column_size()-2*font_size, column_size()-2*font_size);
		lv_obj_set_style_text_font(_cal, font, LV_PART_MAIN);
		lv_obj_set_size(_cont, column_size(), column_size());

		lv_style_set_text_font(&_date_style, font);
		lv_style_set_text_font(&_date_style_shadow, font);
		lv_obj_align_to(_date_shadow, _date_label, LV_ALIGN_TOP_LEFT, font_size/5, font_size/5);
	}

	void tick(time_t epoch) override
	{
		setenv("TZ", _tz, 1);

		struct tm * tm = localtime(&epoch);

		if (_last_day == tm->tm_yday)
			return;

		lv_calendar_set_today_date(_cal, tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
		lv_calendar_set_showed_date(_cal, tm->tm_year+1900, tm->tm_mon+1);
		_last_day = tm->tm_yday;

		char time_str[32];
		strftime(time_str, sizeof(time_str), "%a, %d %b %Y", tm);
		lv_label_set_text(_date_label,  time_str);
		lv_label_set_text(_date_shadow, time_str);

		handle_resize();
	}

	Calendar(lv_obj_t * parent, char const *tz)
	{
		strncpy(_tz, tz, sizeof(_tz)-1);

		/* set up a container */
		_cont = lv_obj_create(parent);
		lv_obj_align(_cont, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_style_bg_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_border_opa(_cont, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_pad_all(_cont, 0, LV_PART_MAIN);

		_cal = lv_calendar_create(_cont);
		lv_obj_align(_cal, LV_ALIGN_BOTTOM_MID, 0, 0);

		/* set date shadow style */
		lv_style_init(&_date_style_shadow);
		lv_style_set_text_opa(&_date_style_shadow, LV_OPA_50);
		lv_style_set_text_color(&_date_style_shadow, lv_color_black());

		/* Create date shadow */
		_date_shadow = lv_label_create(_cont);
		lv_obj_add_style(_date_shadow, &_date_style_shadow, 0);

		/* set date style */
		lv_style_init(&_date_style);
		lv_style_set_pad_bottom(&_date_style, 20);

		/* Create date label */
		_date_label = lv_label_create(_cont);
		lv_obj_add_style(_date_label, &_date_style, 0);

		lv_obj_align(_date_label, LV_ALIGN_TOP_MID, 0, 0);

		handle_resize();
	}

	~Calendar() override { lv_obj_del(_cont); }
};


struct Info::Tabular : Widget
{
	lv_obj_t * _table;
	unsigned   _row_cnt { 0 };

	/* Noncopyable */
	Tabular(Tabular const &) = delete;
	void operator= (Tabular const &) = delete;

	Tabular(lv_obj_t * parent)
	: _table(lv_table_create(parent))
	{
		lv_obj_center(_table);

		handle_resize();

		lv_obj_add_event_cb(_table, _draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
	}

	~Tabular() override { lv_obj_del(_table); }

	bool empty() { return _row_cnt == 0; }

	void clear()
	{
		_row_cnt = 0;
		lv_table_set_row_cnt(_table, _row_cnt+1);
	}

	void handle_resize() override
	{
		const lv_font_t * font = font_by_size(scaled_font(10));

		lv_table_set_col_width(_table, 0, column_size()/2);
		lv_table_set_col_width(_table, 1, column_size()/2);

		lv_obj_set_style_text_font(_table, font, LV_PART_MAIN);
	}

	static void _draw_part_event_cb(lv_event_t * e)
	{
		lv_obj_t * obj = lv_event_get_target(e);
		lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);

		if(dsc->part == LV_PART_ITEMS) {
			uint32_t row = dsc->id /  lv_table_get_col_cnt(obj);
			uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

			/* center highlighted rows */
			if(lv_table_has_cell_ctrl(obj, row, 0, LV_TABLE_CELL_CTRL_CUSTOM_1)) {
				dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
				dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE), dsc->rect_dsc->bg_color, LV_OPA_30);
				dsc->rect_dsc->bg_opa = LV_OPA_COVER;
			}
			/* center merged columns */
			else if(lv_table_has_cell_ctrl(obj, row, col, LV_TABLE_CELL_CTRL_MERGE_RIGHT)) {
				dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
			}
			/* right align first column */
			else if(col == 0) {
				dsc->label_dsc->align = LV_TEXT_ALIGN_RIGHT;
			}

			/* make every 2nd row grayish*/
			if((row != 0 && row % 2) == 0) {
				dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), dsc->rect_dsc->bg_color, LV_OPA_10);
				dsc->rect_dsc->bg_opa = LV_OPA_COVER;
			}
		}
	}

	void add_row(const char * col1, const char * col2, bool highlight)
	{
		lv_table_clear_cell_ctrl(_table, _row_cnt, 0, LV_TABLE_CELL_CTRL_MERGE_RIGHT);

		lv_table_set_cell_value(_table, _row_cnt, 0, col1);
		lv_table_set_cell_value(_table, _row_cnt, 1, col2);

		if (highlight) {
			lv_table_add_cell_ctrl(_table, _row_cnt, 0, LV_TABLE_CELL_CTRL_CUSTOM_1);
			lv_table_add_cell_ctrl(_table, _row_cnt, 1, LV_TABLE_CELL_CTRL_CUSTOM_1);
		}

		_row_cnt++;
	}

	void add_merged_row(const char * txt)
	{
		lv_table_set_cell_value(_table, _row_cnt, 0, txt);
		lv_table_add_cell_ctrl(_table, _row_cnt, 0, LV_TABLE_CELL_CTRL_MERGE_RIGHT);
		_row_cnt++;
	}

};


struct Info::Layout
{
	lv_style_t _flex_style { };
	lv_obj_t * _flex_container { lv_obj_create(lv_scr_act()) };

	/* Noncopyable */
	Layout(Layout const &) = delete;
	void operator= (Layout const &) = delete;

	Layout()
	{
		/* create flex layout */
		lv_style_init(&_flex_style);
		lv_style_set_flex_flow(&_flex_style, LV_FLEX_FLOW_COLUMN_WRAP);
		lv_style_set_flex_main_place(&_flex_style, LV_FLEX_ALIGN_SPACE_EVENLY);
		lv_style_set_flex_cross_place(&_flex_style, LV_FLEX_ALIGN_CENTER);
		lv_style_set_layout(&_flex_style, LV_LAYOUT_FLEX);
		lv_style_set_bg_opa(&_flex_style, LV_OPA_0);
		lv_style_set_border_opa(&_flex_style, LV_OPA_0);
		lv_style_set_pad_column(&_flex_style, 2*scaled_font(12));

		lv_obj_set_size(_flex_container, lv_pct(100), lv_pct(100));
		lv_obj_center(_flex_container);
		lv_obj_add_style(_flex_container, &_flex_style, 0);
	}

	~Layout() { lv_obj_del(_flex_container); }

	template <typename FN>
	void with_container(FN && fn)
	{
		fn(_flex_container);
	}

	void handle_resize()
	{
		if (LV_HOR_RES < LV_VER_RES)
			lv_style_set_flex_track_place(&_flex_style, LV_FLEX_ALIGN_CENTER);
		else
			lv_style_set_flex_track_place(&_flex_style, LV_FLEX_ALIGN_START);
	}
};


struct Info::Track : Widget
{
	lv_obj_t * _flex_container { };

	/* Noncopyable */
	Track(Track const &) = delete;
	void operator= (Track const &) = delete;

	Track(lv_obj_t * cont, Alignment const & align)
	: _flex_container(lv_obj_create(cont))
	{
		lv_obj_align(_flex_container, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_size(_flex_container, LV_SIZE_CONTENT, lv_pct(100));
		lv_obj_set_flex_flow(_flex_container, LV_FLEX_FLOW_COLUMN_WRAP);
		switch (align) {
			case TOP:
				lv_obj_set_flex_align(_flex_container, LV_FLEX_ALIGN_START,
				                             LV_FLEX_ALIGN_CENTER,
				                             LV_FLEX_ALIGN_START);
				break;
			case MID:
				lv_obj_set_flex_align(_flex_container, LV_FLEX_ALIGN_CENTER,
				                             LV_FLEX_ALIGN_CENTER,
				                             LV_FLEX_ALIGN_START);
				break;
			case BOTTOM:
				lv_obj_set_flex_align(_flex_container, LV_FLEX_ALIGN_END,
				                             LV_FLEX_ALIGN_CENTER,
				                             LV_FLEX_ALIGN_START);
				break;
		}
		lv_obj_set_style_bg_opa(_flex_container, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_border_opa(_flex_container, LV_OPA_0, LV_PART_MAIN);
		lv_obj_set_style_pad_all(_flex_container, 0, LV_PART_MAIN);
		lv_obj_add_flag(_flex_container, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
	}

	~Track() { lv_obj_del(_flex_container); }

	template <typename FN>
	void with_container(FN && fn)
	{
		fn(_flex_container);
	}
};


void Info::set_background(lv_color_t color, unsigned opacity)
{
	lv_obj_set_style_bg_color(lv_scr_act(), color,   LV_PART_MAIN);
	lv_obj_set_style_bg_opa  (lv_scr_act(), opacity, LV_PART_MAIN);
}


#endif /* _SYSTEM_INFO_H_ */
