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

//TODO Open Points
// - is there an required order for RASET and CASET?
// - can we optimise start_transfer/end_transfer and dc_ set/clear?
int main()
{
	board::init();
	sei();

	// Start SPI interface
	spi::init();
	
	// Start Arduino LC device
	TFT tft;
	tft.soft_reset();
	tft.sleep_out();
	tft.set_color_model(ColorModel::RGB_565);
	tft.normal_mode();
	tft.display_on();

	// tft.set_orientation(Orientation::LANDSCAPE);
	tft.send_command(0x36, 0xA0);					// MY+MV
	// tft.send_command(0x36, 0xA8);					// MY+MV+RGB
	tft.set_column_address(0x0000, 0x009F);
	tft.set_row_address(0x0000, 0x007F);

	// Display RED pixels

	// constexpr RGB_565_COLOR red = {0xff, 0x00, 0x00};
	// const uint16_t red16 = utils::as_uint16_t(red);
	tft.send_command(0x2C);
	tft.start_data();
	for (uint8_t y = 0; y <= 0x7f; ++y)
		for (uint8_t x = 0; x <= 0x9f; ++x)
		{
			// tft.send_command(0x2C, {utils::high_byte(red16), utils::low_byte(red16)});
			tft.partial_data({0x00, 0x00});
			// tft.write_memory(red);
		}
	tft.end_data();
	time::delay_ms(1000);

	// tft.set_row_address(0, 0x7f);
	// tft.set_column_address(0, 0x9f);
	// constexpr RGB_565_COLOR green = {0x00, 0xff, 0x00};
	// for (uint8_t y = 0; y <= 0x7f; ++y)
	// 	for (uint8_t x = 0; x <= 0x9f; ++x)
	// 		tft.write_memory(green);
	// time::delay_ms(1000);

	// tft.set_row_address(0, 0x7f);
	// tft.set_column_address(0, 0x9f);
	// constexpr RGB_565_COLOR blue = {0x00, 0x00, 0xff};
	// for (uint8_t y = 0; y <= 0x7f; ++y)
	// 	for (uint8_t x = 0; x <= 0x9f; ++x)
	// 		tft.write_memory(blue);
	// time::delay_ms(1000);
}
