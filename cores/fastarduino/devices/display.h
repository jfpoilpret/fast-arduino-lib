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
 * Generic API to handle any display device (e.g. Nokia5110, ILI9340 TFT...)
 */
#ifndef DISPLAY_HH
#define DISPLAY_HH

#include "../flash.h"
#include "../types_traits.h"
#include "../utilities.h"
#include "font.h"

namespace devices
{
	/**
	 * Defines generic API for all display devices.
	 * Any actual device class must:
	 * - derive from `AbstractDisplayDevice` template class
	 * - define XCOORD, YCOORD, SCALAR, SIGNED_SCALAR integral types 
	 * - define INVALID_AREA type as an `InvalidArea` template instantiation
	 * - define WIDTH and HEIGHT constants
	 * - define a `protected` default constructor
	 * - implement 3 `protected` drawing primitives
	 * 		- `erase()`
	 * 		- `set_pixel(x, y)`
	 * 		- `write_char(x, y, value)`
	 * - implement `protected` `update(area)` method
	 * - implement any device-specific `public` API
	 * 
	 * A display device class becomes actually usable be defining a type as the
	 * `Display` template class instantiation for the display device class itself.
	 * The type such defined:
	 * - inherits the display device class and thus provides all its specific `public` API
	 * - provides additional 2D drawing primitives as `public` API
	 * 
	 * You can see a concrete example for `LCD5110` device:
	 * TODO show example of Nokia classes inheritance picture
	 * 
	 * Display devices are:
	 * - either based on a SRAM raster buffer they have to handle themselves (this
	 * is applicable only to small resolution devices such as Nokia5110, as SRAM 
	 * is a scarce resource on AVR MCU); this device needs to provide an implementation
	 * of `update()` method that sends invalidated raster pixels to the device.
	 * - or they directly draw evey pixel directly to the device because they cannot
	 * possibly hold a raster buffer in SRAM (too large resolution, like ILI9340);
	 * thsi device must implement an empty `update()` method.
	 */
	namespace display
	{
	}
}

//TODO error handling? (e.g. out of range coordinate)
// - enum class Error : uint8_t {...}
// - Error last_error_ field in Display
// - Error last_error() const
// - reset_error()
namespace devices::display
{
	/**
	 * Mode used when drawing pixels.
	 * This determines how the destination pixel color is affected by the
	 * source color.
	 */
	enum class Mode : uint8_t
	{
		/** Source color simply replaces destination pixel. */
		COPY = 0,
		/** Destination pixel is xor'ed with source color (inversion mode). */
		XOR,
		/** Destination pixel is and'ed with source color (clear mode). */
		AND,
		/** Destination pixel is or'ed with source color (set mode). */
		OR
	};

	/**
	 * Class to be used by a display device class that can handle specific drawing 
	 * modes.
	 * 
	 * @note Not all display devices can support all modes, as most modes (except 
	 * `Mode::COPY`) require access to a raster buffer (either in SRAM or on the 
	 * display chip itself).
	 * 
	 * For each supported mode, 2 methods are defined:
	 * - `xxxx_bw_pixels()` can be used for B&W devices where one byte represents 
	 * 8 pixels; the drawing mode will be applied to all pixels in the byte
	 * - `xxxx_pixel()` is used on ONE pixel color (type defined by @p COLOR).
	 * 
	 * `LCD5110` class shows how this class can be used.
	 * 
	 * @tparam COLOR the type of one pixel color, as handled by the display device.
	 * 
	 * @sa Mode
	 * @sa LCD5110
	 */
	template<typename COLOR> struct PixelCalculator
	{
		/** The type of a pointer function for `xxxx_bw_pixels()`. */
		using COMPUTE_BW_PIXELS = uint8_t (*)(uint8_t, uint8_t);
		/** The type of a pointer function for `xxxx_pixel()`. */
		using COMPUTE_PIXEL = COLOR (*)(COLOR, COLOR);

		/// @cond notdocumented
		struct Calculators
		{
			COMPUTE_BW_PIXELS bw_pixels_op;
			COMPUTE_PIXEL pixel_op;
		};
		/// @endcond

		/**
		 * Return the proper drawing operations for the given drawing @p mode.
		 * 
		 * @param mode teh drawing mdoe for which you want the proper drawing
		 * operations
		 * @return Calculators a set of 2 drawing operation functions
		 */
		static Calculators get_calculators(Mode mode)
		{
			switch (mode)
			{
				case Mode::COPY:
				default:
				return {copy_bw_pixels, copy_pixel};

				case Mode::XOR:
				return {xor_bw_pixels, xor_pixel};

				case Mode::AND:
				return {and_bw_pixels, and_pixel};

				case Mode::OR:
				return {or_bw_pixels, or_pixel};
			}
		}

		/// @cond notdocumented
		static uint8_t copy_bw_pixels(uint8_t source, UNUSED uint8_t dest)
		{
			return source;
		}
		static COLOR copy_pixel(COLOR source, UNUSED COLOR dest)
		{
			return source;
		}

		static uint8_t xor_bw_pixels(uint8_t source, uint8_t dest)
		{
			return source ^ dest;
		}
		static COLOR xor_pixel(COLOR source, COLOR dest)
		{
			return source ^ dest;
		}

		static uint8_t and_bw_pixels(uint8_t source, uint8_t dest)
		{
			return source & dest;
		}
		static COLOR and_pixel(COLOR source, COLOR dest)
		{
			return source & dest;
		}

		static uint8_t or_bw_pixels(uint8_t source, uint8_t dest)
		{
			return source | dest;
		}
		static COLOR or_pixel(COLOR source, COLOR dest)
		{
			return source | dest;
		}
		/// @endcond
	};

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

