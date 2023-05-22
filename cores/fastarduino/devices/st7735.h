//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * API to handle Arduino LCD display through SPI interface (actually not really SPI 
 * as only MOSI, not MISO, pin is used for data transfer).
 * 
 * It is not clear if Arduino LCD is driven by ST7735 or ILI9163 chip (both are
 * very similar, ILI just seems to have more features: resolutions, scrolling...)
 * 
 * Arduino LCD comes with level adapters hence you can freely power and drive
 * Arduino LCD with 5V.
 * 
 * Example wiring for Arduino UNO:
 * 
 * - pin 11 (PB3,MOSI) ------ MOSI
 * - pin 13 (PB5, SCK) ------ SCK
 * - pin 10 (PB2, SS)  ------ LCD CS
 * - pin 9 (PB1)       ------ D/C
 * - pin 8 (PB0)       ------ Reset
 * - 5V            --[=1K=]-- BL (backlight LED)
 * - 5V                ------ 5V
 * - GND               ------ GND
 * 
 * In the future, we shall decouple chip (ST7735, ILI9163) from actual LCD screen
 * (through the use of settings or traits) so that an actual display breakout can
 * be handled as a combination of both.
 * 
 * @sa https://docs.arduino.cc/retired/other/arduino-lcd-screen
 */
#ifndef ST7735_HH
#define ST7735_HH

#include <string.h>

#include "../initializer_list.h"
#include "../bits.h"
#include "../flash.h"
#include "../gpio.h"
#include "../spi.h"
#include "../time.h"
#include "../utilities.h"
#include "display.h"
#include "font.h"

namespace devices::display
{
	/**
	 * Defines API to handle ST7735 TFT controller/driver chip.
	 * It contains all definitions specific to ST7735 display chip.
	 * 
	 * @note ILI9163 chip is very similar to ST7735 and should be supported
	 * through `ST7735` class too.
	 */
	namespace st7735
	{
	}
}

namespace devices::display::st7735
{
	/** 
	 * LCD display orientation.
	 * It shall fit the orientation of display "in real life" so that all drawings,
	 * text in particular, are properly oriented for the end user.
	 * @sa ST7735
	 */
	enum class Orientation: uint8_t
	{
		/** Landscape */
		LANDSCAPE = bits::BV8(5, 7),
		/** Portrait */
		PORTRAIT = 0,
		/** Landscape but reversed */
		REVERSE_LANDSCAPE = bits::BV8(5, 6),
		/** Portrait but reversed */
		REVERSE_PORTRAIT = bits::BV8(6, 7)
	};

	/**
	 * Color Model to use for the device.
	 * Each model has its advantages and drawbacks (performance, number of colors).
	 * @sa ST7735
	 */
	enum class ColorModel: uint8_t
	{
		/** 4 bits per primary color (12 bits per pixel). */
		RGB_444 = bits::BV8(1, 0),
		/** 16 bits per pixel (5 bits for red and blue, 6 bits for green). */
		RGB_565 = bits::BV8(2, 0),
		/** 6 bits per primary color (18 bits per pixel). */
		RGB_666 = bits::BV8(2, 1)
	};

	/**
	 * Resolutions supported by ST7735 and ILI9163 chips.
	 * @warning ST7735 supports only the 2 first resolutions: 132x162 and 128x160.
	 * Behavior is udnefined if you use other resolutions with ST7735 chip. 
	 */
	enum class Resolution: uint8_t
	{
		/** 132x162 resolution, common to ST7735 and ILI9163 chips. */
		RESOLUTION_132X162,
		/** 128x160 resolution, common to ST7735 and ILI9163 chips. */
		RESOLUTION_128X160,
		/** 128x128 resolution, specific to ILI9163 chip. */
		RESOLUTION_128X128,
		/** 130x130 resolution, specific to ILI9163 chip. */
		RESOLUTION_130X130,
		/** 132x132 resolution, specific to ILI9163 chip. */
		RESOLUTION_132X132,
		/** 120x160 resolution, specific to ILI9163 chip. */
		RESOLUTION_120X160,
	};

