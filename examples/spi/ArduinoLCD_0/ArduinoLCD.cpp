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

#include <fastarduino/devices/fonts/font_h8x12_default.h>
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

using DISPLAY = devices::display::Display<ST7735<CS, DC, RES>>;
using devices::display::Mode;
// using BITMAP_STREAMER = flash::FlashReader<uint8_t>;

//TODO colors transformation
//TODO check RGB in MADCTL command
int main()
{
	board::init();
	sei();

	// Start SPI interface
	spi::init();
	
	// Start Arduino LCD device
	DISPLAY tft;
	tft.begin();

	devices::display::DefaultFont12x8 font{};
	tft.set_font(font);

	constexpr RGB_565_COLOR black = {0x00, 0x00, 0x00};
	// constexpr RGB_565_COLOR red = {0xFF, 0x00, 0x00};
	// constexpr RGB_565_COLOR green = {0x00, 0xFF, 0x00};
	// constexpr RGB_565_COLOR blue = {0x00, 0x00, 0xFF};
	constexpr RGB_565_COLOR white = {0xFF, 0xFF, 0xFF};

	// tft.fill_screen(black);
	// time::delay_ms(1000);
	// tft.fill_screen(red);
	// time::delay_ms(1000);
	// tft.fill_screen(green);
	// time::delay_ms(1000);
	// tft.fill_screen(blue);
	// time::delay_ms(1000);
	tft.fill_screen(white);
	time::delay_ms(1000);

	// tft.set_draw_mode({Mode::COPY, green});
	// tft.set_fill_mode({Mode::COPY, green});
	// tft.draw_circle({79, 63}, 50);
	// time::delay_ms(1000);

	// tft.set_draw_mode({Mode::COPY, black});
	// tft.set_fill_mode({Mode::COPY, black});
	// tft.draw_rounded_rectangle({0, 0}, {159, 31}, 10);
	// tft.set_draw_mode({Mode::COPY, red});
	// tft.set_fill_mode({Mode::COPY, red});
	// tft.draw_rounded_rectangle({0, 32}, {159, 63}, 10);
	// tft.set_draw_mode({Mode::COPY, blue});
	// tft.set_fill_mode({Mode::COPY, blue});
	// tft.draw_rounded_rectangle({0, 64}, {159, 95}, 10);
	// tft.set_draw_mode({Mode::COPY, white});
	// tft.set_fill_mode({Mode::COPY, white});
	// tft.draw_rounded_rectangle({0, 96}, {159, 127}, 10);
	// time::delay_ms(1000);

	//TODO Try font drawing
	tft.set_draw_mode({Mode::COPY, black});
	tft.set_fill_mode({Mode::COPY, white});
	const uint8_t FONT_WIDTH = font.width() + 1;
	const uint8_t FONT_HEIGHT = font.height() + 1;
	uint8_t xc = 0, yc = 0;
	for (char c = font.first_char(); c <= font.last_char(); ++c)
	{
		tft.draw_char({xc, yc}, c);
		xc += FONT_WIDTH;
		if (xc > tft.WIDTH - FONT_WIDTH)
		{
			xc = 0;
			yc += FONT_HEIGHT;
		}
	}

	// // Try display inversion
	// tft.invert_on();
	// time::delay_ms(5000);
	// tft.invert_off();

	// // Try idle mode
	// tft.idle_on();
	// time::delay_ms(5000);

	// // Try display off/on
	// tft.display_off();
	// time::delay_ms(5000);
	// tft.display_on();
}
