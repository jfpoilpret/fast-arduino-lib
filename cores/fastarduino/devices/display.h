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
#include "../types_traits.h"
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
	/// @cond notdocumented
	// This class is here to simplify check, in Display ctor, that DISPLAY_DEVICE is a subclass
	// of AbstractDisplayDevice
	class AbstractDisplayDeviceGhost
	{
	private:
		AbstractDisplayDeviceGhost() = default;
		template<typename, bool>friend class AbstractDisplayDevice;
	};
	/// @endcond

	// All display device implementation should derive from this class!
	template<typename COLOR, bool VERTICAL_FONT> class AbstractDisplayDevice : public AbstractDisplayDeviceGhost
	{
	public:
		//TODO also include invalid area here (as type and as variable)?
		void set_color(COLOR color)
		{
			color_ = color;
		}

		// Display drawing settings: font, color, mode
		void set_font(const Font<VERTICAL_FONT>& font)
		{
			font_ = &font;
		}

	protected:
		const Font<VERTICAL_FONT>* font_ = nullptr;
		COLOR color_{};
	};

	template<typename XCOORD, typename YCOORD, bool USED = true> struct InvalidArea
	{
		InvalidArea() = default;
		InvalidArea(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2)
			: x1{x1}, y1{y1}, x2{x2}, y2{y2}, empty{false} {}

		InvalidArea& operator+=(const InvalidArea<XCOORD, YCOORD>& a)
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

		XCOORD x1 = 0;
		YCOORD y1 = 0;
		XCOORD x2 = 0;
		YCOORD y2 = 0;
		bool empty = true;
	};

	// Use for big displays (without raster), InvalidArea is reduced to `const bool empty = true;`
	// so that we do not waste space and time with extra variables and calculations 
	//TODO check that it works!
	//TODO check that it generates no code!
	/// @cond notdocumented
	template<typename XCOORD, typename YCOORD> struct InvalidArea<XCOORD, YCOORD, false>
	{
		InvalidArea() = default;
		InvalidArea(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2) {}

		InvalidArea& operator+=(const InvalidArea<XCOORD, YCOORD, false>& a)
		{
			return *this;
		}

		static InvalidArea EMPTY;

		const bool empty = true;
	};

	template<typename XCOORD, typename YCOORD, bool USED>
	InvalidArea<XCOORD, YCOORD, USED> InvalidArea<XCOORD, YCOORD, USED>::EMPTY = InvalidArea{};

	template<typename XCOORD, typename YCOORD, bool USED>
	InvalidArea<XCOORD, YCOORD, USED> operator+(
		const InvalidArea<XCOORD, YCOORD, USED>& a1, const InvalidArea<XCOORD, YCOORD, USED>& a2)
	{
		if (a1.empty && a2.empty) return InvalidArea<XCOORD, YCOORD, USED>{};
		if (a1.empty) return a2;
		if (a2.empty) return a1;
		return InvalidArea<XCOORD, YCOORD, USED>{
			(a1.x1 < a2.x1 ? a1.x1 : a2.x1),
			(a1.y1 < a2.y1 ? a1.y1 : a2.y1),
			(a1.x2 > a2.x2 ? a1.x2 : a2.x2),
			(a1.y2 > a2.y2 ? a1.y2 : a2.y2)
		};
	}
	/// @endcond

	//TODO document constraints on DISPLAY_DEVICE
	// Screen update is not always automatic! You must call update() once you have changed display bitmap content
	template<typename DISPLAY_DEVICE> class Display: public DISPLAY_DEVICE
	{
	public:
		using XCOORD = typename DISPLAY_DEVICE::XCOORD;
		using YCOORD = typename DISPLAY_DEVICE::YCOORD;
		using SCALAR = typename DISPLAY_DEVICE::SCALAR;

		static constexpr XCOORD WIDTH = DISPLAY_DEVICE::WIDTH;
		static constexpr YCOORD HEIGHT = DISPLAY_DEVICE::HEIGHT;

	protected:
		using SIGNED_SCALAR = typename DISPLAY_DEVICE::SIGNED_SCALAR;
		using INVALID_AREA = typename DISPLAY_DEVICE::INVALID_AREA;

	public:
		Display()
		{
			// Check at compile-time that DISPLAY_DEVICE is a subclass of AbstractDisplayDevice
			types_traits::derives_from<DISPLAY_DEVICE, AbstractDisplayDeviceGhost>{};
		}

		// Display drawing primitives
		void erase()
		{
			DISPLAY_DEVICE::erase();
			invalidate();
		}

		void write_char(XCOORD x, YCOORD y, char value)
		{
			if (this->font_ == nullptr) return;
			if (!is_valid_xy(x, y)) return;
			const INVALID_AREA invalid = DISPLAY_DEVICE::write_char(x, y, value);
			invalidate(invalid);
		}

		void write_string(XCOORD x, YCOORD y, const char* content)
		{
			if (this->font_ == nullptr) return;
			if (!is_valid_xy(x, y)) return;
			INVALID_AREA invalid = INVALID_AREA::EMPTY;
			while (*content)
			{
				invalid += DISPLAY_DEVICE::write_char(x, y, *content++);
				x += this->font_->width() + 1;
			}
			// Invalidate if needed
			invalidate(invalid);
		}

		void write_string(XCOORD x, YCOORD y, const flash::FlashStorage* content)
		{
			if (this->font_ == nullptr) return;
			if (!is_valid_xy(x, y)) return;
			INVALID_AREA invalid = INVALID_AREA::EMPTY;
			uint16_t address = (uint16_t) content;
			while (char value = pgm_read_byte(address++))
			{
				invalid += DISPLAY_DEVICE::write_char(x, y, value);
				x += this->font_->width() + 1;
			}
			// Invalidate if needed
			invalidate(invalid);
		}

		void draw_pixel(XCOORD x, YCOORD y)
		{
			if (!is_valid_xy(x, y)) return;
			const INVALID_AREA invalid = DISPLAY_DEVICE::set_pixel(x, y);
			// Invalidate if needed
			invalidate(invalid);
		}

		//TODO rework if (x2, y2) should be included in the line or excluded
		void draw_line(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2)
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

		//TODO rework if (x2, y2) should be included in the rectangle or excluded
		void draw_rectangle(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2)
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

		void draw_circle(XCOORD xc, YCOORD yc, SCALAR radius)
		{
			if (!is_valid_xy(xc, yc)) return;
			if ((xc < radius) || (xc + radius > WIDTH)) return;
			if ((yc < radius) || (yc + radius > HEIGHT)) return;
			// Apply Bresenham's circle algorithm
			draw_circle_bresenham(xc, yc, radius);
			invalidate(INVALID_AREA{XCOORD(xc - radius), YCOORD(yc - radius), 
				XCOORD(xc + radius), YCOORD(yc + radius)});
		}

		//TODO additional 2D primitives here? e.g. arc, fill
		// void draw_arc()

		// Display update (raster copy to device)
		void update()
		{
			DISPLAY_DEVICE::update(invalid_area_);
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

		static bool is_valid_xy(XCOORD x, YCOORD y)
		{
			return (x < WIDTH) && (y < HEIGHT);
		}

		void draw_vline(XCOORD x1, YCOORD y1, YCOORD y2)
		{
			swap_to_sort(y1, y2);
			for (YCOORD y = y1; y <= y2; ++y)
				DISPLAY_DEVICE::set_pixel(x1, y);
		}
		
		void draw_hline(XCOORD x1, YCOORD y1, XCOORD x2)
		{
			swap_to_sort(x1, x2);
			for (XCOORD x = x1; x <= x2; ++x)
				DISPLAY_DEVICE::set_pixel(x, y1);
		}

		// Draw a segment according to Bresenham algorithm
		// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
		// https://fr.wikipedia.org/wiki/Algorithme_de_trac%C3%A9_de_segment_de_Bresenham
		void draw_line_bresenham(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2)
		{
			// We are sure that x1 < x2 when calling this method
			SIGNED_SCALAR dx = SIGNED_SCALAR(x2) - SIGNED_SCALAR(x1);
			SIGNED_SCALAR dy = SIGNED_SCALAR(y2) - SIGNED_SCALAR(y1);

			//TODO Find a way to factor code to get reduced code size
			if (dy > 0)
			{
				// 1st quadrant
				if (dx >= dy)
				{
					// 1st octant
					SIGNED_SCALAR e = dx;
					dx *= 2;
					dy *= 2;
					while (true)
					{
						DISPLAY_DEVICE::set_pixel(x1, y1);
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
					SIGNED_SCALAR e = dy;
					dx *= 2;
					dy *= 2;
					while (true)
					{
						DISPLAY_DEVICE::set_pixel(x1, y1);
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
					SIGNED_SCALAR e = dx;
					dx *= 2;
					dy *= 2;
					while (true)
					{
						DISPLAY_DEVICE::set_pixel(x1, y1);
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
					SIGNED_SCALAR e = dy;
					dx *= 2;
					dy *= 2;
					while (true)
					{
						DISPLAY_DEVICE::set_pixel(x1, y1);
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
		void draw_circle_bresenham(XCOORD xc, YCOORD yc, SCALAR radius)
		{
			XCOORD x = 0;
			YCOORD y = radius;
			SIGNED_SCALAR m = 5 - 4 * radius;
			while (x <= y)
			{
				DISPLAY_DEVICE::set_pixel(x +  xc, y + yc);
				DISPLAY_DEVICE::set_pixel(y +  xc, x + yc);
				DISPLAY_DEVICE::set_pixel(-x +  xc, y + yc);
				DISPLAY_DEVICE::set_pixel(-y +  xc, x + yc);
				DISPLAY_DEVICE::set_pixel(x +  xc, -y + yc);
				DISPLAY_DEVICE::set_pixel(y +  xc, -x + yc);
				DISPLAY_DEVICE::set_pixel(-x +  xc, -y + yc);
				DISPLAY_DEVICE::set_pixel(-y +  xc, -x + yc);
				if (m > 0)
				{
					--y;
					m -= 8 * y;
				}
				++x;
				m += 8 * x + 4;
			}
		}

		template<typename COORD>
		static bool swap_to_sort(COORD& a1, COORD& a2)
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
	};
}

#endif /* DISPLAY_HH */
/// @endcond