	/**
	 * Class encapsulating the color of a pixel in `ColorModel::RGB_444` model,
	 * ie 4 bits per primary color, red, green, blue.
	 * 
	 * This is the most efficient `ColorModel` to use with ST7735 chip, but it is
	 * much limited on colors nuances (4K distinct colors).
	 * 
	 * @sa ColorModel::RGB_444
	 * @sa RGB_565_COLOR
	 * @sa RGB_666_COLOR
	 */
	class RGB_444_COLOR
	{
	public:
		/**
		 * Create an 444 RGB color directly from a 16-bits word, of which only
		 * 12 MSB are used, 4 bits respectively for red, green and blue.
		 */
		constexpr RGB_444_COLOR(uint16_t rgb = 0): rgb_{rgb} {}

		/**
		 * Create an 444 RGB color from 3 primary colors, each defined as a byte,
		 * of which only 4 MSB are used, ie 0xFF is the same 0xF0 for given value.
		 */
		constexpr RGB_444_COLOR(uint8_t red, uint8_t green, uint8_t blue):
			rgb_{rgb_to_color(red, green, blue)} {}

		/**
		 * Return the color as a word, which only 12 MSB are significant.
		 */
		uint16_t color() const
		{
			return rgb_;
		}

	private:
		static constexpr uint16_t MASK_RED		= 0b1111'0000'0000'0000;
		static constexpr uint16_t MASK_GREEN	= 0b0000'1111'0000'0000;
		static constexpr uint16_t MASK_BLUE		= 0b0000'0000'1111'0000;

		static constexpr uint16_t rgb_to_color(uint8_t red, uint8_t green, uint8_t blue)
		{
			return	((uint16_t(red) << 12) & MASK_RED)		|
					((uint16_t(green) << 8) & MASK_GREEN)	|
					((uint16_t(blue) << 4) & MASK_BLUE);
		}

		uint16_t rgb_;
	};

	/**
	 * Class encapsulating the color of a pixel in `ColorModel::RGB_565` model,
	 * ie 5 bits for red, 6 bits for green, and 5 bits for blue..
	 * 
	 * This is the best trade-off `ColorModel` to use with ST7735 chip, as it offers
	 * wide scale of colors nuances (64K) but it is quite efficient.
	 * 
	 * @sa ColorModel::RGB_565
	 * @sa RGB_444_COLOR
	 * @sa RGB_666_COLOR
	 */
	class RGB_565_COLOR
	{
	public:
		/**
		 * Create an 565 RGB color directly from a 16-bits word.
		 */
		constexpr RGB_565_COLOR(uint16_t rgb = 0): rgb_{rgb} {}

		/**
		 * Create an 565 RGB color from 3 primary colors, each defined as a byte,
		 * of which only 5 MSB (for red and blue) or 6 MSB (for green) are used.
		 */
		constexpr RGB_565_COLOR(uint8_t red, uint8_t green, uint8_t blue):
			rgb_{rgb_to_color(red, green, blue)} {}

		/**
		 * Return the color as a word, directly understood by ST7735 chip.
		 */
		uint16_t color() const
		{
			return rgb_;
		}

	private:
		static constexpr uint16_t MASK_RED		= 0b1111'1000'0000'0000;
		static constexpr uint16_t MASK_GREEN	= 0b0000'0111'1110'0000;
		static constexpr uint16_t MASK_BLUE		= 0b0000'0000'0001'1111;

		static constexpr uint16_t rgb_to_color(uint8_t red, uint8_t green, uint8_t blue)
		{
			return	((uint16_t(red) << 8) & MASK_RED)		|
					((uint16_t(green) << 3) & MASK_GREEN)	|
					(uint16_t(blue) & MASK_BLUE);
		}

		uint16_t rgb_;
	};

	/**
	 * Class encapsulating the color of a pixel in `ColorModel::RGB_666` model,
	 * ie 6 bits for each primary color.
	 * 
	 * This is the least efficient `ColorModel`, but it provides the most color 
	 * nuances (256K), which you may need to display photographs, for instance.
	 * 
	 * @sa ColorModel::RGB_565
	 * @sa RGB_444_COLOR
	 * @sa RGB_666_COLOR
	 */
	class RGB_666_COLOR
	{
	public:
		/**
		 * Create a black 666 RGB color.
		 */
		constexpr RGB_666_COLOR()
			:	red_{0}, green_{0}, blue_{0} {}

