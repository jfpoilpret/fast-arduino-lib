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
 * This tests almost all display fonctions (primitives) with different modes.
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
#include <fastarduino/devices/fonts/font_v5x7_default.h>
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
using DRAW_MODE = typename DISPLAY::DRAW_MODE;
using devices::display::Mode;
using BITMAP_STREAMER = flash::FlashReader<uint8_t>;

// W16xH12 BITMAP
static const uint8_t BITMAP[] PROGMEM = {
	0b00111100, 0b00111100,
	0b01000010, 0b01000010,
	0b10000001, 0b10000001,
	0b10000001, 0b10000001,
	0b01000000, 0b00000010,
	0b00100000, 0b00000100,
	0b00010000, 0b00001000,
	0b00001000, 0b00010000,
	0b00000100, 0b00100000,
	0b00000010, 0b01000000,
	0b00000001, 0b10000000,
	0b00000001, 0b10000000,
};

static constexpr uint16_t TITLE_MS = 2000;
static constexpr uint16_t SETTINGS_MS = 3000;
static constexpr uint16_t CHAR_MS = 100;
static constexpr uint16_t PIXEL_MS = 5;
static constexpr uint16_t DELAY_MS = 1000;
static constexpr uint16_t BLINK_MS = 250;

static const flash::FlashStorage* color(bool color)
{
	if (color)
		return F("BLACK");
	else
		return F("WHITE");
}

static const flash::FlashStorage* mode(Mode mode)
{
	switch (mode)
	{
		case Mode::COPY:
		return F("COPY");
		
		case Mode::XOR:
		return F("XOR");
		
		case Mode::AND:
		return F("AND");
		
		case Mode::OR:
		return F("OR");
		
		case Mode::NO_CHANGE:
		return F("NONE");
	}
}

static void setup(DISPLAY& nokia, DRAW_MODE draw, DRAW_MODE fill, bool skip_setup = false)
{
	nokia.set_fill_mode({Mode::NO_CHANGE, true});
	nokia.set_draw_mode({Mode::COPY, true});
	nokia.erase();

	nokia.draw_string({0, 0}, F("DRAW:"));
	nokia.draw_string({0, 8}, F(" color:"));
	nokia.draw_string({42, 8}, color(draw.color()));
	nokia.draw_string({0, 16}, F(" mode:"));
	nokia.draw_string({42, 16}, mode(draw.mode()));

	if (fill)
	{
		nokia.draw_string({0, 24}, F("FILL:"));
		nokia.draw_string({0, 32}, F(" color:"));
		nokia.draw_string({42, 32}, color(fill.color()));
		nokia.draw_string({0, 40}, F(" mode:"));
		nokia.draw_string({42, 40}, mode(fill.mode()));
	}

	nokia.update();
	time::delay_ms(SETTINGS_MS);
	// Extra wait time if fill mode active so we can read it
	if (fill)
		time::delay_ms(SETTINGS_MS);

	nokia.erase();
	nokia.update();
	if (!skip_setup)
	{
		nokia.set_draw_mode(draw);
		nokia.set_fill_mode(fill);
	}
}

