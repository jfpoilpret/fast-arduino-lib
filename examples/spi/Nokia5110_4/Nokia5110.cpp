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

/*
 * Example program to check display to Nokia 5110 (driven by chip PCD8544).
 * This tests a given font.
 * 
 * PCD8544 chip is using 3.3V levels max. Several breakouts exist (e.g. Adafruit), 
 * but most of them (all?) do not include level converters.
 * It is highly suggested to use level converters (5V->3.3V, no need for bidir)
 * for all logical signals.
 * 
 * I generally use CD74HC4050 CMOS circuit which can be used to lower 6 digital 
 * signals from 5V to 3.3V (PCD8544 needs 5 signals).
 * Most breakouts also include backlighting LEDs which current must be restricted
 * according to breakout datasheet.
 * 
 * Wiring: 
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (SCK): connected to 5110 breakout SCLK pin (via level converter)
 *   - D11 (MOSI): connected to 5110 breakout DN pin (via level converter)
 *   - D10 (SS): connected to 5110 breakout SCE pin (via level converter)
 *   - D9: connected to 5110 breakout D/C pin (via level converter)
 *   - D8: connected to 5110 breakout RST pin (via level converter)
 *   - 5110 breakout LED pin connected to 3.3V via 330 resistor
 */

#include <fastarduino/devices/lcd5110.h>
#include <fastarduino/devices/font.h>
#include <fastarduino/devices/display.h>
#include <fastarduino/time.h>

// Font to test
#include <fastarduino/devices/fonts/font_v5x7_default.h>
#include "Font7x5.h"

#ifndef ARDUINO_UNO
#error "Current target is not supported!"
#endif

// For testing we use default SS pin as CS
static constexpr const board::DigitalPin CS = board::DigitalPin::D10_PB2;
static constexpr const board::DigitalPin DC = board::DigitalPin::D9_PB1;
static constexpr const board::DigitalPin RES = board::DigitalPin::D8_PB0;

using NOKIA = devices::display::LCD5110<CS, DC, RES>;
using DISPLAY = devices::display::Display<NOKIA>;
using devices::display::Mode;

static constexpr uint16_t DELAY_MS = 20000;

int main()
{
	board::init();
	sei();

	// Start SPI interface
	spi::init();
	
	// Start or init SPI device if needed
	DISPLAY nokia;
	nokia.reset();
	nokia.set_draw_mode({Mode::COPY, true});
	nokia.set_display_bias();
	nokia.set_display_contrast();
	nokia.normal();
	// devices::display::DefaultVerticalFont7x5 font{};
	Font7x5 font{};
	nokia.set_font(font);
	nokia.power_up();
	nokia.erase();
	nokia.update();

	const uint8_t FONT_WIDTH = font.width() + 1;
	const uint8_t FONT_HEIGHT = font.height() + 1;
	while (true)
	{
		uint8_t x = 0, y = 0;
		for (uint8_t c = font.first_char(); c <= font.last_char(); ++c)
		{
			nokia.draw_char({x, y}, char(c));
			x += FONT_WIDTH;
			if (x > nokia.WIDTH - FONT_WIDTH)
			{
				x = 0;
				y += FONT_HEIGHT;
				if (y > nokia.HEIGHT - FONT_HEIGHT)
				{
					nokia.update();
					time::delay_ms(DELAY_MS);
					y = 0;
					nokia.erase();
					nokia.update();
				}
			}
		}
		nokia.update();
		time::delay_ms(DELAY_MS);
		nokia.erase();
		nokia.update();
	}
}