		/**
		 * Create an 666 RGB color from 3 primary colors, each defined as a byte,
		 * of which only 6 MSB are used, ie 0xFC is the same as 0xFF.
		 */
		constexpr RGB_666_COLOR(uint8_t red, uint8_t green, uint8_t blue)
			:	red_{uint8_t(red & MASK)}, green_{uint8_t(green & MASK)}, blue_{uint8_t(blue & MASK)} {}
		
		/**
		 * Return the red compopnent of this color as a byte, which only 6 MSB are significant.
		 */
		uint8_t red() const
		{
			return red_;
		}

		/**
		 * Return the green component of this color as a byte, which only 6 MSB are significant.
		 */
		uint8_t green() const
		{
			return green_;
		}

		/**
		 * Return the blue component of this color as a byte, which only 6 MSB are significant.
		 */
		uint8_t blue() const
		{
			return blue_;
		}

	private:
		static constexpr uint8_t MASK = 0b1111'1100;

		uint8_t red_;
		uint8_t green_;
		uint8_t blue_;
	};

	/** 
	 * Gamma correction curve to apply to the display.
	 * ST7735 (and ILI9163) provide 4 predefined gamma curves.
	 * Actual definition of each curve depends on GS pin level (datasheet §10.1.16).
	 */
	enum class Gamma: uint8_t
	{
		/** Pre-edfined GC0 gamma curve. */
		GC0 = 0x01,
		/** Pre-edfined GC1 gamma curve. */
		GC1 = 0x02,
		/** Pre-edfined GC2 gamma curve. */
		GC2 = 0x04,
		/** Pre-edfined GC3 gamma curve. */
		GC3 = 0x08
	};

	/**
	 * SPI device driver for ST7735 display chip.
	 * 
	 * This driver offers various API to:
	 * - display characters or strings
	 * - display pixels
	 * All drawing API is applied directly on the device (no raster buffer).
	 * 
	 * @note ST7735 chip is very similar to IL9163 (which is a bit more powerful,
	 * in terms of supported resolutions and features, such as scrolling). Using
	 * `ST7735` class to drive an `ILI9163` should work directly, but with reduced 
	 * fonctionality.
	 * 
	 * @warning This class shall be used along with `devices::display::Display` 
	 * template class. It cannot be instantiated on its own.
	 * 
	 * All public API in ST7735 is available through the encapsulating Display
	 * instance.
	 * 
	 * Once Display has been instantiated for ST7735 driver, you should call
	 * the following API before it can be used to display anything:
	 * 1. `hard_reset()` [optional, but advised]
	 * 2. `soft_reset()`
	 * 3. `sleep_out()`
	 * 4. `display_on()`
	 *
	 * @tparam SCE the output pin used for Chip Selection of the ST7735 chip on
	 * the SPI bus.
	 * @tparam DC the output pin used to select Data (high) or Command (low) mode 
	 * of ST7735 chip.
	 * @tparam RST the output pin used to reset ST7735 chip
	 * @tparam COLOR_MODEL the number of color bits per pixel, as supported by ST7735
	 * @tparam ORIENTATION the display orientation, as seen by the end user
	 * @tparam RESOLUTION the display resolution
	 * @tparam RGB_BGR `true` if the display inverts RGB order of pixels
	 * 
	 * @sa Display
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST,
		ColorModel COLOR_MODEL, Orientation ORIENTATION, Resolution RESOLUTION, bool RGB_BGR>
	class ST7735 : public spi::SPIDevice<SCE, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(8'000'000UL)>
	{
	private:
		using TRAITS = DisplayDeviceTrait<ST7735<SCE, DC, RST, COLOR_MODEL, ORIENTATION, RESOLUTION, RGB_BGR>>;
		using COLOR = typename TRAITS::COLOR;
		static constexpr uint8_t WIDTH = TRAITS::WIDTH;
		static constexpr uint8_t HEIGHT = TRAITS::HEIGHT;
		using DRAW_CONTEXT = DrawContext<COLOR, false>;

	public:
		/**
		 * Start ST7735 chip before actual usage.
		 * This handles chip reset, sleep mode leave, chip configuration and display
		 * switch on.
		 * 
		 * @warning Calling this method is absolutely mandatory before any drawing 
		 * primitive can be called.
		 * 
		 * @param force_hard_reset indicate whether hard reset (through `RST` pin)
		 * is required; typically this is not needed at power on (a soft reset is enough).
		 * 
		 * @sa hard_reset()
		 * @sa soft_reset()
		 * @sa sleep_out()
		 * @sa display_on()
		 */
		void begin(bool force_hard_reset = false)
		{
			if (force_hard_reset)
				hard_reset();
			soft_reset();
			sleep_out();
			set_color_model(COLOR_MODEL);
			set_orientation(ORIENTATION);
			set_column_address(0, WIDTH - 1);
			set_row_address(0, HEIGHT - 1);
			display_on();
		}

