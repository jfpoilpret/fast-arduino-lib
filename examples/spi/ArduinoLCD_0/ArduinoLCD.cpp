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
 * Prototype program to check implementation of ArduinoLCD display (ST7735 chip).
 * Wiring: 
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (SCK): connected to ArduinoLCD breakout SCK pin
 *   - D11 (MOSI): connected to ArduinoLCD breakout MOSI pin
 *   - D10: connected to ArduinoLCD breakout D/C pin
 *   - D9: connected to ArduinoLCD breakout LCD CS pin
 *   - D7: connected to ArduinoLCD breakout Reset pin
 *   - ArduinoLCD breakout LED pin connected to 5V via 1K resistor
 */

#include <fastarduino/devices/fonts/smallfont_h6x12.h>
#include <fastarduino/devices/fonts/arialfont_h16x16.h>
#include <fastarduino/devices/fonts/retrofont_h8x16.h>
#include <fastarduino/devices/st7735.h>
#include <fastarduino/devices/display.h>
#include <fastarduino/utilities.h>
#include <fastarduino/time.h>

#ifndef ARDUINO_UNO
#error "Current target is not supported!"
#endif

using namespace devices::display::st7735;

static constexpr const board::DigitalPin CS = board::DigitalPin::D9_PB1;
static constexpr const board::DigitalPin DC = board::DigitalPin::D10_PB2;
static constexpr const board::DigitalPin RES = board::DigitalPin::D7_PD7;

// using DISPLAY = devices::display::Display<ARDUINO_IDE<CS, DC, RES, ColorModel::RGB_565, Orientation::LANDSCAPE>>;
using DISPLAY = devices::display::Display<ARDUINO_IDE<CS, DC, RES, ColorModel::RGB_666, Orientation::LANDSCAPE>>;
// using DISPLAY = devices::display::Display<ARDUINO_IDE<CS, DC, RES, ColorModel::RGB_444, Orientation::LANDSCAPE>>;
// using DISPLAY = devices::display::Display<ARDUINO_IDE<CS, DC, RES, ColorModel::RGB_444, Orientation::PORTRAIT>>;
// using DISPLAY = devices::display::Display<ARDUINO_IDE<CS, DC, RES, ColorModel::RGB_444, Orientation::REVERSE_LANDSCAPE>>;
// using DISPLAY = devices::display::Display<ARDUINO_IDE<CS, DC, RES, ColorModel::RGB_444, Orientation::REVERSE_PORTRAIT>>;
using COLOR = DISPLAY::COLOR;
static constexpr uint8_t WIDTH = DISPLAY::WIDTH;
static constexpr uint8_t HEIGHT = DISPLAY::HEIGHT;
using devices::display::Mode;
// using BITMAP_STREAMER = flash::FlashReader<uint8_t>;

int main()
{
	board::init();
	sei();

	// Start SPI interface
	spi::init();
	
	// Start Arduino LCD device
	DISPLAY tft;
	tft.begin();

	// devices::display::SmallFont12x6 font{};
	// devices::display::ArialFont16x16 font{};
	devices::display::RetroFont8x16 font{};
	tft.set_font(font);

	constexpr COLOR black = {0x00, 0x00, 0x00};
	constexpr COLOR red = {0xFF, 0x00, 0x00};
	constexpr COLOR green = {0x00, 0xFF, 0x00};
	constexpr COLOR blue = {0x00, 0x00, 0xFF};
	constexpr COLOR white = {0xFF, 0xFF, 0xFF};

	tft.fill_screen(black);
	time::delay_ms(1000);
	tft.fill_screen(red);
	time::delay_ms(1000);
	tft.fill_screen(green);
	time::delay_ms(1000);
	tft.fill_screen(blue);
	time::delay_ms(1000);
	tft.fill_screen(white);
	time::delay_ms(1000);

	tft.set_draw_mode({Mode::COPY, green});
	tft.set_fill_mode({Mode::COPY, green});
	tft.draw_circle({WIDTH / 2, HEIGHT / 2}, 50);
	time::delay_ms(1000);

	tft.set_draw_mode({Mode::COPY, black});
	tft.set_fill_mode({Mode::COPY, black});
	tft.draw_rounded_rectangle({0, 0}, {WIDTH - 1, HEIGHT / 4 - 1}, 10);
	tft.set_draw_mode({Mode::COPY, red});
	tft.set_fill_mode({Mode::COPY, red});
	tft.draw_rounded_rectangle({0, HEIGHT / 4}, {WIDTH - 1, HEIGHT / 2 - 1}, 10);
	tft.set_draw_mode({Mode::COPY, blue});
	tft.set_fill_mode({Mode::COPY, blue});
	tft.draw_rounded_rectangle({0, HEIGHT / 2}, {WIDTH - 1, 3 * HEIGHT / 4 - 1}, 10);
	tft.set_draw_mode({Mode::COPY, white});
	tft.set_fill_mode({Mode::COPY, white});
	tft.draw_rounded_rectangle({0, 3 * HEIGHT / 4}, {WIDTH - 1, HEIGHT - 1}, 10);
	time::delay_ms(1000);

	// Try font drawing
	tft.set_draw_mode({Mode::COPY, black});
	tft.set_fill_mode({Mode::COPY, white});
	const uint8_t FONT_WIDTH = font.width() + 1;
	const uint8_t FONT_HEIGHT = font.height() + 1;
	uint8_t xc = 0, yc = 0;
	for (char c = font.first_char(); c <= font.last_char(); ++c)
	{
		tft.draw_char({xc, yc}, c);
		xc += FONT_WIDTH;
		if (xc > WIDTH - FONT_WIDTH)
		{
			xc = 0;
			yc += FONT_HEIGHT;
		}
	}
	time::delay_ms(5000);

	// Try display inversion
	tft.invert_on();
	time::delay_ms(5000);
	tft.invert_off();

	// Try idle mode
	tft.idle_on();
	time::delay_ms(5000);

	// Try display off/on
	tft.display_off();
	time::delay_ms(5000);
	tft.display_on();
}