static void display_title(DISPLAY& nokia, const flash::FlashStorage* title)
{
	nokia.set_fill_mode({Mode::NO_CHANGE, true});
	nokia.set_draw_mode({Mode::COPY, true});
	nokia.erase();
	nokia.draw_string({0, 16}, title);
	nokia.update();
	time::delay_ms(TITLE_MS);
	nokia.erase();
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
	devices::display::DefaultVerticalFont7x5 font{};
	nokia.set_font(font);
	nokia.power_up();

	display_title(nokia, F("===> CHAR <==="));
	setup(nokia, {Mode::COPY, true}, {Mode::NO_CHANGE, false});

	uint8_t x = 0, y = 0;
	const uint8_t FONT_WIDTH = font.width() + 1;
	const uint8_t FONT_HEIGHT = font.height() + 1;
	for (uint8_t c = font.first_char(); c <= font.last_char(); ++c)
	{
		nokia.draw_char({x, y}, char(c));
		nokia.update();
		time::delay_ms(CHAR_MS);
		x += FONT_WIDTH;
		if (x > nokia.WIDTH - FONT_WIDTH)
		{
			x = 0;
			y += FONT_HEIGHT;
			if (y > nokia.HEIGHT - FONT_HEIGHT)
			{
				y = 0;
				nokia.erase();
			}
		}
	}

	nokia.invert();
	time::delay_ms(DELAY_MS);

	nokia.normal();
	time::delay_ms(DELAY_MS);

	setup(nokia, {Mode::OR, true}, {Mode::NO_CHANGE, false});
	for (uint8_t c = font.first_char(); c <= font.last_char(); ++c)
	{
		nokia.draw_char({40, 16}, char(c));
		nokia.update();
		time::delay_ms(CHAR_MS);
	}

	setup(nokia, {Mode::AND, true}, {Mode::NO_CHANGE, false}, true);
	// first draw a black rectangle in the 7x5 location of the displayed character
	nokia.set_draw_mode({Mode::COPY, true});
	for (uint8_t y = 8; y < 30; ++y)
		for (uint8_t x = 35; x < 50; ++x)
			nokia.draw_point({x, y});
	nokia.update();
	nokia.set_draw_mode({Mode::AND, true});
	for (uint8_t c = 'A'; c <= 'Z'; ++c)
	{
		nokia.draw_char({40, 16}, char(c));
		nokia.update();
		time::delay_ms(CHAR_MS);
	}

	display_title(nokia, F("===> STR <==="));
	setup(nokia, {Mode::COPY, true}, {Mode::NO_CHANGE, false});
	nokia.draw_string({8, 16}, "Coucou!");
	nokia.update();
	time::delay_ms(DELAY_MS);

	display_title(nokia, F("===> FSTR <==="));
	setup(nokia, {Mode::OR, true}, {Mode::NO_CHANGE, false});
	nokia.draw_string({8, 16}, F("Coucou!"));
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.draw_string({9, 16}, F("Coucou!"));
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.draw_string({10, 16}, F("Coucou!"));
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Try drawing pixels
	display_title(nokia, F("===> PIXL <==="));
	setup(nokia, {Mode::COPY, true}, {Mode::NO_CHANGE, false});
	for (uint8_t y = 0; y < nokia.HEIGHT; ++y)
		for (uint8_t x = 0; x < nokia.WIDTH; ++x)
		{
			nokia.draw_point({x, y});
			nokia.update();
			time::delay_ms(PIXEL_MS);
		}
	
	display_title(nokia, F("===> LINE <==="));
	setup(nokia, {Mode::COPY, true}, {Mode::NO_CHANGE, false});
	// Try drawing H line
	nokia.draw_line({10, 40}, {79, 40});
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Try drawing V line
	nokia.draw_line({70, 0}, {70, 45});
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Try drawing other lines
	nokia.draw_line({0, 0}, {83, 47});
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.draw_line({0, 47}, {83, 0});
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.draw_line({0, 0}, {30, 47});
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.draw_line({0, 47}, {30, 0});
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Try drawing rectangle
	display_title(nokia, F("===> RECT <==="));
	setup(nokia, {Mode::COPY, true}, {Mode::NO_CHANGE, false});
	nokia.draw_rectangle({30, 35}, {55, 45});
	nokia.update();
	time::delay_ms(DELAY_MS);

	setup(nokia, {Mode::XOR, true}, {Mode::NO_CHANGE, false});
	for (uint8_t i = 0; i < 10; ++i)
	{
		nokia.draw_rectangle({30, 35}, {55, 45});
		nokia.update();
		time::delay_ms(BLINK_MS);
	}
	time::delay_ms(DELAY_MS);

	// Try drawing circle
	display_title(nokia, F("===> CIRC <==="));
	setup(nokia, {Mode::COPY, true}, {Mode::NO_CHANGE, false});
	nokia.draw_circle({DISPLAY::WIDTH / 2, DISPLAY::HEIGHT / 2}, 20);
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Check XOR mode for circle boundaries
	setup(nokia, {Mode::XOR, true}, {Mode::NO_CHANGE, false});
	for (uint8_t i = 0; i < 10; ++i)
	{
		nokia.draw_circle({42, 24}, 20);
		nokia.update();
		time::delay_ms(BLINK_MS);
	}
	time::delay_ms(DELAY_MS);

	// Try drawing rounded rectangle
	display_title(nokia, F("=> RND RECT <="));
	setup(nokia, {Mode::COPY, true}, {Mode::NO_CHANGE, false});
	nokia.draw_rounded_rectangle({5, 5}, {78, 42}, 8);
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Check XOR mode for roundes rectangle boundaries
	setup(nokia, {Mode::XOR, true}, {Mode::NO_CHANGE, false});
	for (uint8_t i = 0; i < 10; ++i)
	{
		nokia.draw_rounded_rectangle({5, 5}, {78, 42}, 8);
		nokia.update();
		time::delay_ms(BLINK_MS);
	}
	time::delay_ms(DELAY_MS);

	// Try drawing polyline
	display_title(nokia, F("=> POLYLINE <="));
	setup(nokia, {Mode::COPY, true}, {Mode::NO_CHANGE, false});
	nokia.draw_polyline({{10, 10}, {15, 25}, {50, 30}, {70, 45}, {12, 40}});
	nokia.update();
	time::delay_ms(DELAY_MS);
	
	// Try drawing polygon
	display_title(nokia, F("=> POLYGON <="));
	setup(nokia, {Mode::COPY, true}, {Mode::NO_CHANGE, false});
	nokia.draw_polygon({{10, 10}, {15, 25}, {50, 30}, {70, 45}, {12, 40}});
	nokia.update();
	time::delay_ms(DELAY_MS);
	
	// Try fillers
	display_title(nokia, F("=> FIL RECT <="));
	setup(nokia, {Mode::COPY, true}, {Mode::COPY, true});
	nokia.draw_rectangle({30, 35}, {55, 45});
	nokia.update();
	time::delay_ms(DELAY_MS);

	setup(nokia, {Mode::NO_CHANGE, true}, {Mode::XOR, true}, true);
	nokia.set_fill_mode({Mode::NO_CHANGE, false});
	nokia.set_draw_mode({Mode::COPY, true});
	nokia.draw_rectangle({30, 35}, {55, 45});
	nokia.update();
	nokia.set_draw_mode({Mode::NO_CHANGE, true});
	nokia.set_fill_mode({Mode::XOR, true});
	for (uint8_t i = 0; i < 10; ++i)
	{
		nokia.draw_rectangle({30, 35}, {55, 45});
		nokia.update();
		time::delay_ms(BLINK_MS);
	}
	time::delay_ms(DELAY_MS);

	// Try fillers on rounded rectangles
	display_title(nokia, F("=> FIL RREC <="));
	setup(nokia, {Mode::COPY, true}, {Mode::COPY, true});
	nokia.draw_rounded_rectangle({5, 5}, {78, 42}, 8);
	nokia.update();
	time::delay_ms(DELAY_MS);

	setup(nokia, {Mode::NO_CHANGE, true}, {Mode::XOR, true}, true);
	nokia.set_fill_mode({Mode::NO_CHANGE, false});
	nokia.set_draw_mode({Mode::COPY, true});
	nokia.draw_rounded_rectangle({5, 5}, {78, 42}, 8);
	nokia.update();
	nokia.set_draw_mode({Mode::NO_CHANGE, true});
	nokia.set_fill_mode({Mode::XOR, true});
	for (uint8_t i = 0; i < 10; ++i)
	{
		nokia.draw_rounded_rectangle({5, 5}, {78, 42}, 8);
		nokia.update();
		time::delay_ms(BLINK_MS);
	}
	time::delay_ms(DELAY_MS);

	display_title(nokia, F("=> FIL CIRC <="));
	setup(nokia, {Mode::COPY, true}, {Mode::COPY, true});
	nokia.draw_circle({42, 24}, 20);
	nokia.update();
	time::delay_ms(DELAY_MS);

	setup(nokia, {Mode::NO_CHANGE, true}, {Mode::XOR, true}, true);
	nokia.set_draw_mode({Mode::COPY, true});
	nokia.set_fill_mode({Mode::NO_CHANGE});
	nokia.draw_circle({42, 24}, 20);
	nokia.update();
	nokia.set_draw_mode({Mode::NO_CHANGE});
	nokia.set_fill_mode({Mode::XOR, true});
	for (uint8_t i = 0; i < 10; ++i)
	{
		nokia.draw_circle({42, 24}, 20);
		nokia.update();
		time::delay_ms(BLINK_MS);
	}
	time::delay_ms(DELAY_MS);

	// Try Bitmap
	display_title(nokia, F("==> BITMAP <=="));
	setup(nokia, {Mode::COPY, true}, {Mode::COPY, false});
	nokia.draw_bitmap({36, 18}, {16, 12}, BITMAP_STREAMER{BITMAP});
	// nokia.draw_bitmap({36, 18}, {16, 12}, BITMAP);
	nokia.update();
	time::delay_ms(DELAY_MS);

	setup(nokia, {Mode::XOR, true}, {Mode::XOR, true}, true);
	nokia.set_draw_mode({Mode::COPY, true});
	nokia.set_fill_mode({Mode::COPY, false});
	nokia.draw_bitmap({36, 18}, {16, 12}, BITMAP_STREAMER{BITMAP});
	// nokia.draw_bitmap({36, 18}, {16, 12}, BITMAP);
	nokia.update();
	nokia.set_draw_mode({Mode::XOR, true});
	nokia.set_fill_mode({Mode::XOR, true});
	for (uint8_t i = 0; i < 10; ++i)
	{
		nokia.draw_bitmap({36, 18}, {16, 12}, BITMAP_STREAMER{BITMAP});
		// nokia.draw_bitmap({36, 18}, {16, 12}, BITMAP);
		nokia.update();
		time::delay_ms(BLINK_MS);
	}
	time::delay_ms(DELAY_MS);

	nokia.erase();
	nokia.update();
}