		/**
		 * Perform a hard reset of the ST7735 chip, ie through the `RST` pin.
		 * After this method is called, some reconfiguration may be needed; please
		 * refer to chip datasheet for further information.
		 * 
		 * @sa soft_reset()
		 */
		void hard_reset()
		{
			// Reset device according to datasheet §9.12
			rst_.clear();
			time::delay_us(10);
			rst_.set();
			time::delay_ms(120);
		}

		/**
		 * Perform a software reset of the ST7735 chip.
		 * After this method is called, some reconfiguration may be needed; please
		 * refer to chip datasheet for further information.
		 * 
		 * @sa hard_reset()
		 */
		void soft_reset()
		{
			send_command(CMD_SOFT_RESET);
			time::delay_ms(50);
		}

		/**
		 * Enter ST7735 into sleep mode.
		 * In sleep mode, the chip consumes very little current but no display occurs,
		 * however, it is possible tooerform drawing primitives that will affect 
		 * ST7735 memory raster, and will display later when leaving sleep mode.
		 * 
		 * @sa sleep_out()
		 */
		void sleep_in()
		{
			send_command(CMD_SLEEP_IN);
			time::delay_ms(120);
		}

		/**
		 * Leave ST7735 from sleep mode.
		 * After execution, the chip will display again its raster content to the LCD.
		 * 
		 * @sa sleep_in()
		 */
		void sleep_out()
		{
			send_command(CMD_SLEEP_OUT);
			time::delay_ms(120);
		}

		/**
		 * Enter ST7735 chip into idle mode.
		 * In this mdoe, the chip consumes less current but display is still active,
		 * only with lower resolution (8 bits only, which may be enough for simple
		 * user interface or pure text display).
		 * 
		 * @sa idle_off()
		 */
		void idle_on()
		{
			send_command(CMD_IDLE_ON);
		}

		/**
		 * Leave ST7735 from idle mode.
		 * After execution, the chip displays pixels in 18-bit mode (even though
		 * you may only use 444 or 565 `ColorModel`).
		 * 
		 * @sa idle_on()
		 */
		void idle_off()
		{
			send_command(CMD_IDLE_OFF);
		}

		/**
		 * Enter ST7735 chip into partial mode, meaning that only a subset (rows)
		 * of the display is used.
		 * This may reduce current consumption, depending on how many rows are left active.
		 * 
		 * You leav epartial mode through `normal_mode()`.
		 * 
		 * @param start_row the index of first row to activate
		 * @param end_row the index of last row to activate
		 * 
		 * @sa normal_mode()
		 */
		void partial_mode(uint16_t start_row, uint16_t end_row)
		{
			//TODO Check validity of args!
			send_command(CMD_PARTIAL_AREA, {
				utils::high_byte(start_row), utils::low_byte(start_row),
				utils::high_byte(end_row), utils::low_byte(end_row)});
			send_command(CMD_PARTIAL_MODE);
		}

		/**
		 * Leave ST7735 from partial mode, back into normal mode.
		 * In normal mode, the full display is active.
		 * 
		 * @sa partial_mode()
		 */
		void normal_mode()
		{
			send_command(CMD_NORMAL_MODE);
		}

		/**
		 * Invert whole display.
		 * 
		 * @sa invert_off()
		 */
		void invert_on()
		{
			send_command(CMD_INVERT_ON);
		}

