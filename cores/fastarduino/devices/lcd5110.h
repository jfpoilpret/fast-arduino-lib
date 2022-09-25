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
 * TODO
 *                 W25Q80BV
 *                +----U----+
 * (/CS)--------1-|/CS   VCC|-8---------(VCC)
 * (MISO)-------2-|DO  /HOLD|-7--VVVV---(VCC)
 *            --3-|/WP   CLK|-6---------(CLK)
 * (GND)--------4-|GND    DI|-5---------(MOSI)
 *                +---------+
 * 
 * Note that the PCD8544 chip used for Nokia 5110 display is using CMOS and works on 
 * Vcc = 3.3V (not 5V), thus any inputs shall be limited to 3.3V,
 * We can do that either with level converters (expensive solution) or directly with
 * resistors in series between AVR digital output pins and PCD8544 input pins.
 * 
 * TODO resistors values to use for each pin.
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

namespace devices
{
	/**
	 * TODO namespace doc
	 * 
	 */
	namespace display
	{
	}
}
namespace devices::display
{
	/**
	 * SPI device driver for Nokia 5110 display chip.
	 * @tparam SCE the output pin used for Chip Selection of the PCD8544 chip on
	 * the SPI bus.
	 * @tparam DC the output pin used to select Data (high) or Command (low) mode 
	 * of PCD8544 chip.
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC> class LCD5110 : 
		public spi::SPIDevice<SCE, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(4'000'000UL)>
	{
	public:
		static constexpr uint8_t FONT_WIDTH = 5U;
		static constexpr uint8_t FONT_HEIGHT = 8U;
		static constexpr uint8_t FONT_1ST_CHAR = 0x20;
		static constexpr uint8_t FONT_TOTAL_CHARS = 0x80 - FONT_1ST_CHAR;
		
		/**
		 * Create a new device driver for a Nokia 5110 display chip.
		 */
		LCD5110()
		{
			memset(display_, 0, sizeof(display_));
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

		// The API work directly on device
		void write_char(uint8_t row, uint8_t column, char value)
		{
			// Find first byte (vertical) of character in font_
			uint16_t index  = (uint8_t(value) - FONT_1ST_CHAR) * FONT_WIDTH;
			const uint8_t* ptr = &font_[index];
			uint8_t char_bytes[FONT_WIDTH];
			//TODO find better API of read_flash()!
			flash::read_flash(uint16_t(ptr), char_bytes, FONT_WIDTH);

			set_rc(row, column);
			dc_.set();
			this->start_transfer();
			for (uint8_t i = 0; i < FONT_WIDTH; ++i)
				this->transfer(char_bytes[i]);
			this->end_transfer();
		}

		//NOTE: there is no auto LF in this function!
		void write_string(uint8_t row, uint8_t column, const char* value)
		{
			while (*value)
			{
				write_char(row++, column++, *value);
				++value;
			}
		}

		void write_string(uint8_t row, uint8_t column, const flash::FlashStorage* value)
		{
			//TODO
		}

		void set_bitmap()
		{
			//TODO
		}

		void erase()
		{
			memset(display_, 0, sizeof(display_));
			update();
		}

		// These API work only on local display zone
		void set_pixel(uint8_t x, uint8_t y);
		void clear_pixel(uint8_t x, uint8_t y);

		// Copy all display map onto the device
		void update()
		{
			set_rc(0, 0);
			dc_.set();
			this->start_transfer();
			for (uint16_t i = 0; i < sizeof(display_); ++i)
			{
				this->transfer(display_[i]);
			}
			this->end_transfer();
		}

		// Shouldn't these 3 functions be set together?
		// void set_temperature_control();
		// void set_display_bias();
		// void set_op_voltage();

		// Drawing API?

	private:
		static constexpr uint8_t ROWS = 48;
		static constexpr uint8_t COLS = 84;

		static constexpr uint8_t FUNCTION_SET_MASK = bits::BV8(5);
		static constexpr uint8_t FUNCTION_SET_POWER_DOWN = bits::BV8(2);
		static constexpr uint8_t FUNCTION_SET_EXTENDED = bits::BV8(0);

		static constexpr uint8_t DISPLAY_CONTROL_MASK = bits::BV8(3);
		static constexpr uint8_t DISPLAY_CONTROL_BLANK = 0;
		static constexpr uint8_t DISPLAY_CONTROL_NORMAL = bits::BV8(2);
		static constexpr uint8_t DISPLAY_CONTROL_FULL = bits::BV8(0);
		static constexpr uint8_t DISPLAY_CONTROL_INVERSE = bits::BV8(0, 2);

		static constexpr uint8_t SET_Y_ADDRESS = bits::BV8(6);
		static constexpr uint8_t SET_X_ADDRESS = bits::BV8(7);
		
		void send_command(uint8_t command)
		{
			dc_.clear();
			this->start_transfer();
			this->transfer(command);
			this->end_transfer();
		}

		void set_rc(uint8_t row, uint8_t col)
		{
			if (row > (ROWS / 8)) row = 0;
			if (col > COLS) col = 0;
			dc_.clear();
			this->start_transfer();
			this->transfer(row | SET_Y_ADDRESS);
			this->transfer(col | SET_X_ADDRESS);
			this->end_transfer();
		}

		// Font used in characters display
		const uint8_t* font_ = nullptr;
		// Display map (copy of chip display map)
		uint8_t display_[ROWS * COLS / 8];

		//TODO Current mode? Current X/Y?...

		gpio::FAST_PIN<DC> dc_{gpio::PinMode::OUTPUT};
	};
}

#endif /* LC5110_HH */
/// @endcond
