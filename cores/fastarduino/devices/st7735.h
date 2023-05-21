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
 * - pin 11 (PB3,MOSI) --I>-- MOSI
 * - pin 13 (PB5, SCK) --I>-- SCLK
 * - pin 10 (PB2, SS)  --I>-- SCE
 * - pin 9 (PB1)       --I>-- D/C
 * - pin 8 (PB0)       --I>-- RST
 * - 3.3V         --[=330=]-- LED
 * - 5V                ------ 5VV
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
	//TODO DOC
	//TODO rework to avoid this strange embedding of namespaces for the sake of traits definition
	namespace st7735
	{
		/// @cond notdocumented
		// Forward declaration to allow traits definition
		template<board::DigitalPin, board::DigitalPin, board::DigitalPin> class ST7735; 
		class RGB_565_COLOR;
		/// @endcond
	}
	/// @cond notdocumented
	// Traits for ST7735 display
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST>
	struct DisplayDeviceTrait<st7735::ST7735<SCE, DC, RST>> : 
		DisplayDeviceTrait_impl<st7735::RGB_565_COLOR, 160, 128, false, false> {};
	/// @endcond
}
namespace devices::display::st7735
{
	enum class Orientation: uint8_t
	{
		LANDSCAPE = bits::BV8(5, 7),
		PORTRAIT = 0,
		REVERSE_LANDSCAPE = bits::BV8(5, 6),
		REVERSE_PORTRAIT = bits::BV8(6, 7)
	};

	enum class ColorModel: uint8_t
	{
		RGB_444 = bits::BV8(1, 0),
		RGB_565 = bits::BV8(2, 0),
		RGB_666 = bits::BV8(2, 1)
	};

	//TODO more resolutions supported by ILI9163 "brother" chip
	enum class Resolution: uint8_t
	{
		RESOLUTION_132X162,
		RESOLUTION_128X160,
	};

	//TODO Avoid bit fields!
	// struct RGB_444_COLOR
	// {
	// 	uint8_t red		: 4;
	// 	uint8_t green	: 4;
	// 	uint8_t blue	: 4;
	// };

	class RGB_565_COLOR
	{
	public:
		//TODO DOC that only MSB are used (5,6,5) for each primary color
		constexpr RGB_565_COLOR(uint16_t rgb = 0): rgb_{rgb} {}
		constexpr RGB_565_COLOR(uint8_t red, uint8_t green, uint8_t blue):
			rgb_{rgb_to_color(red, green, blue)} {}

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

	//TODO Avoid bit fields!
	// struct RGB_666_COLOR
	// {
	// 	constexpr RGB_666_COLOR(uint8_t red, uint8_t green, uint8_t blue):
	// 		red(red), green(green), blue(blue) {}

	// 	uint8_t red		: 6;
	// 	uint8_t			: 2;
	// 	uint8_t green	: 6;
	// 	uint8_t			: 2;
	// 	uint8_t blue	: 6;
	// 	uint8_t			: 2;
	// };

	enum class Gamma: uint8_t
	{
		GC0 = 0x01,
		GC1 = 0x02,
		GC2 = 0x04,
		GC3 = 0x08
	};

	//TODO actually ILI9163 is near ST7735 (maybe more powerful, in terms or resolutions and more)
	/**
	 * SPI device driver for ST7735 display chip.
	 * 
	 * This driver offers various API to:
	 * - display characters or strings
	 * - display pixels
	 * All drawing API is applied directly on the device (no raster buffer).
	 * 
	 * 
	 * 
	 * @warning This class shall be used along with `devices::display::Display` 
	 * template class. It cannot be instantiated on its own.
	 * 
	 * All public API in ST7735 is available through the encapsulating Display
	 * instance.
	 * 
	 * Once Display has been instantiated for ST7735 driver, you should call
	 * the following API before it can be used to display anything:
	 * TODO update
	 * 1. `hard_reset()`
	 * 2. `soft_reset()`
	 * 3. `sleep_out()`
	 * 4. `display_on()`
	 *
	 * @tparam SCE the output pin used for Chip Selection of the ST7735 chip on
	 * the SPI bus.
	 * @tparam DC the output pin used to select Data (high) or Command (low) mode 
	 * of ST7735 chip.
	 * @tparam RST the output pin used to reset ST7735 chip
	 * 
	 * @sa Display
	 */
	//TODO add template args for color model, resolution, orientation
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST>
	class ST7735 : public spi::SPIDevice<SCE, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(8'000'000UL)>
	{
	private:
		using TRAITS = DisplayDeviceTrait<ST7735<SCE, DC, RST>>;
		using COLOR = typename TRAITS::COLOR;
		static constexpr uint8_t WIDTH = TRAITS::WIDTH;
		static constexpr uint8_t HEIGHT = TRAITS::HEIGHT;
		using DRAW_CONTEXT = DrawContext<RGB_565_COLOR, false>;