		/**
		 * Leave inversion mode.
		 * 
		 * @sa invert_on()
		 */
		void invert_off()
		{
			send_command(CMD_INVERT_OFF);
		}

		/**
		 * Blank full display.
		 * Drawing primitvies can still be called but will not affect display, only
		 * raster memory, so that they will become visible later, when `display_on()`
		 * will be called.
		 * 
		 * @sa display_on()
		 */
		void display_off()
		{
			send_command(CMD_DISPLAY_OFF);
			time::delay_ms(120);
		}

		/**
		 * Recover from display off mode.
		 * Once executed, LCD display will be refresshed from the chip raster memory.
		 * 
		 * @sa display_off()
		 */
		void display_on()
		{
			send_command(CMD_DISPLAY_ON);
			time::delay_ms(120);
		}

		/**
		 * Enable Tearing Effect line for ST7735 chip.
		 * This allows synchronization of drawing commands with actual LCD display,
		 * to avoid so-called "tearing effect" in the middle of an image.
		 * 
		 * @warning Most breakouts do not expose the TE signal line, hence this API is
		 * useless with such breakouts.
		 * 
		 * @param vertical_blanking_only  
		 * 
		 * @sa tear_effect_off()
		 */
		void tear_effect_on(bool vertical_blanking_only)
		{
			const uint8_t telom = (vertical_blanking_only ? 0 : 1);
			send_command(CMD_TEAR_ON, telom);
		}

		/**
		 * Disable Tearing Effect line for ST7735 chip.
		 * 
		 * @sa tear_effect_on()
		 */
		void tear_effect_off()
		{
			send_command(CMD_TEAR_OFF);
		}

		/**
		 * Set the gamma correction curve to use by the chip, among 4 pre-defined
		 * curves.
		 * 
		 * @param gamma_curve the gamma curve the chip shall use now
		 */
		void set_gamma(Gamma gamma_curve)
		{
			send_command(CMD_SET_GAMMA, uint8_t(gamma_curve));
		}

		/**
		 * Fill the entire display with @p color.
		 * 
		 * @param color the new color to fill the whole display with
		 */
		void fill_screen(COLOR color)
		{
			set_column_address(0, WIDTH - 1);
			set_row_address(0, HEIGHT - 1);
			start_memory_write();
			for (uint8_t x = 0; x < WIDTH; ++x)
				for (uint8_t y = 0; y < HEIGHT; ++y)
					write_memory(color);
			stop_memory_write();
		}

	protected:
		/// @cond notdocumented
		ST7735() = default;

		void erase()
		{
			fill_screen(COLOR{});
		}
		
		// NOTE Coordinates must have been first verified by caller
		bool set_pixel(uint8_t x, uint8_t y, const DRAW_CONTEXT& context)
		{
			set_column_address(x, x);
			set_row_address(y , y);
			start_memory_write();
			//TODO should use context.draw_mode().pixel_op() instead!
			write_memory(context.draw_mode().color());
			stop_memory_write();
			return true;
		}

		bool is_valid_char_xy(UNUSED uint8_t x, UNUSED uint8_t y)
		{
			return true;
		}

		// NOTE Coordinates must have been first verified by caller
		uint8_t write_char(uint8_t x, uint8_t y, uint16_t glyph_ref, const DRAW_CONTEXT& context)
		{
			// Check column and row not out of range for characters!
			const uint8_t width = context.font().width();
			const uint8_t height = context.font().height();
			const uint8_t interchar_space = ((x + width + 1) < WIDTH) ? context.font().interchar_space() : 0;
			const COLOR fg = context.foreground();
			const COLOR bg = context.background();

			set_column_address(x, x + width - 1 + interchar_space);
			set_row_address(y, y + height - 1);
			start_memory_write();
			uint8_t glyph_index  = 0;
			for (uint8_t glyph_row = 0; glyph_row < context.font().glyph_rows(); ++glyph_row)
			{
				// Counter of remaining width to draw (in pixels)
				uint8_t row_width = width;
				for (uint8_t glyph_col = 0; glyph_col < context.font().glyph_cols(); ++glyph_col)
				{
					uint8_t pixel_bar = context.font().get_char_glyph_byte(glyph_ref, glyph_index++);
					uint8_t mask = 0x80;
					// On last column, width may be less than 8!
					const uint8_t current_width = (row_width > 8 ? 8 : row_width);
					row_width -= 8;
					for (uint8_t i = 0; i < current_width; ++i)
					{
						const COLOR& color = (pixel_bar & mask) ? fg : bg;
						write_memory(color);
						mask >>= 1;
					}
				}
				// add interspace if needed
				for (uint8_t i = 0; i < interchar_space; ++i)
				{
					write_memory(bg);
				}
			}
			stop_memory_write();

			// Return actual width written to display
			return width + interchar_space;
		}

