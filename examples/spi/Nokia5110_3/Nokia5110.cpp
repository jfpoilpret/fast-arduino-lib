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

/*
 * Example program to check display to Nokia 5110 (driven by chip PCD8544).
 * This tests font capabilities.
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

static constexpr uint16_t CHAR_MS = 200;
static constexpr uint16_t DELAY_MS = 2000;

// define font 8x16 with only "A"
class VerticalFont15x7 : public devices::display::Font<true>
{
public:
	VerticalFont15x7() : Font{0x41, 0x41, 7, 15, FONT} {}

private:
	static const uint8_t FONT[] PROGMEM;
};

const uint8_t VerticalFont15x7::FONT[] PROGMEM =
{
	//TODO glyph for A
	// Bytes are always row after row, with columns inside each row first
	0x7e, 0x11, 0x11, 0x11, 0x7e, // 0x41 A
};

// define font 16x8 with only "A"
class VerticalFont7x15 : public devices::display::Font<true>
{
public:
	VerticalFont7x15() : Font{0x41, 0x41, 15, 7, FONT} {}

private:
	static const uint8_t FONT[] PROGMEM;
};

const uint8_t VerticalFont7x15::FONT[] PROGMEM =
{
	// Bytes are always row after row, with columns inside each row first
	0x7e, 0x7e, 0x7e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x7e, 0x7e, 0x7e,  // 0x41 A
};

//TODO define font 16x16

static void check_font(DISPLAY& nokia, const devices::display::Font<true>& font)
{
	nokia.set_font(font);
	uint8_t x = 0;
	const uint8_t FONT_WIDTH = font.width() + 1;
	for (uint8_t i = 0; i < 10; ++i)
	{
		nokia.draw_char({x, 0}, 'A');
		nokia.update();
		time::delay_ms(CHAR_MS);
		x += FONT_WIDTH;
	}

	time::delay_ms(DELAY_MS);
	nokia.erase();
	nokia.draw_string({8, 8}, "AAAAAAAA");
	nokia.update();

	time::delay_ms(DELAY_MS);
	nokia.erase();
	nokia.draw_string({8, 24}, F("AAAAAAAA"));
	nokia.update();
}

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
	nokia.power_up();
	nokia.erase();
	nokia.update();

	check_font(nokia, VerticalFont7x15{});
}
