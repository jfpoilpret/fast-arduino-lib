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
 * API to handle Nokia 5110 display through SPI interface (actually not really SPI 
 * as only MOSI, not MISO, pin is used for data transfer).
 * 
 * Connection diagram:
 * TODO show 4050 usage
 *                 W25Q80BV
 *                +----U----+
 * (/CS)--------1-|/CS   VCC|-8---------(VCC)
 * (MISO)-------2-|DO  /HOLD|-7--VVVV---(VCC)
 *            --3-|/WP   CLK|-6---------(CLK)
 * (GND)--------4-|GND    DI|-5---------(MOSI)
 *                +---------+
 * 
 * Note that the PCD8544 chip used for Nokia 5110 display is using CMOS and works on 
 * Vcc = 3.3V (not 5V), thus any inputs shall be limited to 3.3V.
 * The only safe ways to do that are to use level converters (MOSFET based) or 
 * possibly use the IC 74HC4050.
 * 
 * TODO explain extra pins not handled by this device
 */
#ifndef LC5110_HH
#define LC5110_HH

#include <string.h>

#include "../bits.h"
#include "../flash.h"
#include "../gpio.h"
#include "../spi.h"
#include "../time.h"
#include "../utilities.h"

//TODO Future design (V2 only, needed when support for other devices is implemented):
// - GraphicsDrawing<LCDDevice> (or better name)
//		- 2D draw API
//		- Text API
// - LCDDevice
//		- basic primitives (pixel, char, bitmap) used by GraphicsDrawing
//		- general primitives need for display (update, invalidate, erase...)
//		- device-specific primitives
// Use traits for devices specific constraints (size, color, coordinates)
// Question:
// - who shall hold the pixmap buffer?

//TODO add graphics pixels API

//TODO Add image API (pixmap)
//		- format?
//		- converters?

//TODO better use of spi start/end transaction (do once only)

//TODO infer generic font support: different widths, different characters sets (range)

//TODO API DOC

// Optional improvements:
//TODO define specific ostream for display (is that even possible)?
namespace devices
{
	namespace display
	{
	}
}

namespace devices::display
{
	// Coordinates systems conventions (variable naming)
	// - (x,y) coordinates of one pixel in the LCD matrix
	// - (r,c) coordinates of an 8-pixel vertical bar in the LCD matrix
	// - (row, column) coordinates of one character (8x5 pixels + 1 spacing) in the LCD matrix

	// Screen update is not automatic! You must call update() once you have changed display bitmap content

	/**
	 * SPI device driver for Nokia 5110 display chip.
	 * 
	 * This driver offers various API to:
	 * - display characters or strings
	 * - display pixels
	 * All drawing API work on a pixel buffer in memory, never directly on the LCD 
	 * device. Buffer is copied to the LCD device through `update()` calls.
	 * 
	 * TODO
	 * Note that PCD8544 chip used in Nokia 5110 is powered at 3.3V and does not 
	 * bear 5V voltage of pins in most Arduino. Hence, all signals from Arduino
	 * output pins must be covnerted from 5V to 3.3V, for this you will need a level
	 * converter:
	 * - this may be 4050 chip (up to 6 pins)
	 * - or you may use of those common MOSFET-based converters breakouts
	 * - or you may build your own
	 * 
	 * Example wiring for Arduino UNO:
	 * 
	 * - pin 11 (PB3,MOSI) --I>-- DN
	 * - pin 13 (PB5, SCK) --I>-- SCLK
	 * - pin 10 (PB2, SS)  --I>-- SCE
	 * - pin 9 (PB1)       --I>-- D/C
	 * - pin 8 (PB0)       --I>-- RST (*)
	 * - 3.3V         --[=330=]-- LED (**)
	 * - 3.3V              ------ 3.3V
	 * - GND               ------ GND
	 * 
	 * TODO
	 * 
	 * @tparam SCE the output pin used for Chip Selection of the PCD8544 chip on
	 * the SPI bus.
	 * @tparam DC the output pin used to select Data (high) or Command (low) mode 
	 * of PCD8544 chip.
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST> class LCD5110 : 
		public spi::SPIDevice<SCE, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(4'000'000UL)>
	{
	public:
		static constexpr uint8_t FONT_WIDTH = 5U;
		static constexpr uint8_t FONT_HEIGHT = 8U;
		static constexpr uint8_t FONT_1ST_CHAR = 0x20;
		static constexpr uint8_t FONT_TOTAL_CHARS = 0x80 - FONT_1ST_CHAR;
		
		/**
		 * Create a new device driver for a Nokia 5110 display chip.
		 * TODO high_impedence_reset if no level converter available (pullup 3.3V instead)
		 */
		LCD5110(bool high_impedence_reset)
		{
			// Reset device according to datasheet
			gpio::FastPinType<RST>::set_mode(gpio::PinMode::OUTPUT, false);
			time::delay_us(1);
			if (high_impedence_reset)
				gpio::FastPinType<RST>::set_mode(gpio::PinMode::INPUT);
			else
				gpio::FastPinType<RST>::set();
			memset(display_, 0, sizeof(display_));
			set_display_bias(DEFAULT_BIAS);
			set_display_contrast(DEFAULT_VOP);
		}

