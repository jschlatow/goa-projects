/*
 * \brief  Screenshot component using capture session and libpng
 * \author Johannes Schlatow
 * \date   2021-06-04
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode include */
#include <base/env.h>
#include <libc/component.h>
#include <base/log.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <capture_session/connection.h>
#include <rtc_session/connection.h>

/* libpng includes */
#include <png.h>

namespace Screenshot {

	using namespace Genode;

	struct Main;
}

struct Screenshot::Main
{
	Libc::Env &_env;

	using Pixel          = Capture::Pixel;
	using Area           = Capture::Area;
	using Affected_rects = Capture::Session::Affected_rects;

	Attached_rom_dataspace _rom   { _env, "trigger" };
	Heap                   _heap  { _env.ram(), _env.rm() };

	bool                   _last_state { false };

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

	Rtc::Connection      _rtc        { _env };
	Capture::Connection  _capture    { _env };

	struct Png_output
	{
		public:
			typedef String<64> Filename;

		private:
			png_byte  *_image        { nullptr };
			png_bytep *_row_pointers { nullptr };
			Allocator &_alloc;
			Area       _area;

			/*
			 * Noncopyable
			 */
			Png_output(Png_output const &);
			Png_output &operator= (Png_output const &);

			template <typename T>
			T *_addr()
			{
				return static_cast<T *>((void*)_image);
			}

		public:

			Png_output(Allocator &alloc, Area area)
			: _alloc(alloc), _area(area)
			{
				_image        = new (_alloc) png_byte[area.w * area.h * sizeof(Pixel)];
				_row_pointers = new (_alloc) png_bytep[area.h];

				if (_image == nullptr || _row_pointers == nullptr)
					throw Out_of_ram();

				/* Set up pointers into your "image" byte array. */
				for (size_t k = 0; k < area.h; k++)
					_row_pointers[k] = _image + k * area.w * sizeof(Pixel);
			}

			~Png_output()
			{
				destroy(_alloc, _row_pointers);
				destroy(_alloc, _image);
			}

			void write_png(Filename const &name)
			{
				/* open file */
				FILE *fp = fopen(name.string(), "wb");
				if (fp == nullptr) {
					error("failed to open file ", name);
					return;
				}

				/* alloc png write struct */
				png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				                                              nullptr,
				                                              nullptr,
				                                              nullptr);
				if (png_ptr == nullptr)
				{
					error("Allocation of png write struct failed.");
					fclose(fp);
					return;
				}

				/* alloc png info struct */
				png_infop info_ptr = png_create_info_struct(png_ptr);
				if (info_ptr == nullptr)
				{
					error("Allocation of png info struct failed.");
					fclose(fp);
					png_destroy_write_struct(&png_ptr, nullptr);
					return;
				}

				/* libpng's default error handling via longjump magic */
				if (setjmp(png_jmpbuf(png_ptr)))
				{
					error("Error writing png file.");
					fclose(fp);
					png_destroy_write_struct(&png_ptr, &info_ptr);
					return;
				}

				png_init_io(png_ptr, fp);

				/* setup png header */
				png_set_IHDR(png_ptr, info_ptr, _area.w, _area.h, 8,
				             PNG_COLOR_TYPE_RGB, PNG_INTERLACE_ADAM7,
				             PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

				/* write file header */
				png_write_info(png_ptr, info_ptr);

				/* remove unused alpha channel in Pixel_rgb888 */
				png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

				/* Flip BGR pixels to RGB. */
				png_set_bgr(png_ptr);

				/* write image */
				png_write_image(png_ptr, _row_pointers);
				png_write_end(png_ptr, info_ptr);

				/* cleanup */
				png_destroy_write_struct(&png_ptr, &info_ptr);
				fclose(fp);
			}

			template <typename FN>
			void with_surface(FN const &fn)
			{
				Surface<Pixel> surface(_addr<Pixel>(), _area);

				fn(surface);
			}
	};

	using Screen = Capture::Connection::Screen;

	Constructible<Png_output> _output { };
	Constructible<Screen>     _screen { };

	void _handle_update()
	{
		_rom.update();
		if (!_rom.valid()) return;

		Xml_node const &xml = _rom.xml();

		if (xml.attribute_value("enabled", _last_state) == _last_state) return;

		_last_state = !_last_state;

		Area area = _area_from_xml(xml, _capture.screen_size());

		if (!area.valid()) {
			error("Invalid screen size");
			return;
		}

		_screen.construct(_capture, _env.rm(), Screen::Attr { .px = area,
		                                                      .mm = { },
		                                                      .viewport = area,
		                                                      .rotate = { },
		                                                      .flip = { } });
		_output.construct(_heap, area);

		/* copy image data from capture session to image buffer */
		_screen->with_texture([&] (Texture<Pixel> const &texture) {

			_output->with_surface([&] (Surface<Pixel> &surface) {
				Affected_rects const affected = _capture.capture_at(_point_from_xml(xml));

				affected.for_each_rect([&] (Capture::Rect const rect) {

					surface.clip(rect);

					Blit_painter::paint(surface, texture, Capture::Point(0, 0));
				});
			});
		});

		/* use tag name from XML als filename base and
		 * append current date/time
		 */
		Png_output::Filename name(xml.type(),"_" ,_rtc.current_time(), ".png");

		log("Writing screenshot to ", name);
		Libc::with_libc([&] () {_output->write_png(name);});
	}

	Signal_handler<Main> _rom_handler { _env.ep(), *this, &Main::_handle_update };

	Main(Libc::Env &env) : _env(env)
	{
		_rom.sigh(_rom_handler);
	}
};


void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] () {
		static Screenshot::Main main(env);
	});
}
