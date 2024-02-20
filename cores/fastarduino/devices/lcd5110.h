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
 * API to handle Nokia 5110 display through SPI interface (actually not really SPI 
 * as only MOSI, not MISO, pin is used for data transfer).
 * 
 * Note that PCD8544 chip used in Nokia 5110 is powered at 3.3V and does not 
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

//FIXME strange display in Nokia1 is that due to a buffer overflow?
namespace devices::display
{
	/** Possible temperature coeeficient that can be set on Nokia5110 display. */
	enum class TemperatureCoefficient : uint8_t
	{
		/** TC0 Temperature coefficient 1mV/K  */
		TC0_1mV_K = 0x04,
		/** TC0 Temperature coefficient 9mV/K  */
		TC1_9mV_K = 0x05,
		/** TC0 Temperature coefficient 17mV/K  */
		TC2_17mV_K = 0x06,
		/** TC0 Temperature coefficient 24mV/K  */
		TC3_24mV_K = 0x07
	};

	/// @cond notdocumented
	// Forward declaration to allow traits definition
	template<board::DigitalPin, board::DigitalPin, board::DigitalPin> class LCD5110; 
	// Traits for Nokia displays
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST>
	struct DisplayDeviceTrait<LCD5110<SCE, DC, RST>> : 
		DisplayDeviceTrait_impl<bool, 84, 48, true, true> {};
	/// @endcond

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
	 * 6. `set_mode()` if you want to use a specific drawing mode, other than 
	 * default `Mode::COPY`.
	 * 7. `set_font()` if you intend to display text
	 *
	 * @warning For optimization reasons, text display can always occur at a `y`
	 * position that must be a multiple of 8, otherwise nothing will get drawn.
	 * 
	 * @tparam SCE the output pin used for Chip Selection of the PCD8544 chip on
	 * the SPI bus.
	 * @tparam DC the output pin used to select Data (high) or Command (low) mode 
	 * of PCD8544 chip.
	 * 
	 * @sa Display
	 */
	template<board::DigitalPin SCE, board::DigitalPin DC, board::DigitalPin RST> class LCD5110 : 
		public spi::SPIDevice<SCE, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(4'000'000UL)>
	{
	private:
		using TRAITS = DisplayDeviceTrait<LCD5110<SCE, DC, RST>>;
		static constexpr uint8_t WIDTH = TRAITS::WIDTH;
		static constexpr uint8_t HEIGHT = TRAITS::HEIGHT;
		using DRAW_CONTEXT = DrawContext<bool, true>;

	public:
		//DEBUG ONLY (TODO REMOVE AFTERWARDS)
		const uint8_t* raster() const
		{
			return display_;
		}

		/**
		 * Reset PCD8544 chip and Nokia 5110 display.
		 * This shall be called at launch time.
		 */
		void reset()
		{
			// Reset device according to datasheet
			gpio::FastPinType<RST>::set_mode(gpio::PinMode::OUTPUT, false);
			time::delay_us(1);
			gpio::FastPinType<RST>::set();
		}

		/**
		 * Set bias for Nokia5110 LCD display. Must be called before first use.
		 * 
		 * @param bias new bias value for the display, must be between `0` and `7` 
		 * included.
		 */
		void set_display_bias(uint8_t bias = DEFAULT_BIAS)
		{
			if (bias > MAX_BIAS) bias = MAX_BIAS;
			this->start_transfer();
			send_command_(FUNCTION_SET_MASK | FUNCTION_SET_EXTENDED);
			send_command_(EXTENDED_SET_BIAS | bias);
			send_command_(FUNCTION_SET_MASK);
			this->end_transfer();
		}

		/**
		 * Set contrast for Nokia5110 LCD display. Must be called before first use.
		 * 
		 * @param contrast new contrast value for the display, must be between `0`
		 * and `127` included.
		 */
		void set_display_contrast(uint8_t contrast = DEFAULT_VOP)
		{
			if (contrast > MAX_VOP) contrast = MAX_VOP;
			this->start_transfer();
			send_command_(FUNCTION_SET_MASK | FUNCTION_SET_EXTENDED);
			send_command_(EXTENDED_SET_VOP | contrast);
			send_command_(FUNCTION_SET_MASK);
			this->end_transfer();
		}

		/**
		 * Set temperature coefficient for Nokia5110 display.
		 * 
		 * @param coef new temperature coefficent for the display
		 * 
		 * @sa TemperatureCoefficient
		 */
		void set_temperature_control(TemperatureCoefficient coef)
		{
			this->start_transfer();
			send_command_(FUNCTION_SET_MASK | FUNCTION_SET_EXTENDED);
			send_command_(EXTENDED_SET_BIAS | uint8_t(coef));
			send_command_(FUNCTION_SET_MASK);
			this->end_transfer();
		}

		/**
		 * Set Nokia5110 display to power down mode.
		 */
		void power_down()
		{
			send_command(FUNCTION_SET_MASK | FUNCTION_SET_POWER_DOWN);
		}

		/**
		 * Set Nokia5110 display to power up mode.
		 * This must be called before first display use.
		 */
		void power_up()
		{
			send_command(FUNCTION_SET_MASK);
		}

		/**
		 * Blank Nokia5110 LCD display.
		 * @sa normal()
		 * @sa full()
		 */
		void blank()
		{
			send_command(DISPLAY_CONTROL_MASK | DISPLAY_CONTROL_BLANK);
		}

		/**
		 * Blacken Nokia5110 LCD display.
		 * @sa normal()
		 * @sa blank()
		 */
		void full()
		{
			send_command(DISPLAY_CONTROL_MASK | DISPLAY_CONTROL_FULL);
		}

		/**
		 * Invert Nokia5110 LCD display.
		 * @sa normal()
		 */
		void invert()
		{
			send_command(DISPLAY_CONTROL_MASK | DISPLAY_CONTROL_INVERSE);
		}

		/**
		 * Set Nokia5110 LCD display to normal mode.
		 * This must be called before first display use.
		 * @sa blank()
		 */
		void normal()
		{
			send_command(DISPLAY_CONTROL_MASK | DISPLAY_CONTROL_NORMAL);
		}

	protected:
		/// @cond notdocumented
		LCD5110()
		{
			erase();
		}

		void erase()
		{
			memset(display_, 0, sizeof(display_));
		}

		void before_draw(UNUSED uint8_t x1, UNUSED uint8_t y1, UNUSED uint8_t x2, UNUSED uint8_t y2) {}
		void after_draw(UNUSED uint8_t x1, UNUSED uint8_t y1, UNUSED uint8_t x2, UNUSED uint8_t y2) {}

		// NOTE Coordinates must have been first verified by caller
		bool set_pixel(uint8_t x, uint8_t y, const DRAW_CONTEXT& context)
		{
			// Convert to (r,c)
			const uint8_t c = x;
			const uint8_t r = y / ROW_HEIGHT;
			const uint8_t offset = y % ROW_HEIGHT;
			uint8_t mask = bits::BV8(offset);
			// Get pointer to pixel byte
			uint8_t* pix_column = get_display(r, c);
			// Evaluate final pixel color based on color_ and mode_
			const bool current = (*pix_column & mask);
			const bool dest = context.draw_mode().pixel_op(current);

			// Based on calculated color, set pixel
			if (dest)
			{
				if (current) return false;
				*pix_column |= mask;
			}
			else
			{
				if (!current) return false;
				*pix_column &= uint8_t(~mask);
			}
			return true;
		}

		bool is_valid_char_xy(UNUSED uint8_t x, uint8_t y)
		{
			return (y % ROW_HEIGHT) == 0;
		}

		// NOTE Coordinates must have been first verified by caller
		uint8_t write_char(uint8_t x, uint8_t y, uint16_t glyph_ref, const DRAW_CONTEXT& context)
		{
			// Check column and row not out of range for characters!
			const uint8_t width = context.font().width();
			uint8_t row = y / ROW_HEIGHT;
			const uint8_t col = x;
			//FIXME condition is incorrect actually (if context.font().interchar_space() > 1)
			const uint8_t interchar_space = ((x + width + 1) < WIDTH) ? context.font().interchar_space() : 0;

			uint8_t glyph_index  = 0;
			for (uint8_t glyph_row = 0; glyph_row < context.font().glyph_rows(); ++glyph_row)
			{
				// Get pointer to first byte in row to write in display buffer
				uint8_t* display_ptr = get_display(row, col);

				for (uint8_t i = 0; i < width; ++i)
				{
					uint8_t pixel_bar = context.font().get_char_glyph_byte(glyph_ref, glyph_index);
					*display_ptr = context.draw_mode().bw_pixels_op(pixel_bar, *display_ptr);
					++display_ptr;
					++glyph_index;
				}
				// add interspace if needed
				for (uint8_t i = 0; i < interchar_space; ++i)
				{
					*display_ptr = context.draw_mode().bw_pixels_op(0x00, *display_ptr);
					++display_ptr;
				}
				++row;
			}

			// Return actual width written to display
			return width + interchar_space;
		}

		// Copy invalidated rectangle of display map onto the device
		void update(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
		{
			const uint8_t size = (x2 - x1 + 1);
			const uint8_t xmin = x1;
			const uint8_t ymin = y1 / ROW_HEIGHT;
			const uint8_t ymax = y2 / ROW_HEIGHT;
			this->start_transfer();
			for (uint8_t y = ymin; y <= ymax; ++y)
			{
				set_rc_(y, xmin);
				dc_.set();
				const uint8_t* display = get_display(y, xmin);
				this->transfer(display, size);
			}
			this->end_transfer();
		}
		/// @endcond

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

		void send_command_(uint8_t command)
		{
			dc_.clear();
			this->transfer(command);
		}

		void send_command(uint8_t command)
		{
			this->start_transfer();
			send_command_(command);
			this->end_transfer();
		}

		void set_rc_(uint8_t r, uint8_t c)
		{
			dc_.clear();
			this->transfer(r | SET_ROW_ADDRESS);
			this->transfer(c | SET_COL_ADDRESS);
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
