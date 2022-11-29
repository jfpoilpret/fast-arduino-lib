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
#include "../utilities.h"
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
	protected:
		static constexpr bool VERTICAL_FONT = DisplayDevice::VERTICAL_FONT;
		using SIGNED_COORDINATE = typename DisplayDevice::SIGNED_COORDINATE;
		using INVALID_AREA = InvalidArea<typename DisplayDevice::COORDINATE>;

	public:
		using COORDINATE = typename DisplayDevice::COORDINATE;

		static constexpr COORDINATE WIDTH = DisplayDevice::WIDTH;
		static constexpr COORDINATE HEIGHT = DisplayDevice::HEIGHT;
		//TODO does it need to be public?
		static constexpr uint8_t DEPTH = DisplayDevice::DEPTH;

		//TODO Define COLOR type (bool, uint8_t, uint16_t, uint32_t or specific uint24_t)
		//TODO API to handle colors

		Display() = default;

		// Display drawing settings: font, color, mode
		void set_font(const Font<VERTICAL_FONT>& font)
		{
			font_ = &font;
		}

		// Display drawing primitives
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

			// Check if specifc case (vertical or horizontal line)
			if (x1 == x2)
			{
				// if 2 points are the same: nothing to do
				if (y1 == y2) return;
				// Ensure y1 < y2
				swap_to_sort(y1, y2);
				draw_vline(x1, y1, y2);
			}
			else if (y1 == y2)
			{
				// Ensure x1 < x2
				swap_to_sort(x1, x2);
				draw_hline(x1, y1, x2);
			}
			else
			{
				// Possibly swap x1-x2 and y1-y2 to ensure x1 < x2
				if (swap_to_sort(x1, x2)) utils::swap(y1, y2);
				// Usual case, apply Bresenham's line algorithm
				draw_line_bresenham(x1, y1, x2, y2);
				// Ensure y1 < y2 for invalid region instantiation
				swap_to_sort(y1, y2);
			}
			invalidate(INVALID_AREA{x1, y1, x2, y2});
		}

		void draw_rectangle(COORDINATE x1, COORDINATE y1, COORDINATE x2, COORDINATE y2)
		{
			if (!is_valid_xy(x1, y1)) return;
			if (!is_valid_xy(x2, y2)) return;
			if ((x1 == x2) || (y1 == y2)) return;

			// Possibly swap x1-x2 and y1-y2
			swap_to_sort(x1, x2);
			swap_to_sort(y1, y2);
			// Simply draw 2 horizontal and 2 vertical lines
			draw_hline(x1, y1, x2);
			draw_hline(x1, y2, x2);
			draw_vline(x1, y1, y2);
			draw_vline(x2, y1, y2);
			invalidate(INVALID_AREA{x1, y1, x2, y2});
		}

		void draw_circle(COORDINATE xc, COORDINATE yc, COORDINATE radius)
		{
			if (!is_valid_xy(xc, yc)) return;
			if ((xc < radius) || (xc + radius > WIDTH)) return;
			if ((yc < radius) || (yc + radius > HEIGHT)) return;
			// Apply Bresenham's circle algorithm
			draw_circle_bresenham(xc, yc, radius);
			invalidate(INVALID_AREA{COORDINATE(xc - radius), COORDINATE(yc - radius), 
				COORDINATE(xc + radius), COORDINATE(yc + radius)});
		}

		//TODO additional 2D primitives here? e.g. arc, fill
		// void draw_arc()

		// Display update (raster copy to device)
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

		void draw_vline(COORDINATE x1, COORDINATE y1, COORDINATE y2)
		{
			swap_to_sort(y1, y2);
			for (COORDINATE y = y1; y <= y2; ++y)
				DisplayDevice::set_pixel(x1, y, true);
		}
		
		void draw_hline(COORDINATE x1, COORDINATE y1, COORDINATE x2)
		{
			swap_to_sort(x1, x2);
			for (COORDINATE x = x1; x <= x2; ++x)
				DisplayDevice::set_pixel(x, y1, true);
		}

		// Draw a segment according to Bresenham algorithm
		// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
		// https://fr.wikipedia.org/wiki/Algorithme_de_trac%C3%A9_de_segment_de_Bresenham
		void draw_line_bresenham(COORDINATE x1, COORDINATE y1, COORDINATE x2, COORDINATE y2)
		{
			// We are sure that x1 < x2 when calling this method
			SIGNED_COORDINATE dx = SIGNED_COORDINATE(x2) - SIGNED_COORDINATE(x1);
			SIGNED_COORDINATE dy = SIGNED_COORDINATE(y2) - SIGNED_COORDINATE(y1);

			//TODO Find a way to factor code to get reduced code size
			if (dy > 0)
			{
				// 1st quadrant
				if (dx >= dy)
				{
					// 1st octant
					SIGNED_COORDINATE e = dx;
					dx *= 2;
					dy *= 2;
					while (true)
					{
						DisplayDevice::set_pixel(x1, y1, true);
						if (x1 == x2) break;
						++x1;
						e -= dy;
						if (e < 0)
						{
							++y1;
							e += dx;
						}
					}
				}
				else
				{
					// 2nd octant
					SIGNED_COORDINATE e = dy;
					dx *= 2;
					dy *= 2;
					while (true)
					{
						DisplayDevice::set_pixel(x1, y1, true);
						if (y1 == y2) break;
						++y1;
						e -= dx;
						if (e < 0)
						{
							++x1;
							e += dy;
						}
					}
				}
			}
			else
			{
				// 4th quadrant
				if (dx >= -dy)
				{
					// 8th octant
					SIGNED_COORDINATE e = dx;
					dx *= 2;
					dy *= 2;
					while (true)
					{
						DisplayDevice::set_pixel(x1, y1, true);
						if (x1 == x2) break;
						++x1;
						e += dy;
						if (e < 0)
						{
							--y1;
							e += dx;
						}
					}
				}
				else
				{
					// 7th octant
					SIGNED_COORDINATE e = dy;
					dx *= 2;
					dy *= 2;
					while (true)
					{
						DisplayDevice::set_pixel(x1, y1, true);
						if (y1 == y2) break;
						--y1;
						e += dx;
						if (e > 0)
						{
							++x1;
							e += dy;
						}
					}
				}
			}
		}

		// https://fr.wikipedia.org/wiki/Algorithme_de_trac%C3%A9_d%27arc_de_cercle_de_Bresenham
		void draw_circle_bresenham(COORDINATE xc, COORDINATE yc, COORDINATE radius)
		{
			COORDINATE x = 0;
			COORDINATE y = radius;
			SIGNED_COORDINATE m = 5 - 4 * radius;
			while (x <= y)
			{
				DisplayDevice::set_pixel(x +  xc, y + yc, true);
				DisplayDevice::set_pixel(y +  xc, x + yc, true);
				DisplayDevice::set_pixel(-x +  xc, y + yc, true);
				DisplayDevice::set_pixel(-y +  xc, x + yc, true);
				DisplayDevice::set_pixel(x +  xc, -y + yc, true);
				DisplayDevice::set_pixel(y +  xc, -x + yc, true);
				DisplayDevice::set_pixel(-x +  xc, -y + yc, true);
				DisplayDevice::set_pixel(-y +  xc, -x + yc, true);
				if (m > 0)
				{
					--y;
					m -= 8 * y;
				}
				++x;
				m += 8 * x + 4;
			}
		}

		static bool swap_to_sort(COORDINATE& a1, COORDINATE& a2)
		{
			if (a1 > a2)
			{
				utils::swap(a1, a2);
				return true;
			}
			else
				return false;
		}
		
	private:
		// Minimal rectangle to update
		INVALID_AREA invalid_area_;
		const Font<VERTICAL_FONT>* font_ = nullptr;
	};
}

#endif /* DISPLAY_HH */
/// @endcond
