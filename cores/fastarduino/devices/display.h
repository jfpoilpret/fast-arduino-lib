//   Copyright 2016-2022 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/// @cond api

/**
 * @file
 * TODO
 */
#ifndef DISPLAY_HH
#define DISPLAY_HH

#include "../flash.h"
#include "font.h"

namespace devices
{
	//TODO APIDOC
	namespace display
	{
	}
}

//TODO error handling? (e.g. out of range coordinate)
namespace devices::display
{
	template<typename COORDINATE> struct InvalidArea
	{
		InvalidArea() = default;
		InvalidArea(COORDINATE x1, COORDINATE y1, COORDINATE x2, COORDINATE y2)
			: x1{x1}, y1{y1}, x2{x2}, y2{y2}, empty{false} {}

		InvalidArea& operator+=(const InvalidArea<COORDINATE>& a)
		{
			if (!a.empty)
			{
				if (empty)
				{
					x1 = a.x1;
					y1 = a.y1;
					x2 = a.x2;
					y2 = a.y2;
					empty = false;
				}
				else
				{
					if (a.x1 < x1) x1 = a.x1;
					if (a.y1 < y1) y1 = a.y1;
					if (a.x2 > x2) x2 = a.x2;
					if (a.y2 > y2) y2 = a.y2;
				}
			}
			return *this;
		}

		static InvalidArea EMPTY;

		COORDINATE x1 = 0;
		COORDINATE y1 = 0;
		COORDINATE x2 = 0;
		COORDINATE y2 = 0;
		bool empty = true;
	};

	template<typename COORDINATE> InvalidArea<COORDINATE> InvalidArea<COORDINATE>::EMPTY = InvalidArea{};

	template<typename COORDINATE>
	InvalidArea<COORDINATE> operator+(const InvalidArea<COORDINATE>& a1, const InvalidArea<COORDINATE>& a2)
	{
		if (a1.empty && a2.empty) return InvalidArea<COORDINATE>{};
		if (a1.empty) return a2;
		if (a2.empty) return a1;
		return InvalidArea<COORDINATE>{
			(a1.x1 < a2.x1 ? a1.x1 : a2.x1),
			(a1.y1 < a2.y1 ? a1.y1 : a2.y1),
			(a1.x2 > a2.x2 ? a1.x2 : a2.x2),
			(a1.y2 > a2.y2 ? a1.y2 : a2.y2)
		};
	}

	//TODO what are the expectations on DisplayDevice type?
	// - size constants?
	// - expected API?
	template<typename DisplayDevice> class Display: public DisplayDevice
	{
		static constexpr bool VERTICAL_FONT = DisplayDevice::VERTICAL_FONT;

	public:
		using COORDINATE = typename DisplayDevice::COORDINATE;
		using INVALID_AREA = InvalidArea<COORDINATE>;

		static constexpr COORDINATE WIDTH = DisplayDevice::WIDTH;
		static constexpr COORDINATE HEIGHT = DisplayDevice::HEIGHT;
		static constexpr COORDINATE DEPTH = DisplayDevice::DEPTH;

		Display() = default;

		void set_font(const Font<VERTICAL_FONT>& font)
		{
			font_ = &font;
		}

		void erase()
		{
			DisplayDevice::erase();
			invalidate();
		}

		void write_char(COORDINATE x, COORDINATE y, char value)
		{
			if (font_ == nullptr) return;
			if (!is_valid_xy(x, y)) return;
			const INVALID_AREA invalid = DisplayDevice::write_char(*font_, x, y, value);
			invalidate(invalid);
		}

		void write_string(COORDINATE x, COORDINATE y, const char* content)
		{
			if (font_ == nullptr) return;
			if (!is_valid_xy(x, y)) return;
			INVALID_AREA invalid = INVALID_AREA::EMPTY;
			while (*content)
			{
				invalid += DisplayDevice::write_char(*font_, x, y, *content++);
				x += font_->width() + 1;
			}
			// Invalidate if needed
			invalidate(invalid);
		}

		void write_string(COORDINATE x, COORDINATE y, const flash::FlashStorage* content)
		{
			if (font_ == nullptr) return;
			if (!is_valid_xy(x, y)) return;
			INVALID_AREA invalid = INVALID_AREA::EMPTY;
			uint16_t address = (uint16_t) content;
			while (char value = pgm_read_byte(address++))
			{
				invalid += DisplayDevice::write_char(*font_, x, y, value);
				x += font_->width() + 1;
			}
			// Invalidate if needed
			invalidate(invalid);
		}

		void set_pixel(COORDINATE x, COORDINATE y)
		{
			if (!is_valid_xy(x, y)) return;
			const INVALID_AREA invalid = DisplayDevice::set_pixel(x, y, true);
			// Invalidate if needed
			invalidate(invalid);
		}

		void clear_pixel(COORDINATE x, COORDINATE y)
		{
			if (!is_valid_xy(x, y)) return;
			const INVALID_AREA invalid = DisplayDevice::set_pixel(x, y, false);
			// Invalidate if needed
			invalidate(invalid);
		}

		void draw_line(COORDINATE x1, COORDINATE y1, COORDINATE x2, COORDINATE y2)
		{
			if (!is_valid_xy(x1, y1)) return;
			if (!is_valid_xy(x2, y2)) return;
			//TODO use https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
			// Specific lines: horizontal or vertical (straightforward)
		}

		void draw_rectangle(COORDINATE x1, COORDINATE y1, COORDINATE x2, COORDINATE y2)
		{
			if (!is_valid_xy(x1, y1)) return;
			if (!is_valid_xy(x2, y2)) return;
			//TODO simply draw 2 hoizontal and 2 vertical lines
		}

		void draw_circle(COORDINATE x, COORDINATE y, COORDINATE ray)
		{
			if (!is_valid_xy(x, y)) return;
			//TODO
		}

		void update()
		{
			DisplayDevice::update(invalid_area_);
			invalid_area_.empty = true;
		}

		void force_update()
		{
			invalidate();
			update();
		}

	protected:
		void invalidate(const INVALID_AREA& area)
		{
			invalid_area_ += area;
		}

		void invalidate()
		{
			invalid_area_ = INVALID_AREA{0, 0, WIDTH - 1, HEIGHT - 1};
		}

		static bool is_valid_xy(COORDINATE x, COORDINATE y)
		{
			return (x < WIDTH) && (y < HEIGHT);
		}

	private:
		// Minimal rectangle to update
		INVALID_AREA invalid_area_;
		const Font<VERTICAL_FONT>* font_ = nullptr;
	};
}

#endif /* DISPLAY_HH */
/// @endcond