		// No invalid region, so no update() operation in effect
		void update(UNUSED uint8_t x1, UNUSED uint8_t y1, UNUSED uint8_t x2, UNUSED uint8_t y2) {}

		void set_orientation(Orientation orientation)
		{
			send_command(CMD_SET_ADDRESS_MODE, uint8_t(orientation) | RGB_ORDER);
		}
		void set_color_model(ColorModel model)
		{
			send_command(CMD_PIXEL_FORMAT, uint8_t(model));
		}

		void set_column_address(uint16_t xstart, uint16_t xend)
		{
			send_command(CMD_SET_COLUMN_ADDRESS, {
				utils::high_byte(xstart), utils::low_byte(xstart),
				utils::high_byte(xend), utils::low_byte(xend)});
		}

		void set_row_address(uint16_t ystart, uint16_t yend)
		{
			send_command(CMD_SET_ROW_ADDRESS, {
				utils::high_byte(ystart), utils::low_byte(ystart),
				utils::high_byte(yend), utils::low_byte(yend)});
		}

		void start_memory_write()
		{
			send_command(CMD_WRITE_MEMORY);
			pixel_index_ = 0;
		}
		void write_memory(COLOR color)
		{
			write_memory_(color);
		}
		void stop_memory_write()
		{
			if (pixel_index_ % 2)
				send_data({utils::high_byte(first_444_color_), utils::low_byte(first_444_color_)});
			send_command(CMD_NOP);
		}
		/// @endcond

	private:
		// Value to add to MADCTL (CMD_SET_ADDRESS_MODE) for Arduino LCD
		// NOTE: this flag depends on the LCD screen
		static constexpr uint8_t RGB_ORDER = (RGB_BGR ? 0x08 : 0x00);
		// ST7735 commands (note: subset of ILI9163)
		static constexpr uint8_t CMD_NOP				= 0x00;
		static constexpr uint8_t CMD_SOFT_RESET			= 0x01;

		static constexpr uint8_t CMD_SLEEP_IN			= 0x10;
		static constexpr uint8_t CMD_SLEEP_OUT			= 0x11;
		static constexpr uint8_t CMD_DISPLAY_ON			= 0x29;
		static constexpr uint8_t CMD_DISPLAY_OFF		= 0x28;
		static constexpr uint8_t CMD_IDLE_ON			= 0x39;
		static constexpr uint8_t CMD_IDLE_OFF			= 0x38;
		static constexpr uint8_t CMD_INVERT_ON			= 0x21;
		static constexpr uint8_t CMD_INVERT_OFF			= 0x20;

		static constexpr uint8_t CMD_PARTIAL_MODE		= 0x12;
		static constexpr uint8_t CMD_NORMAL_MODE		= 0x13;
		static constexpr uint8_t CMD_PARTIAL_AREA		= 0x30;
		static constexpr uint8_t CMD_TEAR_OFF			= 0x34;
		static constexpr uint8_t CMD_TEAR_ON			= 0x35;

		static constexpr uint8_t CMD_SET_ADDRESS_MODE	= 0x36;
		static constexpr uint8_t CMD_PIXEL_FORMAT		= 0x3A;

		static constexpr uint8_t CMD_SET_GAMMA			= 0x26;

		static constexpr uint8_t CMD_SET_COLUMN_ADDRESS	= 0x2A;
		static constexpr uint8_t CMD_SET_ROW_ADDRESS	= 0x2B;
		static constexpr uint8_t CMD_WRITE_MEMORY		= 0x2C;

