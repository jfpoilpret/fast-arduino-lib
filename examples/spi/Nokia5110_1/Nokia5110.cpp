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
 * TODO
 * 
 * Wiring: 
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (SCK): connected to 5110 breakout SCLK pin (via level converter)
 *   - D11 (MOSI): connected to 5110 breakout DN pin (via level converter)
 *   - D10 (SS): connected to 5110 breakout SCE pin (via level converter)
 *   - D9: connected to 5110 breakout D/C pin (via level converter)
 *   - 5110 breakout RST pin connected to 3.3V via 10K resistor (pullup)
 *   - 5110 breakout LED pin connected to 3.3V via 330 resistor
 *   - direct USB access (traces output)
 */

#include <fastarduino/devices/lcd5110.h>
#include <fastarduino/devices/lcd5110_font1.h>
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

static constexpr uint16_t DELAY_MS = 2000;

int main()
{
	board::init();
	sei();

	// Start SPI interface
	spi::init();
	
	// Start or init SPI device if needed
	DISPLAY nokia;
	nokia.reset();
	nokia.set_display_bias();
	nokia.set_display_contrast();
	nokia.normal();
	devices::display::Font7x5 font{};
	nokia.set_font(font);
	nokia.power_up();

	nokia.erase();
	nokia.update();

	time::delay_ms(DELAY_MS);
	nokia.write_char(0, 0, 'A');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(8, 0, 'B');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(16, 0, 'C');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(24, 0, 'D');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(32, 0, 'E');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(40, 0, 'F');
	nokia.update();
	time::delay_ms(DELAY_MS);

	nokia.write_char(0, 8, 'a');
	nokia.update();
	nokia.write_char(8, 8, 'b');
	nokia.update();
	nokia.write_char(16, 8, 'c');
	nokia.update();
	nokia.write_char(24, 8, 'd');
	nokia.update();
	nokia.write_char(32, 8, 'e');
	nokia.update();
	nokia.write_char(40, 8, 'f');
	nokia.update();
	time::delay_ms(DELAY_MS);

	nokia.invert();
	time::delay_ms(DELAY_MS);

	nokia.normal();
	time::delay_ms(DELAY_MS);

	nokia.write_string(0, 16, "Coucou!");
	nokia.update();
	time::delay_ms(DELAY_MS);

	nokia.write_string(0, 24, F("Coucou!"));
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Try drawing pixels
	nokia.clear_pixel(0, 1);
	nokia.clear_pixel(0, 2);
	for (uint8_t x = 0; x < 84; ++x)
		nokia.set_pixel(x, 3);
	for (uint8_t y = 40; y < 48; ++y)
		nokia.set_pixel(42, y);
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Try drawing H line
	nokia.draw_line(10, 40, 79, 40);
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Try drawing V line
	nokia.draw_line(70, 0, 70, 45);
	nokia.update();
	time::delay_ms(DELAY_MS);

	// Try drawing other lines
	//TODO

	// Try drawing rectangle
	nokia.draw_rectangle(30, 35, 55, 45);
	nokia.update();
}
