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
 *   - D13 (SCK): connected to ArduinoLCD breakout SCLK pin
 *   - D11 (MOSI): connected to ArduinoLCD breakout DN pin
 *   - D10: connected to ArduinoLCD breakout D/C pin
 *   - D9: connected to ArduinoLCD breakout CS pin
 *   - D7: connected to ArduinoLCD breakout RST pin
 *   - ArduinoLCD breakout LED pin connected to 5V via 1K resistor
 */

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

class TFT : public ST7735<CS, DC, RES>
{
	friend int main();
};

// using DISPLAY = devices::display::Display<TFT>;
// using DRAW_MODE = typename DISPLAY::DRAW_MODE;
// using devices::display::Mode;
// using BITMAP_STREAMER = flash::FlashReader<uint8_t>;

//TODO colors transformation
//TODO check RGB in MADCTL command
int main()
{
	board::init();
	sei();

	// Start SPI interface
	spi::init();
	
	// Start Arduino LC device
	TFT tft;
	tft.begin();

	constexpr RGB_565_COLOR black = {0x00, 0x00, 0x00};
	constexpr RGB_565_COLOR red = {0xFF, 0x00, 0x00};
	constexpr RGB_565_COLOR green = {0x00, 0xFF, 0x00};
	constexpr RGB_565_COLOR blue = {0x00, 0x00, 0xFF};
	constexpr RGB_565_COLOR white = {0xFF, 0xFF, 0xFF};

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

	// Display 4 stripes of pixels of 4 distinct colors: black, red, blue, white
	tft.start_memory_write();
	for (uint8_t y = 0; y <= 0x7f; ++y)
	{
		for (uint8_t x = 0; x <= 0x9f; ++x)
		{
			if (y < 0x20)
				tft.write_memory(black);
			else if (y < 0x40)
				tft.write_memory(red);
			else if (y < 0x60)
				tft.write_memory(blue);
			else
				tft.write_memory(white);
		}
	}
	tft.stop_memory_write();
	time::delay_ms(1000);

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