		//TODO temp control API?
		// void set_temperature_control();

		void set_display_bias(uint8_t bias)
		{
			if (bias > MAX_BIAS) bias = MAX_BIAS;
			send_command(FUNCTION_SET_MASK | FUNCTION_SET_EXTENDED);
			send_command(EXTENDED_SET_BIAS | bias);
			send_command(FUNCTION_SET_MASK);
		}

		void set_display_contrast(uint8_t contrast)
		{
			if (contrast > MAX_VOP) contrast = MAX_VOP;
			send_command(FUNCTION_SET_MASK | FUNCTION_SET_EXTENDED);
			send_command(EXTENDED_SET_VOP | contrast);
			send_command(FUNCTION_SET_MASK);
		}

		void power_down()
		{
			send_command(FUNCTION_SET_MASK | FUNCTION_SET_POWER_DOWN);
		}

		void power_up()
		{
			send_command(FUNCTION_SET_MASK);
		}

		void blank()
		{
			send_command(DISPLAY_CONTROL_MASK | DISPLAY_CONTROL_BLANK);
		}

		void full()
		{
			send_command(DISPLAY_CONTROL_MASK | DISPLAY_CONTROL_FULL);
		}

		void invert()
		{
			send_command(DISPLAY_CONTROL_MASK | DISPLAY_CONTROL_INVERSE);
		}

		void normal()
		{
			send_command(DISPLAY_CONTROL_MASK | DISPLAY_CONTROL_NORMAL);
		}

		//TODO API
		// Fonts must be 5x8 pixels and must be stored in Flash
		//TODO define conditional fonts in optional fonts includes
		//TODO maybe allow more than 96 characters in table?
		void set_font(const uint8_t font[][FONT_WIDTH])
		{
			font_ = (const uint8_t*) font;
		}

		void write_char(uint8_t row, uint8_t column, char value)
		{
			// Check column and row not out of range for characters!
			if (!is_valid_row_column(row, column)) return;

			// Load pixmap for `value` character
			uint8_t pixmap[FONT_WIDTH];
			if (get_char_pixmap(value, pixmap) == nullptr)
				return;

			// Convert coordinates for display matrix writing
			uint8_t r, c;
			convert_row(row, r);
			convert_column(column, c);
			uint8_t* display_ptr = get_display(r, c);

			for (uint8_t i = 0; i < FONT_WIDTH; ++i)
				*display_ptr++ = pixmap[i];
			invalidate(r, c, r, c + FONT_WIDTH + 1);
		}

		//TODO these functions could be externalized to a GraphicsDrawing class!
		//NOTE: there is no auto LF in this function!
		void write_string(uint8_t row, uint8_t column, const char* content)
		{
			while (*content)
				write_char(row, column++, *content++);
		}