		// According to datasheet §6.2,
		// CS is cleared first, then DC is cleared, then command is transferred
		// If data then DC is set, then data is transferred
		void send_command(uint8_t command)
		{
			this->start_transfer();
			dc_.clear();
			this->transfer(command);
			this->end_transfer();
		}

		void send_command(uint8_t command, uint8_t data)
		{
			this->start_transfer();
			dc_.clear();
			this->transfer(command);
			dc_.set();
			this->transfer(data);
			this->end_transfer();
		}
		
		void send_command(uint8_t command, std::initializer_list<uint8_t> data)
		{
			this->start_transfer();
			dc_.clear();
			this->transfer(command);
			dc_.set();
			this->transfer(data.begin(), data.size());
			this->end_transfer();
		}

		void send_data(std::initializer_list<uint8_t> data)
		{
			this->start_transfer();
			dc_.set();
			this->transfer(data.begin(), data.size());
			this->end_transfer();
		}

		void write_memory_(RGB_444_COLOR color)
		{
			if (pixel_index_ % 2)
			{
				// This is 2nd pixel, write 1st and 2nd pixels
				const uint16_t value = color.color() >> 4;
				send_data({
					utils::high_byte(first_444_color_), 
					uint8_t(utils::low_byte(first_444_color_) | utils::high_byte(value)),
					utils::low_byte(value)});
			}
			else
			{
				// 1st pixel, do not write it now, store it for next call
				first_444_color_ = color.color();
			}
			++pixel_index_;
		}

		void write_memory_(RGB_565_COLOR color)
		{
			uint16_t value = color.color();
			send_data({utils::high_byte(value), utils::low_byte(value)});
		}

		void write_memory_(RGB_666_COLOR color)
		{
			send_data({color.red(), color.green(), color.blue()});
		}

		// Pin to control data Vs command sending through SPI
		gpio::FAST_PIN<DC> dc_{gpio::PinMode::OUTPUT};
		gpio::FAST_PIN<RST> rst_{gpio::PinMode::OUTPUT, true};