	/**
	 * Types of errors that can occur on `Display` instances.
	 * @sa AbstractDisplayDevice.last_error()
	 */
	enum class Error : uint8_t
	{
		//TODO simplify list (redundant codes)
		/** No error occurred. */
		NO_ERROR = 0,
		/** A text drawing primitive has been called but no font has been set yet. */
		NO_FONT_SET,
		/** A text drawing primitive has been called with a character value which has no glyph in current font. */
		NO_GLYPH_FOUND,
		/** A drawing primitive has been called with (x,y) coordinates out of display range. */
		COORDS_OUT_OF_RANGE,
		/** 
		 * A drawing primitive has been called with invalid (x,y) coordinates; this
		 * is different to `COORDS_OUT_OF_RANGE` in the sens that actual display devices 
		 * may impose specific constraint on x or y coordinates (e.g. being a multiple of 8).
		 */
		COORDS_INVALID,
		/**
		 * A drawing primitive with valid (x,y) coordinates would draw its shape
		 * outside the display estate, which is forbidden. 
		 * This may be due to extra arguments (e.g. circle radius).
		 */
		SHAPE_OUT_OF_DISPLAY,
		/** A drawing primitive (line) was called with coordinates of 2 identical points. */
		POINTS_NOT_SEPARATE,
		/** Trying to draw a rectangle that would be only a simple line. */
		FLAT_RECTANGLE
	};

	/**
	 * Abstract base class for all display device classes.
	 * This provides a few useful API that are needed by subclasses.
	 * 
	 * @tparam COLOR the type of one pixel color, as handled by the display device.
	 * @tparam VERTICAL_FONT `true` if the device needs fonts where every byte maps
	 * to a column of 8 pixels, rather than a row of 8 pixels. For instance,
	 * `LCD5110` and its associated PCD8544 chip have such a vertical organization.
	 */
	template<typename COLOR, bool VERTICAL_FONT> class AbstractDisplayDevice : public AbstractDisplayDeviceGhost
	{
	public:
		/**
		 * Set color to use for next calls to drawing primitives.
		 * The new color is simply available to subclasses in the `color_` field.
		 * 
		 * @param color the new color to use from now
		 * @sa color_
		 */
		void set_color(COLOR color)
		{
			color_ = color;
		}

		/**
		 * Set the new font to use for next calls to text drawing perimitives. 
		 * A pointer to the new font is simply available to subclasses in the 
		 * `font_` field.
		 * 
		 * @param font the new font to use from now
		 * @sa Font
		 * @sa font_
		 */
		void set_font(const Font<VERTICAL_FONT>& font)
		{
			font_ = &font;
		}

		/**
		 * Error code of latest called drawing primitive.
		 * Automatically erased (set to `Error::NO_ERROR`) by a successful call to
		 * a drawing primitive.
		 * If the latest drawing primitive set an error, this means nothing was 
		 * drawn at all.
		 * 
		 * @return Error `Error::NO_ERROR` if last drawing primitive ran successfully.
		 */
		Error last_error() const
		{
			return last_error_;
		}

	protected:
		bool check_font()
		{
			if (font_ != nullptr) return true;
			last_error_ = Error::NO_FONT_SET;
			return false;
		}

		/** 
		 * Pointer to the current `Font` that shall be used by text drawing primitives.
		 * @warning This can be `nullptr`!
		 */
		const Font<VERTICAL_FONT>* font_ = nullptr;

		/** Current color that shall be used by drawing primitives. */
		COLOR color_{};

		/** 
		 * The result status of the last drawing primitive called.
		 * @sa last_error()
		 */
		Error last_error_ = Error::NO_ERROR;
	};