		void write_string(uint8_t row, uint8_t column, const flash::FlashStorage* content)
		{
			uint16_t address = (uint16_t) content;
			while (char value = pgm_read_byte(address++))
				write_char(row, column++, value);
		}

		void set_bitmap()
		{
			//TODO
		}

		void erase()
		{
			memset(display_, 0, sizeof(display_));
			invalidate_all();
		}

		void set_pixel(uint8_t x, uint8_t y)
		{
			set_pixel(x, y, true);
		}

		void clear_pixel(uint8_t x, uint8_t y)
		{
			set_pixel(x, y, false);
		}

		void set_pixel(uint8_t x, uint8_t y, bool pixel)
		{
			// Check coordinates are OK
			if (!is_valid_xy(x, y)) return;
			// Convert to (r,c)
			uint8_t r, c, offset;
			convert_x(x, c);
			convert_y(y, r, offset);
			uint8_t mask = bits::BV8(offset);
			// Get pointer to pixel byte
			uint8_t* pix_column = get_display(r, c);
			// Check if pixel is already same as pixel, force pixel if needed
			if (pixel)
			{
				if (*pix_column & mask) return;
				*pix_column |= mask;
			}
			else
			{
				if (!(*pix_column & mask)) return;
				*pix_column &= ~mask;
			}
			invalidate(r, c, r, c);
		}

		// Copy invalidated rectangle of display map onto the device
		void update()
		{
			if (need_update_)
			{
				const uint8_t size = (c2_ - c1_ + 1);
				for (uint8_t r = r1_; r <= r2_; ++r)
				{
					set_rc(r, c1_);
					dc_.set();
					this->start_transfer();
					const uint8_t* display = get_display(r, c1_);
					this->transfer(display, size);
					this->end_transfer();
				}
				need_update_ = false;
			}
		}

	private:
		static constexpr uint8_t ROWS = 48;
		static constexpr uint8_t COLS = 84;
		static constexpr uint8_t ROW_HEIGHT = 8;

		static constexpr uint8_t FUNCTION_SET_MASK = bits::BV8(5);
		static constexpr uint8_t FUNCTION_SET_POWER_DOWN = bits::BV8(2);
		static constexpr uint8_t FUNCTION_SET_EXTENDED = bits::BV8(0);

		static constexpr uint8_t EXTENDED_SET_BIAS = bits::BV8(4);
		static constexpr uint8_t EXTENDED_SET_VOP = bits::BV8(7);

		static constexpr uint8_t DISPLAY_CONTROL_MASK = bits::BV8(3);
		static constexpr uint8_t DISPLAY_CONTROL_BLANK = 0;
		static constexpr uint8_t DISPLAY_CONTROL_NORMAL = bits::BV8(2);
		static constexpr uint8_t DISPLAY_CONTROL_FULL = bits::BV8(0);
		static constexpr uint8_t DISPLAY_CONTROL_INVERSE = bits::BV8(0, 2);

		static constexpr uint8_t SET_ROW_ADDRESS = bits::BV8(6);
		static constexpr uint8_t SET_COL_ADDRESS = bits::BV8(7);

		static constexpr uint8_t MAX_BIAS = 0x07;
		static constexpr uint8_t DEFAULT_BIAS = 0x04;
		static constexpr uint8_t MAX_VOP = 0x7F;
		static constexpr uint8_t DEFAULT_VOP = 40;
		
		void invalidate(uint8_t r1, uint8_t c1, uint8_t r2, uint8_t c2)
		{
			if (!need_update_)
			{
				r1_ = r1;
				c1_ = c1;
				r2_ = r2;
				c2_ = c2;
				need_update_ = true;
			}
			else
			{
				if (r1 < r1_) r1_ = r1;
				if (r2 > r2_) r2_ = r2;
				if (c1 < c1_) c1_ = c1;
				if (c2 > c2_) c2_ = c2;
			}
		}