	public:
		void begin(bool force_hard_reset = false)
		{
			if (force_hard_reset)
				hard_reset();
			soft_reset();
			sleep_out();
			set_color_model(ColorModel::RGB_565);
			set_orientation(Orientation::LANDSCAPE);
			set_column_address(0, WIDTH - 1);
			set_row_address(0, HEIGHT - 1);
			display_on();
		}

		void hard_reset()
		{
			// Reset device according to datasheet
			rst_.clear();
			time::delay_us(10);
			rst_.set();
			time::delay_ms(120);
		}

		void soft_reset()
		{
			send_command(CMD_SOFT_RESET);
			time::delay_ms(50);
		}

		void sleep_in()
		{
			send_command(CMD_SLEEP_IN);
			time::delay_ms(120);
		}
		void sleep_out()
		{
			send_command(CMD_SLEEP_OUT);
			time::delay_ms(120);
		}

		void idle_on()
		{
			send_command(CMD_IDLE_ON);
		}
		void idle_off()
		{
			send_command(CMD_IDLE_OFF);
		}

		void partial_mode(uint16_t start_row, uint16_t end_row)
		{
			//TODO Check validity of args!
			send_command(CMD_PARTIAL_AREA, {
				utils::high_byte(start_row), utils::low_byte(start_row),
				utils::high_byte(end_row), utils::low_byte(end_row)});
			send_command(CMD_PARTIAL_MODE);
		}
		void normal_mode()
		{
			send_command(CMD_NORMAL_MODE);
		}

		void invert_on()
		{
			send_command(CMD_INVERT_ON);
		}
		void invert_off()
		{
			send_command(CMD_INVERT_OFF);
		}

		void display_on()
		{
			send_command(CMD_DISPLAY_ON);
			time::delay_ms(120);
		}
		void display_off()
		{
			send_command(CMD_DISPLAY_OFF);
			time::delay_ms(120);
		}

		//NOTE most breakouts do not expose pin TE hence the followin method may not work
		void tear_effect_on(bool vertical_blanking_only)
		{
			const uint8_t telom = (vertical_blanking_only ? 0 : 1);
			send_command(CMD_TEAR_ON, telom);
		}
		//NOTE most breakouts do not expose pin TE hence the followin method may not work
		void tear_effect_off()
		{
			send_command(CMD_TEAR_OFF);
		}

		void set_gamma(Gamma gamma_curve)
		{
			send_command(CMD_SET_GAMMA, uint8_t(gamma_curve));
		}

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
		ST7735()
		{
			//TODO minimum init code?
			// include color setting, orientation...
		}

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
			const bool add_interchar_space = ((x + width + 1) < WIDTH);
			const COLOR fg = context.foreground();
			const COLOR bg = context.background();

			set_column_address(x, x + width - 1 + (add_interchar_space? 1 : 0));
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
					// add interspace if needed
					if (add_interchar_space)
					{
						write_memory(bg);
					}
				}
			}
			stop_memory_write();

			// Return actual width written to display
			return width + (add_interchar_space ? 1 : 0);
		}

		// No invalid region, so no update() operation in effect
		void update(UNUSED uint8_t x1, UNUSED uint8_t y1, UNUSED uint8_t x2, UNUSED uint8_t y2) {}
		/// @endcond

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
		}
		void write_memory(COLOR color)
		{
			//TODO 3 types for COLOR will require transform or not
			// RGB444 Not supported yet TODO
			if (sizeof(COLOR) == 2)
			{
				// RGB565
				uint16_t value = color.color();
				send_data({utils::high_byte(value), utils::low_byte(value)});
			}
			else
			{
				// RGB666
				// send_data({color.red, color.green, color.blue});
			}
		}
		void stop_memory_write()
		{
			send_command(CMD_NOP);
		}

	private:
		// Value to add to MADCTL (CMD_SET_ADDRESS_MODE) for Arduino LCD
		// NOTE: this flag depends on the LCD screen
		//TODO that should be part of a trait!
		static constexpr uint8_t RGB_ORDER = 0x08;
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

		// Pin to control data Vs command sending through SPI
		gpio::FAST_PIN<DC> dc_{gpio::PinMode::OUTPUT};
		gpio::FAST_PIN<RST> rst_{gpio::PinMode::OUTPUT, true};
	};
}

#endif /* ST7735_HH */
/// @endcond