	/**
	 * TODO DOCUMENT
	 * 
	 * InvalidArea can be summed together (union).
	 * 
	 * @tparam XCOORD the integral type for X coordinates
	 * @tparam YCOORD the integral type for Y coordinates
	 * @tparam USED `true` (default) if the display device is based on raster 
	 * updates and thus needs invalid areas support, `false` otherwise, providing
	 * an empty InvalidArea object, which will allow same API whatever the situation,
	 * whilst increasing performance (time and size) when invalid areas are not used.
	 */
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

	//TODO check that it works and it generates no code!
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

	// Screen update is not always automatic! You must call update() once you have changed display bitmap content
	template<typename DISPLAY_DEVICE> class Display : public DISPLAY_DEVICE
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
			if (!DISPLAY_DEVICE::check_font()) return;
			if (!is_valid_xy(x, y)) return;
			const INVALID_AREA invalid = DISPLAY_DEVICE::write_char(x, y, value);
			invalidate(invalid);
		}

		void write_string(XCOORD x, YCOORD y, const char* content)
		{
			if (!DISPLAY_DEVICE::check_font()) return;
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
			if (!DISPLAY_DEVICE::check_font()) return;
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

		void draw_line(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2)
		{
			if (!is_valid_xy(x1, y1)) return;
			if (!is_valid_xy(x2, y2)) return;

			// Check if specifc case (vertical or horizontal line)
			if (x1 == x2)
			{
				// if 2 points are the same: nothing to do
				if (y1 == y2)
				{
					DISPLAY_DEVICE::last_error_ = Error::POINTS_NOT_SEPARATE;
					return;
				}
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

		void draw_rectangle(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2)
		{
			if (!is_valid_xy(x1, y1)) return;
			if (!is_valid_xy(x2, y2)) return;
			if ((x1 == x2) || (y1 == y2))
			{
				DISPLAY_DEVICE::last_error_ = Error::FLAT_RECTANGLE;
				return;
			}
			// Possibly swap x1-x2 and y1-y2
			swap_to_sort(x1, x2);
			swap_to_sort(y1, y2);
			// Simply draw 2 horizontal and 2 vertical lines
			draw_hline(x1, y1, x2);
			draw_hline(x1, y2, x2);
			// Note that we avoid drawing the same pixels (corners) twice 
			// (due to a drawing mode that might potentially be XOR)
			draw_vline(x1, y1 + 1, y2 - 1);
			draw_vline(x2, y1 + 1, y2 - 1);
			invalidate(INVALID_AREA{x1, y1, x2, y2});
		}

		void draw_circle(XCOORD xc, YCOORD yc, SCALAR radius)
		{
			if (!is_valid_xy(xc, yc)) return;
			//FIXME also check radius != 0 (radius == 0 crashes MCU!)
			if (	(xc < radius) || (xc + radius > WIDTH)
				||	(yc < radius) || (yc + radius > HEIGHT))
			{
				DISPLAY_DEVICE::last_error_ = Error::SHAPE_OUT_OF_DISPLAY;
				return;
			}
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
			DISPLAY_DEVICE::last_error_ = Error::NO_ERROR;
		}

		void invalidate()
		{
			invalid_area_ = INVALID_AREA{0, 0, WIDTH - 1, HEIGHT - 1};
			DISPLAY_DEVICE::last_error_ = Error::NO_ERROR;
		}

		bool is_valid_xy(XCOORD x, YCOORD y)
		{
			if ((x < WIDTH) && (y < HEIGHT)) return true;
			DISPLAY_DEVICE::last_error_ = Error::COORDS_OUT_OF_RANGE;
			return false;
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
		INVALID_AREA invalid_area_ = INVALID_AREA::EMPTY;
	};
}

#endif /* DISPLAY_HH */
/// @endcond
