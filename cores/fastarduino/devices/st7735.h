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
 * TODO update doc
 * API to handle Nokia 5110 display through SPI interface (actually not really SPI 
 * as only MOSI, not MISO, pin is used for data transfer).
 * 
 * Note that ST7735 chip used in Nokia 5110 is powered at 3.3V and does not 
 * bear 5V voltage from pins of most Arduino. Hence, all signals from Arduino
 * output pins must be converted from 5V to 3.3V, for this you will need a level
 * converter:
 * - this may be CD74HC4050 chip (up to 6 pins with level conversion)
 * - or you may use one of those common MOSFET-based converters breakouts
 * - or you may build your own
 * 
 * Example wiring for Arduino UNO (with CD74HC4050):
 * 
 * - pin 11 (PB3,MOSI) --I>-- DN
 * - pin 13 (PB5, SCK) --I>-- SCLK
 * - pin 10 (PB2, SS)  --I>-- SCE
 * - pin 9 (PB1)       --I>-- D/C
 * - pin 8 (PB0)       --I>-- RST
 * - 3.3V         --[=330=]-- LED
 * - 3.3V              ------ 3.3V
 * - GND               ------ GND
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
		LANDSCAPE = bits::BV8(5),
		PORTRAIT = 0,
		REVERSE_LANDSCAPE = bits::BV8(5, 6, 7),
		REVERSE_PORTRAIT = bits::BV8(6, 7)
	};

	enum class ColorModel: uint8_t
	{
		RGB_444 = bits::BV8(1, 0),
		RGB_565 = bits::BV8(2, 0),
		RGB_666 = bits::BV8(2, 1)
	};

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
		constexpr RGB_565_COLOR(uint16_t rgb): rgb_{rgb} {}
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
	 * @warning This class shall be used along with `devices::display::Display` 
	 * template class. It cannot be instantiated on its own.
	 * 
	 * All public API in ST7735 is available through the encapsulating Display
	 * instance.
	 * 
	 * Once Display has been instantiated for ST7735 driver, you should call
	 * the following API before it can be used to display anything:
	 * TODO update
	 * 1. `reset()` device
	 * 2. `set_display_bias()` to relevant value
	 * 3. `set_display_contrast()` to relevant value
	 * 4. `power_up()` device
	 * 5. `set_color()` to define the pixel color (black or white) to use in all
	 * subsequent drawing primitives
	 * 6. `set_mode()` if you want to use a specific drawing mode, other than 
	 * default `Mode::COPY`.
	 * 7. `set_font()` if you intend to display text
	 *
	 * @tparam SCE the output pin used for Chip Selection of the ST7735 chip on
	 * the SPI bus.
	 * @tparam DC the output pin used to select Data (high) or Command (low) mode 
	 * of ST7735 chip.
	 * @tparam RST the output pin used to reset ILI9361 chip
	 * 
	 * @sa Display
	 */
	//TODO try 8MHz SPI
	//TODO add template args for color model, resolution, orientation
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST>
	class ST7735 : public spi::SPIDevice<SCE, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(4'000'000UL)>
	{
	private:
		using TRAITS = DisplayDeviceTrait<ST7735<SCE, DC, RST>>;
		using COLOR = typename TRAITS::COLOR;
		static constexpr uint8_t WIDTH = TRAITS::WIDTH;
		static constexpr uint8_t HEIGHT = TRAITS::HEIGHT;
		using DRAW_CONTEXT = DrawContext<RGB_565_COLOR, false>;

	public:
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

	protected:
		/// @cond notdocumented
		ST7735()
		{
			//TODO minimum init code?
			// include color setting
		}

		// NOTE Coordinates must have been first verified by caller
		bool set_pixel(uint8_t x, uint8_t y, const DRAW_CONTEXT& context)
		{
			// // Convert to (r,c)
			// const uint8_t c = x;
			// const uint8_t r = y / ROW_HEIGHT;
			// const uint8_t offset = y % ROW_HEIGHT;
			// uint8_t mask = bits::BV8(offset);
			// // Get pointer to pixel byte
			// uint8_t* pix_column = get_display(r, c);
			// // Evaluate final pixel color based on color_ and mode_
			// const bool current = (*pix_column & mask);
			// const bool dest = context.draw_mode().pixel_op(current);

			// // Based on calculated color, set pixel
			// if (dest)
			// {
			// 	if (current) return false;
			// 	*pix_column |= mask;
			// }
			// else
			// {
			// 	if (!current) return false;
			// 	*pix_column &= uint8_t(~mask);
			// }
			return true;
		}

		bool is_valid_char_xy(UNUSED uint8_t x, UNUSED uint8_t y)
		{
			return true;
		}

		// NOTE Coordinates must have been first verified by caller
		uint8_t write_char(uint8_t x, uint8_t y, uint16_t glyph_ref, const DRAW_CONTEXT& context)
		{
			// // Check column and row not out of range for characters!
			// const uint8_t width = context.font().width();
			// uint8_t row = y / ROW_HEIGHT;
			// const uint8_t col = x;
			// bool add_interchar_space = ((col + width + 1) < WIDTH);

			// uint8_t glyph_index  = 0;
			// for (uint8_t glyph_row = 0; glyph_row < context.font().glyph_rows(); ++glyph_row)
			// {
			// 	// Get pointer to first byte in row to write in display buffer
			// 	uint8_t* display_ptr = get_display(row, col);

			// 	for (uint8_t i = 0; i <= width; ++i)
			// 	{
			// 		const bool space_column = (i == width);
			// 		uint8_t pixel_bar = 0x00;
			// 		if (!space_column)
			// 			pixel_bar = context.font().get_char_glyph_byte(glyph_ref, glyph_index);
			// 		if ((!space_column) || add_interchar_space)
			// 			*display_ptr = context.draw_mode().bw_pixels_op(pixel_bar, *display_ptr);
			// 		++display_ptr;
			// 		++glyph_index;
			// 	}
			// 	++row;
			// }

			// // Return actual width writtent to display
			// return width + (add_interchar_space ? 1 : 0);
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