		// counter of RGB444 pixels sent, used to properly combine pixel pairs in transmission
		uint8_t pixel_index_ = 0;
		uint16_t first_444_color_{};
	};

	/** 
	 * Alias type for ST7735 with 128x160 resolution.
	 * @sa ST7735
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST, 
		ColorModel COLOR_MODEL, Orientation ORIENTATION, bool RGB_BGR>
	using ST7735_128X160 = ST7735<SCE, DC, RST, COLOR_MODEL, 
		ORIENTATION, Resolution::RESOLUTION_128X160, RGB_BGR>;

	/** 
	 * Alias type for ARDUINO IDE (128x160 resolution).
	 * @sa ST7735
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST, 
		ColorModel COLOR_MODEL, Orientation ORIENTATION>
	using ARDUINO_IDE = ST7735<SCE, DC, RST, COLOR_MODEL, ORIENTATION, 
		Resolution::RESOLUTION_128X160, true>;

	/** 
	 * Alias type for ST7735 with 132x162 resolution.
	 * @sa ST7735
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST, 
		ColorModel COLOR_MODEL, Orientation ORIENTATION, bool RGB_BGR>
	using ST7735_132X162 = ST7735<SCE, DC, RST, COLOR_MODEL, 
		ORIENTATION, Resolution::RESOLUTION_132X162, RGB_BGR>;

	/** 
	 * Alias type for ILI9163 with 120x160 resolution.
	 * @sa ST7735
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST, 
		ColorModel COLOR_MODEL, Orientation ORIENTATION, bool RGB_BGR>
	using ILI9163_120X160 = ST7735<SCE, DC, RST, COLOR_MODEL, 
		ORIENTATION, Resolution::RESOLUTION_120X160, RGB_BGR>;

	/** 
	 * Alias type for ILI9163 with 128x128 resolution.
	 * @sa ST7735
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST, 
		ColorModel COLOR_MODEL, Orientation ORIENTATION, bool RGB_BGR>
	using ILI9163_128X128 = ST7735<SCE, DC, RST, COLOR_MODEL, 
		ORIENTATION, Resolution::RESOLUTION_128X128, RGB_BGR>;

	/** 
	 * Alias type for ILI9163 with 130x130 resolution.
	 * @sa ST7735
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST, 
		ColorModel COLOR_MODEL, Orientation ORIENTATION, bool RGB_BGR>
	using ILI9163_130X130 = ST7735<SCE, DC, RST, COLOR_MODEL, 
		ORIENTATION, Resolution::RESOLUTION_130X130, RGB_BGR>;

	/** 
	 * Alias type for ILI9163 with 132x132 resolution.
	 * @sa ST7735
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST, 
		ColorModel COLOR_MODEL, Orientation ORIENTATION, bool RGB_BGR>
	using ILI9163_132X132 = ST7735<SCE, DC, RST, COLOR_MODEL, 
		ORIENTATION, Resolution::RESOLUTION_132X132, RGB_BGR>;
}

// Add specific traits for ST7735 display chip
namespace devices::display
{
	/// @cond notdocumented
	using st7735::ST7735_128X160, st7735::ST7735_132X162;
	using st7735::ILI9163_120X160, st7735::ILI9163_128X128, st7735::ILI9163_130X130, st7735::ILI9163_132X132;
	using st7735::ColorModel;
	using st7735::RGB_444_COLOR, st7735::RGB_565_COLOR, st7735::RGB_666_COLOR;
	using st7735::Orientation;

	// Temporary macros to define traits for plenty of different devices
	#define ST7735_TRAIT(TYPE, COLOR, ORIENT, WIDTH, HEIGHT)											\
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST, bool RGB_BGR>			\
	struct DisplayDeviceTrait<TYPE <SCE, DC, RST, ColorModel::COLOR, Orientation::ORIENT, RGB_BGR>> : 	\
		DisplayDeviceTrait_impl<COLOR##_COLOR, WIDTH, HEIGHT, false, false> {};

	#define ST7735_TRAITS(TYPE, WIDTH, HEIGHT)						\
	ST7735_TRAIT(TYPE, RGB_444, LANDSCAPE, WIDTH, HEIGHT)			\
	ST7735_TRAIT(TYPE, RGB_565, LANDSCAPE, WIDTH, HEIGHT)			\
	ST7735_TRAIT(TYPE, RGB_666, LANDSCAPE, WIDTH, HEIGHT)			\
	ST7735_TRAIT(TYPE, RGB_444, REVERSE_LANDSCAPE, WIDTH, HEIGHT)	\
	ST7735_TRAIT(TYPE, RGB_565, REVERSE_LANDSCAPE, WIDTH, HEIGHT)	\
	ST7735_TRAIT(TYPE, RGB_666, REVERSE_LANDSCAPE, WIDTH, HEIGHT)	\
	ST7735_TRAIT(TYPE, RGB_444, PORTRAIT, HEIGHT, WIDTH)			\
	ST7735_TRAIT(TYPE, RGB_565, PORTRAIT, HEIGHT, WIDTH)			\
	ST7735_TRAIT(TYPE, RGB_666, PORTRAIT, HEIGHT, WIDTH)			\
	ST7735_TRAIT(TYPE, RGB_444, REVERSE_PORTRAIT, HEIGHT, WIDTH)	\
	ST7735_TRAIT(TYPE, RGB_565, REVERSE_PORTRAIT, HEIGHT, WIDTH)	\
	ST7735_TRAIT(TYPE, RGB_666, REVERSE_PORTRAIT, HEIGHT, WIDTH)

	// Traits for 128x160 resolution devices
	ST7735_TRAITS(ST7735_128X160, 160, 128)
	// Traits for 132x162 resolution devices
	ST7735_TRAITS(ST7735_132X162, 162, 132)
	// Traits for 120x160 resolution devices
	ST7735_TRAITS(ILI9163_120X160, 160, 120)
	// Traits for 128x128 resolution devices
	ST7735_TRAITS(ILI9163_128X128, 128, 128)
	// Traits for 130x130 resolution devices
	ST7735_TRAITS(ILI9163_130X130, 130, 130)
	// Traits for 132x132 resolution devices
	ST7735_TRAITS(ILI9163_132X132, 132, 132)
	
	#undef ST7735_TRAITS
	#undef ST7735_TRAIT
	/// @endcond
}

#endif /* ST7735_HH */
/// @endcond
