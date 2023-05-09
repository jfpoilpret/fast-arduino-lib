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
#include "../initializer_list.h"
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
	 * - define HAS_RASTER bool constant indicating if the device uses a raster buffer
	 * - define WIDTH and HEIGHT constants
	 * - define a `protected` default constructor
	 * - implement 4 `protected` drawing primitives
	 * 		- `erase()`
	 * 		- `set_pixel(x, y)`
	 * 		- `is_valid_char_xy(x, y)`
	 * 		- `write_char(x, y, glyph_ref)`
	 * - implement `protected` `update(x1, y1, x2, y2)` method
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
	 * this device must implement an empty `update()` method.
	 */
	namespace display
	{
	}
}

namespace devices::display
{
	/// @cond notdocumented
	template<typename XCOORD, typename YCOORD>
	struct Point
	{
		Point(XCOORD x, YCOORD y) : x{x}, y{y} {}
		XCOORD x;
		YCOORD y;
	};
	/// @endcond

	//TODO use device traits to determine what modes a device can handle!
	/**
	 * Mode used when drawing pixels.
	 * This determines how the destination pixel color is affected by the
	 * source color.
	 * 
	 * @note Not all display devices can support all modes, as most modes (except 
	 * `Mode::COPY`) require access to a raster buffer (either in SRAM or on the 
	 * display chip itself).
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
		OR,
		/** In this mode, the destination pixel never changes, whatever the source color. */
		NO_CHANGE = 0xFF
	};

	//TODO Add pattern
	/**
	 * Drawing Mode to use for `Display` drawing primitives.
	 * This encapsulates a pixel operation `Mode` and a color to use.
	 * 
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
	template<typename COLOR> class DrawMode
	{
	public:
		DrawMode(Mode mode = Mode::NO_CHANGE, COLOR color = COLOR{})
			: mode_{mode}, color_{color} {}

		/**
		 * Test if this `DrawMode` can change display or not, ie if its mode
		 * is not `Mode::NO_CHANGE`.
		 * This is useful in order to avoid complex primitive functions that will
		 * waste CPU with no effect.
		 */
		operator bool() const
		{
			return mode_ != Mode::NO_CHANGE;
		}

		/**
		 * Combine 8 source B&W pixels and 8 destination B&W pixels, all gathered
		 * in a byte, according to the `Mode` set at construction time.
		 * 
		 * @param source the 8 B&W pixels we want to apply onto @p dest
		 * @param dest the current 8 B&W pixels present on display, with which 
		 * @p source pixels shall be combined according to `Mode`
		 * @return uint8_t the resulting 8 B&W pixels to display
		 */
		uint8_t bw_pixels_op(uint8_t source, uint8_t dest) const
		{
			// Invert source if colow is black
			if (!color_)
				source = ~source;
			switch (mode_)
			{
				case Mode::COPY:
				default:
				return source;

				case Mode::XOR:
				return source ^ dest;

				case Mode::AND:
				return source & dest;

				case Mode::OR:
				return source | dest;
			}
		}

		/**
		 * Combine the predefined color (defined at construction time) with one
		 * destination pixel, according to the `Mode` also set at construction time.
		 * 
		 * @param dest the current pixel color present on display, with which 
		 * color shall be combined according to `Mode`
		 * @return COLOR the resulting color to display
		 */
		COLOR pixel_op(COLOR dest) const
		{
			switch (mode_)
			{
				case Mode::COPY:
				default:
				return color_;

				case Mode::XOR:
				return color_ ^ dest;

				case Mode::AND:
				return color_ & dest;

				case Mode::OR:
				return color_ | dest;
			}
		}

	private:
		Mode mode_;
		COLOR color_;
	};

	//TODO redesign to avoid this complex hierarchy
	//TODO only use Display template on actual device, and pass info (font, draw mode...) to drawing methods
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
		/** No error occurred. */
		NO_ERROR = 0,
		/** A text drawing primitive has been called but no font has been set yet. */
		NO_FONT_SET,
		/** A text drawing primitive has been called with a character value which has no glyph in current font. */
		NO_GLYPH_FOUND,
		/**
		 * A drawing primitive would draw its shape outside the display estate, 
		 * which is forbidden. 
		 * This may be due to out of range (x,y) coordinates, or extra arguments 
		 * (e.g. too large circle radius).
		 */
		OUT_OF_DISPLAY,
		/** 
		 * A drawing primitive has been called with invalid (x,y) coordinates; this
		 * is different to `COORDS_OUT_OF_RANGE` in the sens that actual display devices 
		 * may impose specific constraint on x or y coordinates (e.g. being a multiple of 8).
		 */
		COORDS_INVALID,
		/**
		 * A drawing primitive would lead to incorrect geometry due to invalid 
		 * arguments.
		 * This may be due to various factors such as:
		 * - trying to draw a line between A and B where A == B
		 * - trying to draw a flat rectangle
		 * - trying to draw a circle with a `0` radius
		 */
		INVALID_GEOMETRY
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
		//TODO DOC
		using DRAW_MODE = DrawMode<COLOR>;

		//TODO DOC
		void set_fill_mode(const DRAW_MODE& mode)
		{
			fill_mode_ = mode;
		}

		/**
		 * Set draw mode (color, pixel op) to use for next calls to drawing primitives.
		 * The new color is simply available to subclasses in the `color_` field.
		 * 
		 * @param mode the new `DRAW_MODE` to use from now
		 */
		void set_draw_mode(const DRAW_MODE& mode)
		{
			draw_mode_ = mode;
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
		/**
		 * Check if a font is currently defined on the display.
		 * This is called by text drawing primitives to ensure drawing can be performed.
		 * If not font is defined, then `last_error_` will be set.
		 * 
		 * @return true if a font has been defined on the display
		 * @return false if no font is defined for the display
		 */
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

		/** Current draw mode that shall to fill closed surfaces. */
		DRAW_MODE fill_mode_{};

		/** Current draw mode (color, pixel op) that shall be used by drawing primitives. */
		DRAW_MODE draw_mode_{};

		/** 
		 * The result status of the last drawing primitive called.
		 * @sa last_error()
		 */
		Error last_error_ = Error::NO_ERROR;
	};

	/**
	 * Class handling drawing primitives on any display device.
	 * Tha driver for the actual display device is provided by the template
	 * argument @p DISPLAY_DEVICE.
	 * `Display` supports devices with or without a raster buffer.
	 * 
	 * @warning if @p DISPLAY_DEVICE uses a raster buffer, then drawing primitives
	 * never display anything until you call `update()`.
	 * 
	 * Drawing primitives calls may generate various errors and possibly have
	 * no impact on display.
	 * Errors may be checked after each drawing primitive call, through `last_error()` 
	 * call.
	 * 
	 * @tparam DISPLAY_DEVICE real display device class, must subclass `AbstractDisplay`
	 */
	template<typename DISPLAY_DEVICE> class Display : public DISPLAY_DEVICE
	{
	public:
		/** Integral type of X coordinate. */
		using XCOORD = typename DISPLAY_DEVICE::XCOORD;
		/** Integral type of Y coordinate. */
		using YCOORD = typename DISPLAY_DEVICE::YCOORD;

		/** Coordinates of a point in the display. */
		using POINT = Point<XCOORD, YCOORD>;

		/** 
		 * Integral type used in various calculations based on coordinates.
		 * Typically the smallest of `XCOORD` and `YCOORD`.
		 */
		using SCALAR = typename DISPLAY_DEVICE::SCALAR;

		/** Display width. Maximum X coordinate on display is `WIDTH - 1`. */
		static constexpr XCOORD WIDTH = DISPLAY_DEVICE::WIDTH;
		/** Display height. Maximum Y coordinate on display is `HEIGHT - 1`. */
		static constexpr YCOORD HEIGHT = DISPLAY_DEVICE::HEIGHT;

	protected:
		/**
		 * Integral signed type used in calculations of some drawing primitives.
		 * This must be larger or equal to `XCOORD` and `YCOORD`.
		 * It mst be large enough to store `-4 * min(WIDTH, HEIGHT)`.
		 */
		using SIGNED_SCALAR = typename DISPLAY_DEVICE::SIGNED_SCALAR;

	public:
		/** Comstruct a display instance. */
		Display()
		{
			// Check at compile-time that DISPLAY_DEVICE is a subclass of AbstractDisplayDevice
			types_traits::derives_from<DISPLAY_DEVICE, AbstractDisplayDeviceGhost>{};
		}

		/**
		 * Erase complete display.
		 */
		void erase()
		{
			DISPLAY_DEVICE::erase();
			invalidate();
		}

		/**
		 * Draw one character at the given display location.
		 * 
		 * @param point coordinates of top left character pixel
		 * @param value code of character to write; this must be available in
		 * the current loaded `Font`.
		 * 
		 * @sa AbstractDisplayDevice::set_font()
		 */
		void draw_char(POINT point, char value)
		{
			// Check one font is currently selected
			if (!DISPLAY_DEVICE::check_font()) return;
			// Check coordinates are suitable for character display
			const uint8_t width = this->font_->width();
			XCOORD x = point.x;
			YCOORD y = point.y;
			if (!is_valid_char_xy(x, y, width)) return;
			// Check glyph exists for current character
			uint16_t glyph_ref = get_glyph(value);
			if (glyph_ref == 0) return;
			// Delegate glyph display to actual device
			uint8_t displayed_width = DISPLAY_DEVICE::write_char(x, y, glyph_ref,
				*DISPLAY_DEVICE::font_, DISPLAY_DEVICE::draw_mode_);
			invalidate(x, y, XCOORD(x + displayed_width), YCOORD(y + this->font_->height() - 1));
		}

		/**
		 * Draw a string of characters at the given display location.
		 * 
		 * @param point coordinates of 1st top left character pixel
		 * @param content C-string of characters to write; all charcaters must be
		 * available in the current loaded `Font`.
		 * 
		 * @sa AbstractDisplayDevice::set_font()
		 */
		void draw_string(POINT point, const char* content)
		{
			// Check one font is currently selected
			if (!DISPLAY_DEVICE::check_font()) return;
			const uint8_t width = this->font_->width();

			XCOORD x = point.x;
			YCOORD y = point.y;
			XCOORD xcurrent = x;
			while (*content)
			{
				// Check coordinates are suitable for character display
				if (!is_valid_char_xy(xcurrent, y, width)) break;
				// Check glyph exists for current character
				uint16_t glyph_ref = get_glyph(*content);
				if (glyph_ref == 0) break;
				// Delegate glyph display to actual device
				const uint8_t displayed_width = DISPLAY_DEVICE::write_char(xcurrent, y, glyph_ref,
					*DISPLAY_DEVICE::font_, DISPLAY_DEVICE::draw_mode_);
				xcurrent += displayed_width;
				++content;
			}
			// Invalidate if needed
			if (xcurrent > x)
			{
				// Clear error only if all content was displayed without issue
				bool clear_error = (*content == 0); 
				invalidate(x, y, XCOORD(xcurrent - 1), YCOORD(y + this->font_->height() - 1), clear_error);
			}
		}

		/**
		 * Draw a string of characters at the given display location.
		 * 
		 * @param point coordinates of 1st top left character pixel
		 * @param content C-string of characters, stored in MCU Flash, to write;
		 * all charcaters must be available in the current loaded `Font`.
		 * 
		 * @sa AbstractDisplayDevice::set_font()
		 */
		void draw_string(POINT point, const flash::FlashStorage* content)
		{
			// Check one font is currently selected
			if (!DISPLAY_DEVICE::check_font()) return;
			const uint8_t width = this->font_->width();

			XCOORD x = point.x;
			YCOORD y = point.y;
			XCOORD xcurrent = x;
			uint16_t address = (uint16_t) content;
			char value;
			while ((value = pgm_read_byte(address)) != 0)
			{
				// Check coordinates are suitable for character display
				if (!is_valid_char_xy(xcurrent, y, width)) break;
				// Check glyph exists for current character
				uint16_t glyph_ref = get_glyph(value);
				if (glyph_ref == 0) break;
				// Delegate glyph display to actual device
				const uint8_t displayed_width = DISPLAY_DEVICE::write_char(xcurrent, y, glyph_ref, 
					*DISPLAY_DEVICE::font_, DISPLAY_DEVICE::draw_mode_);
				xcurrent += displayed_width;
				++address;
			}
			// Invalidate if needed
			if (xcurrent > x)
			{
				// Clear error only if all content was displayed without issue
				bool clear_error = (value == 0);
				invalidate(x, y, XCOORD(xcurrent -1), YCOORD(y + this->font_->height() - 1), clear_error);
			}
		}

		/**
		 * Draw a pixel at given coordinate.
		 * 
		 * @param point coordinates of pixel to draw
		 */
		void draw_point(POINT point)
		{
			XCOORD x = point.x;
			YCOORD y = point.y;
			if (!is_valid_xy(x, y)) return;
			if (DISPLAY_DEVICE::set_pixel(x, y, DISPLAY_DEVICE::draw_mode_))
				invalidate(x, y, x, y);
			else
				// Even when set_pixel() returns false, this is not an error!
				DISPLAY_DEVICE::last_error_ = Error::NO_ERROR;
		}

		/**
		 * Draw a line between 2 points, at given coordinates.
		 * 
		 * @param point1 coordinates of line 1st point
		 * @param point2 coordinates of line 2nd point
		 */
		void draw_line(POINT point1, POINT point2)
		{
			XCOORD x1 = point1.x;
			YCOORD y1 = point1.y;
			XCOORD x2 = point2.x;
			YCOORD y2 = point2.y;
			if (!is_valid_xy(x1, y1)) return;
			if (!is_valid_xy(x2, y2)) return;

			// Check if specifc case (vertical or horizontal line)
			if (x1 == x2)
			{
				// if 2 points are the same: nothing to do
				if (y1 == y2)
				{
					DISPLAY_DEVICE::last_error_ = Error::INVALID_GEOMETRY;
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
			invalidate(x1, y1, x2, y2);
		}

		/**
		 * Draw a rectangle defined by 2 corner points, at given coordinates.
		 * 
		 * @param point1 coordinates of rectangle 1st corner
		 * @param point2 coordinates of rectangle 2nd corner
		 */
		void draw_rectangle(POINT point1, POINT point2)
		{
			XCOORD x1 = point1.x;
			YCOORD y1 = point1.y;
			XCOORD x2 = point2.x;
			YCOORD y2 = point2.y;
			if (!is_valid_xy(x1, y1)) return;
			if (!is_valid_xy(x2, y2)) return;
			if ((x1 == x2) || (y1 == y2))
			{
				DISPLAY_DEVICE::last_error_ = Error::INVALID_GEOMETRY;
				return;
			}
			// Possibly swap x1-x2 and y1-y2
			swap_to_sort(x1, x2);
			swap_to_sort(y1, y2);

			// Draw edges
			if (DISPLAY_DEVICE::draw_mode_)
			{
				// Simply draw 2 horizontal and 2 vertical lines
				draw_hline(x1, y1, x2);
				draw_hline(x1, y2, x2);
				// Note that we avoid drawing the same pixels (corners) twice 
				// (due to a drawing mode that might potentially be XOR)
				draw_vline(x1, y1 + 1, y2 - 1);
				draw_vline(x2, y1 + 1, y2 - 1);
			}

			//TODO Fill rectangle inside
			if (DISPLAY_DEVICE::fill_mode_)
			{
				// Simply draw enough horizontal lines
				for (YCOORD y = y1 + 1; y < y2; ++y)
					draw_hline(x1 + 1, y, x2 - 1);
			}

			invalidate(x1, y1, x2, y2);
		}

		/**
		 * Draw a circle defined by its center, at given coordinates, and its radius.
		 * 
		 * @param center coordinates of circle center
		 * @param radius circle radius
		 */
		void draw_circle(POINT center, SCALAR radius)
		{
			//TODO use filler_ !
			XCOORD xc = center.x;
			YCOORD yc = center.y;
			if (!is_valid_xy(xc, yc)) return;
			if (radius == 0)
			{
				DISPLAY_DEVICE::last_error_ = Error::INVALID_GEOMETRY;
				return;
			}
			if (	(xc < radius) || (xc + radius >= WIDTH)
				||	(yc < radius) || (yc + radius >= HEIGHT))
			{
				DISPLAY_DEVICE::last_error_ = Error::OUT_OF_DISPLAY;
				return;
			}
			// Apply Bresenham's circle algorithm
			draw_circle_bresenham(xc, yc, radius);
			invalidate(XCOORD(xc - radius), YCOORD(yc - radius), XCOORD(xc + radius), YCOORD(yc + radius));
		}

		void draw_polyline(std::initializer_list<POINT> points)
		{
			draw_lines(points, false);
		}

		void draw_polygon(std::initializer_list<POINT> points)
		{
			//TODO use filler_ !
			draw_lines(points, true);
		}

		//TODO additional 2D primitives here? e.g. arc, region fill
		// void draw_arc()

		/**
		 * For display devices having a raster buffer, this method copies invalid
		 * (modified) parts of the raster buffer to the device.
		 * For such devices, nothing will get actually drawn until `update()` is called.
		 * 
		 * This is useless for devices with direct draw to device (no raster buffer).
		 */
		void update()
		{
			if (DISPLAY_DEVICE::HAS_RASTER && !invalid_area_.empty)
			{
				DISPLAY_DEVICE::update(invalid_area_.x1, invalid_area_.y1, invalid_area_.x2, invalid_area_.y2);
				invalid_area_.empty = true;
			}
		}

		/**
		 * For display devices having a raster buffer, this method copies the whole
		 * raster buffer to the device.
		 * 
		 * This is useless for devices with direct draw to device (no raster buffer).
		 */
		void force_update()
		{
			invalidate();
			update();
		}

	protected:
		/// @cond notdocumented
		struct InvalidArea
		{
			InvalidArea() = default;
			InvalidArea(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2)
				: x1{x1}, y1{y1}, x2{x2}, y2{y2}, empty{false} {}

			InvalidArea& operator+=(const InvalidArea& a)
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

		using INVALID_AREA = InvalidArea;

		void invalidate(XCOORD x1, XCOORD y1,XCOORD x2, YCOORD y2, bool clear_error = true)
		{
			if (DISPLAY_DEVICE::HAS_RASTER)
				invalid_area_ += INVALID_AREA{x1, y1, x2, y2};
			if (clear_error)
				DISPLAY_DEVICE::last_error_ = Error::NO_ERROR;
		}

		void invalidate()
		{
			if (DISPLAY_DEVICE::HAS_RASTER)
				invalid_area_ = INVALID_AREA{0, 0, WIDTH - 1, HEIGHT - 1};
			DISPLAY_DEVICE::last_error_ = Error::NO_ERROR;
		}

		bool is_valid_xy(XCOORD x, YCOORD y)
		{
			if ((x < WIDTH) && (y < HEIGHT)) return true;
			DISPLAY_DEVICE::last_error_ = Error::OUT_OF_DISPLAY;
			return false;
		}

		bool is_valid_char_xy(XCOORD x, YCOORD y, uint8_t width)
		{
			if ((x + width > WIDTH) || (y >= HEIGHT))
			{
				DISPLAY_DEVICE::last_error_ = Error::OUT_OF_DISPLAY;
				return false;
			}
			if (DISPLAY_DEVICE::is_valid_char_xy(x, y)) return true;
			DISPLAY_DEVICE::last_error_ = Error::COORDS_INVALID;
			return false;
		}

		uint16_t get_glyph(char value)
		{
			// Load pixmap for current character
			uint16_t glyph_ref = this->font_->get_char_glyph_ref(value);
			if (glyph_ref != 0) return glyph_ref;
			DISPLAY_DEVICE::last_error_ = Error::NO_GLYPH_FOUND;
			return 0;
		}

		void draw_lines(std::initializer_list<POINT> points, bool polygon)
		{
			if (points.size() < 2)
			{
				DISPLAY_DEVICE::last_error_ = Error::INVALID_GEOMETRY;
				return;
			}
			const POINT* next = points.begin();
			POINT current = *next;
			POINT first = current;
			while (++next != points.end())
			{
				draw_line(current, *next);
				current = *next;
			}
			if (polygon)
				draw_line(current, first);
		}

		void draw_vline(XCOORD x1, YCOORD y1, YCOORD y2)
		{
			swap_to_sort(y1, y2);
			for (YCOORD y = y1; y <= y2; ++y)
				DISPLAY_DEVICE::set_pixel(x1, y, DISPLAY_DEVICE::draw_mode_);
		}
		
		void draw_hline(XCOORD x1, YCOORD y1, XCOORD x2)
		{
			swap_to_sort(x1, x2);
			for (XCOORD x = x1; x <= x2; ++x)
				DISPLAY_DEVICE::set_pixel(x, y1, DISPLAY_DEVICE::draw_mode_);
		}

		// Draw a segment according to Bresenham algorithm
		// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
		// https://fr.wikipedia.org/wiki/Algorithme_de_trac%C3%A9_de_segment_de_Bresenham
		void draw_line_bresenham(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2)
		{
			// We are sure that x1 < x2 when calling this method
			SIGNED_SCALAR dx = SIGNED_SCALAR(x2) - SIGNED_SCALAR(x1);
			SIGNED_SCALAR dy = SIGNED_SCALAR(y2) - SIGNED_SCALAR(y1);

			if (dy > 0)
			{
				// 1st quadrant
				draw_line_bresenham_1st_quadrant(x1, y1, x2, y2, dx, dy);
			}
			else
			{
				// 4th quadrant
				draw_line_bresenham_4th_quadrant(x1, y1, x2, y2, dx, dy);
			}
		}

		void draw_line_bresenham_1st_quadrant(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2, 
			SIGNED_SCALAR dx, SIGNED_SCALAR dy)
		{
			if (dx >= dy)
			{
				// 1st octant
				SIGNED_SCALAR e = dx;
				dx *= 2;
				dy *= 2;
				while (true)
				{
					DISPLAY_DEVICE::set_pixel(x1, y1, DISPLAY_DEVICE::draw_mode_);
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
					DISPLAY_DEVICE::set_pixel(x1, y1, DISPLAY_DEVICE::draw_mode_);
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

		void draw_line_bresenham_4th_quadrant(XCOORD x1, YCOORD y1, XCOORD x2, YCOORD y2, 
			SIGNED_SCALAR dx, SIGNED_SCALAR dy)
		{
			if (dx >= -dy)
			{
				// 8th octant
				SIGNED_SCALAR e = dx;
				dx *= 2;
				dy *= 2;
				while (true)
				{
					DISPLAY_DEVICE::set_pixel(x1, y1, DISPLAY_DEVICE::draw_mode_);
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
					DISPLAY_DEVICE::set_pixel(x1, y1, DISPLAY_DEVICE::draw_mode_);
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

		// https://fr.wikipedia.org/wiki/Algorithme_de_trac%C3%A9_d%27arc_de_cercle_de_Bresenham
		void draw_circle_bresenham(XCOORD xc, YCOORD yc, SCALAR radius)
		{
			XCOORD x = 0;
			YCOORD y = radius;
			SIGNED_SCALAR m = 5 - 4 * radius;
			while (x <= y)
			{
				DISPLAY_DEVICE::set_pixel(x +  xc, y + yc, DISPLAY_DEVICE::draw_mode_);
				DISPLAY_DEVICE::set_pixel(y +  xc, x + yc, DISPLAY_DEVICE::draw_mode_);
				DISPLAY_DEVICE::set_pixel(-x +  xc, y + yc, DISPLAY_DEVICE::draw_mode_);
				DISPLAY_DEVICE::set_pixel(-y +  xc, x + yc, DISPLAY_DEVICE::draw_mode_);
				DISPLAY_DEVICE::set_pixel(x +  xc, -y + yc, DISPLAY_DEVICE::draw_mode_);
				DISPLAY_DEVICE::set_pixel(y +  xc, -x + yc, DISPLAY_DEVICE::draw_mode_);
				DISPLAY_DEVICE::set_pixel(-x +  xc, -y + yc, DISPLAY_DEVICE::draw_mode_);
				DISPLAY_DEVICE::set_pixel(-y +  xc, -x + yc, DISPLAY_DEVICE::draw_mode_);
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
		/// @cond notdocumented
		
	private:
		// Minimal rectangle to update
		INVALID_AREA invalid_area_ = INVALID_AREA::EMPTY;
	};

	/// @cond notdocumented
	template<typename DISPLAY_DEVICE>
	typename Display<DISPLAY_DEVICE>::InvalidArea Display<DISPLAY_DEVICE>::InvalidArea::EMPTY = InvalidArea{};
	/// @endcond
}

#endif /* DISPLAY_HH */
/// @endcond
