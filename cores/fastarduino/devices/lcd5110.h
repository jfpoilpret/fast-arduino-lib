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
 */
#ifndef LC5110_HH
#define LC5110_HH

#include <string.h>

#include "../bits.h"
#include "../flash.h"
#include "../gpio.h"
#include "../spi.h"
#include "../time.h"
#include "display.h"
#include "font.h"

// General design rationale:
// - Device class contains only hardware stuff
// - no public drawing API in device class (even erase()?)
// - Everything else is performed by Display (common stuff!)

//TODO reorganize public/protected/private sections

//TODO add "screenshot" facility

//TODO Add image API (pixmap): generic? usable with files (from flash disk)
//		- format?
//		- converters?

//TODO better use of spi start/end transaction (do once only)

//TODO API DOC

// Optional improvements:
//TODO define specific ostream for display (is that even possible)?
namespace devices::display
{
	/**
	 * SPI device driver for Nokia 5110 display chip.
	 * 
	 * This driver offers various API to:
	 * - display characters or strings
	 * - display pixels
	 * All drawing API work on a pixel buffer in memory, never directly on the LCD 
	 * device. Buffer is copied to the LCD device through `update()` calls.
	 * 
	 * @warning This class shall be used along with `devices::display::Display` 
	 * template class. It cannot be instantiated on its own.
	 * 
	 * All public API in LCD5110 is available through the encapsulating Display
	 * instance.
	 * 
	 * Once Display has been instantiated for LCD5110 driver, you should call
	 * the following API before it can be used to display anything:
	 * 1. `reset()` device
	 * 2. `set_display_bias()` to relevant value
	 * 3. `set_display_contrast()` to relevant value
	 * 4. `power_up()` device
	 * 5. `set_color()` to define the pixel color (black or white) to use in all
	 * subsequent drawing primitives
	 * 6. `set_font()` if you intend to display text
	 *
	 * @warning For optimization reasons, text display can always occur at a `y`
	 * position that must be a multiple of 8, otherwise nothing will get drawn.
	 * 
	 * Note that PCD8544 chip used in Nokia 5110 is powered at 3.3V and does not 
	 * bear 5V voltage of pins in most Arduino. Hence, all signals from Arduino
	 * output pins must be covnerted from 5V to 3.3V, for this you will need a level
	 * converter:
	 * - this may be 4050 chip (up to 6 pins)
	 * - or you may use one of those common MOSFET-based converters breakouts
	 * - or you may build your own
	 * 
	 * Example wiring for Arduino UNO:
	 * 
	 * - pin 11 (PB3,MOSI) --I>-- DN
	 * - pin 13 (PB5, SCK) --I>-- SCLK
	 * - pin 10 (PB2, SS)  --I>-- SCE
	 * - pin 9 (PB1)       --I>-- D/C
	 * - pin 8 (PB0)       --I>-- RST
	 * - 3.3V         --[=330=]-- LED
	 * - 3.3V              ------ 3.3V
	 * - GND               ------ GND
	 * 
	 * @tparam SCE the output pin used for Chip Selection of the PCD8544 chip on
	 * the SPI bus.
	 * @tparam DC the output pin used to select Data (high) or Command (low) mode 
	 * of PCD8544 chip.
	 * 
	 * @sa Display
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST> class LCD5110 : 
		public spi::SPIDevice<SCE, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(4'000'000UL)>,
		public AbstractDisplayDevice<bool, true>
	{
	protected:
		using COORDINATE = uint8_t;
		using SIGNED_COORDINATE = int8_t;
		using INVALID_AREA = InvalidArea<COORDINATE>;

		static constexpr COORDINATE WIDTH = 84;
		static constexpr COORDINATE HEIGHT = 48;

	public:
		//TODO temp control API?
		// void set_temperature_control();

		void reset()
		{
			// Reset device according to datasheet
			gpio::FastPinType<RST>::set_mode(gpio::PinMode::OUTPUT, false);
			time::delay_us(1);
			gpio::FastPinType<RST>::set();
		}

		void set_display_bias(uint8_t bias = DEFAULT_BIAS)
		{
			if (bias > MAX_BIAS) bias = MAX_BIAS;
			send_command(FUNCTION_SET_MASK | FUNCTION_SET_EXTENDED);
			send_command(EXTENDED_SET_BIAS | bias);
			send_command(FUNCTION_SET_MASK);
		}

		void set_display_contrast(uint8_t contrast = DEFAULT_VOP)
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

	protected:
		LCD5110()
		{
			erase();
		}

		void erase()
		{
			memset(display_, 0, sizeof(display_));
		}

		// NOTE Coordinates must have been first verified by caller
		INVALID_AREA set_pixel(uint8_t x, uint8_t y)
		{
			// Convert to (r,c)
			const uint8_t c = x;
			const uint8_t r = y / ROW_HEIGHT;
			const uint8_t offset = y % ROW_HEIGHT;
			uint8_t mask = bits::BV8(offset);
			// Get pointer to pixel byte
			uint8_t* pix_column = get_display(r, c);
			// Check if pixel is already same as pixel, force pixel if needed
			if (color_)
			{
				if (*pix_column & mask) return INVALID_AREA::EMPTY;
				*pix_column |= mask;
			}
			else
			{
				if (!(*pix_column & mask)) return INVALID_AREA::EMPTY;
				*pix_column &= ~mask;
			}
			return INVALID_AREA{x, y, x, y};
		}

		// NOTE Coordinates must have been first verified by caller
		INVALID_AREA write_char(uint8_t x, uint8_t y, char value)
		{
			if (y % ROW_HEIGHT != 0) return INVALID_AREA::EMPTY;

			// Check column and row not out of range for characters!
			const uint8_t width = font_->width();
			const uint8_t row = y / ROW_HEIGHT;
			const uint8_t col = x;
			if ((col + width) > WIDTH) return INVALID_AREA::EMPTY;

			// Load pixmap for `value` character
			uint16_t glyph_ref = font_->get_char_glyph_ref(value);
			if (glyph_ref == 0)
				return INVALID_AREA::EMPTY;

			// Get pointer to first byte to write in display buffer
			uint8_t* display_ptr = get_display(row, col);

			for (uint8_t i = 0; i < width; ++i)
			{
				uint8_t pixel_bar = font_->get_char_glyph_byte(glyph_ref, i);
				if (color_)
					*display_ptr++ |= pixel_bar;
				else
					*display_ptr++ &= ~pixel_bar;
			}
			return INVALID_AREA{x, y, COORDINATE(x + width + 1), y};
		}

		void set_bitmap()
		{
			//TODO
		}

		// Copy invalidated rectangle of display map onto the device
		void update(const INVALID_AREA& area)
		{
			if (!area.empty)
			{
				const uint8_t size = (area.x2 - area.x1 + 1);
				const uint8_t xmin = area.x1;
				const uint8_t ymin = area.y1 / ROW_HEIGHT;
				const uint8_t ymax = area.y2 / ROW_HEIGHT;
				for (uint8_t y = ymin; y <= ymax; ++y)
				{
					set_rc(y, xmin);
					dc_.set();
					this->start_transfer();
					const uint8_t* display = get_display(y, xmin);
					this->transfer(display, size);
					this->end_transfer();
				}
			}
		}

	private:
		// Internal organization of Nokia pixmap buffer (1 byte = 8 vertical pixels)
		static constexpr uint8_t ROW_HEIGHT = 8;

		// Masks for Nokia commands
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

		// Masks for bias and operation voltage (i.e. contrast)
		static constexpr uint8_t MAX_BIAS = 0x07;
		static constexpr uint8_t DEFAULT_BIAS = 0x04;
		static constexpr uint8_t MAX_VOP = 0x7F;
		static constexpr uint8_t DEFAULT_VOP = 40;

		void send_command(uint8_t command)
		{
			dc_.clear();
			this->start_transfer();
			this->transfer(command);
			this->end_transfer();
		}

		void set_rc(uint8_t r, uint8_t c)
		{
			dc_.clear();
			this->start_transfer();
			this->transfer(r | SET_ROW_ADDRESS);
			this->transfer(c | SET_COL_ADDRESS);
			this->end_transfer();
		}

		// Get a pointer to display byte at (r,c) coordinates 
		// (r,c) must be valid coordinates in pixmap
		uint8_t* get_display(uint8_t r, uint8_t c)
		{
			return &display_[r * WIDTH + c];
		}
		const uint8_t* get_display(uint8_t r, uint8_t c) const
		{
			return &display_[r * WIDTH + c];
		}

		// Display map (copy of chip display map)
		// Format:	R1C1 R1C2 ... R1Cn
		//			R2C1 R2C2 ... R2Cn
		//			...
		//			RpC1 RpC2 ... RpCn
		uint8_t display_[HEIGHT * WIDTH / ROW_HEIGHT];

		// Pin to control data Vs command sending through SPI
		gpio::FAST_PIN<DC> dc_{gpio::PinMode::OUTPUT};
	};
}

#endif /* LC5110_HH */
/// @endcond