		void invalidate_all()
		{
			r1_ = 0;
			c1_ = 0;
			r2_ = ROWS / ROW_HEIGHT;
			c2_ = COLS;
			need_update_ = true;
		}

		void send_command(uint8_t command)
		{
			dc_.clear();
			this->start_transfer();
			this->transfer(command);
			this->end_transfer();
		}

		// Functions to check and convert coordinates
		// convert (x,y) -> (r,c,offset)
		static void convert_x(uint8_t x, uint8_t& c)
		{
			c = x;
		}
		static void convert_y(uint8_t y, uint8_t& r, uint8_t& offset)
		{
			r = y / ROW_HEIGHT;
			offset = y % ROW_HEIGHT;
		}

		// convert (row,column) -> (r,c)
		void convert_row(uint8_t row, uint8_t& r) const
		{
			r = row * FONT_HEIGHT / ROW_HEIGHT;
		}
		void convert_column(uint8_t column, uint8_t& c) const
		{
			c = column * (FONT_WIDTH + 1);
		}
		
		bool is_valid_row_column(uint8_t row, uint8_t col) const
		{
			if (row > (ROWS / FONT_HEIGHT)) return false;
			if (col > (COLS / (FONT_WIDTH + 1))) return false;
			return true;
		}

		static bool is_valid_rc(uint8_t r, uint8_t c)
		{
			if (r > (ROWS / ROW_HEIGHT)) return false;
			if (c > COLS) return false;
			return true;
		}

		static bool is_valid_xy(uint8_t x, uint8_t y)
		{
			if (x > COLS) return false;
			if (y > ROWS) return false;
			return true;
		}

		//TODO review utility and possibly code and naming guidelines
		void set_rc(uint8_t r, uint8_t c)
		{
			if (r > (ROWS / ROW_HEIGHT)) r = 0;
			if (c > COLS) c = 0;
			dc_.clear();
			this->start_transfer();
			this->transfer(r | SET_ROW_ADDRESS);
			this->transfer(c | SET_COL_ADDRESS);
			this->end_transfer();
		}

		// Fill a pixmap array with a glyph for the requested character from current font
		// Return poijter to the glyph or nulptr if required character does not exist in font
		uint8_t* get_char_pixmap(char value, uint8_t pixmap[FONT_WIDTH]) const
		{
			if ((value < FONT_1ST_CHAR) || (value >= FONT_1ST_CHAR + FONT_TOTAL_CHARS))
				return nullptr;
			// Find first byte (vertical) of character in font_
			uint16_t index  = (uint8_t(value) - FONT_1ST_CHAR) * FONT_WIDTH;
			const uint8_t* ptr = &font_[index];
			return flash::read_flash(uint16_t(ptr), pixmap, FONT_WIDTH);
		}

		// Get a pointer to display byte at (r,c) coordinates, or nulptr if (r,c) is invalid
		uint8_t* get_display(uint8_t r, uint8_t c)
		{
			if (r >= ROWS / 8) return nullptr;
			if (c >= COLS) return nullptr;
			return &display_[r * COLS + c];
		}
		const uint8_t* get_display(uint8_t r, uint8_t c) const
		{
			if (r >= ROWS / 8) return nullptr;
			if (c >= COLS) return nullptr;
			return &display_[r * COLS + c];
		}

		// Font used in characters display
		const uint8_t* font_ = nullptr;

		// Display map (copy of chip display map)
		// Format:	R1C1 R1C2 ... R1Cn
		//			R2C1 R2C2 ... R2Cn
		//			...
		//			RpC1 RpC2 ... RpCn
		uint8_t display_[ROWS * COLS / ROW_HEIGHT];

		// Minimal (r,c) rectangle to update
		bool need_update_ = false;
		uint8_t r1_, c1_, r2_, c2_;

		// Pin to control data Vs command sending through SPI
		gpio::FAST_PIN<DC> dc_{gpio::PinMode::OUTPUT};
	};
}

#endif /* LC5110_HH */
/// @endcond
