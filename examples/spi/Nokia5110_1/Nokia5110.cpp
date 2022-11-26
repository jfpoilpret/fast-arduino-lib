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
#include <fastarduino/time.h>

#ifndef ARDUINO_UNO
#error "Current target is not supported!"
#endif

// For testing we use default SS pin as CS
static constexpr const board::DigitalPin CS = board::DigitalPin::D10_PB2;
static constexpr const board::DigitalPin DC = board::DigitalPin::D9_PB1;
static constexpr const board::DigitalPin RES = board::DigitalPin::D8_PB0;

using NOKIA = devices::display::LCD5110<CS, DC, RES>;

static constexpr uint16_t DELAY_MS = 2000;

int main()
{
	board::init();
	sei();

	// Start SPI interface
	spi::init();
	
	// Start or init SPI device if needed
	NOKIA nokia{false};
	nokia.set_font(devices::display::FONT1);
	nokia.power_up();
	nokia.erase();
	nokia.normal();
	nokia.update();

	time::delay_ms(DELAY_MS);
	nokia.write_char(0, 0, 'A');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(0, 1, 'B');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(0, 2, 'C');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(0, 3, 'D');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(0, 4, 'E');
	nokia.update();
	time::delay_ms(DELAY_MS);
	nokia.write_char(0, 5, 'F');
	nokia.update();
	time::delay_ms(DELAY_MS);

	nokia.write_char(1, 0, 'a');
	nokia.update();
	nokia.write_char(1, 1, 'b');
	nokia.update();
	nokia.write_char(1, 2, 'c');
	nokia.update();
	nokia.write_char(1, 3, 'd');
	nokia.update();
	nokia.write_char(1, 4, 'e');
	nokia.update();
	nokia.write_char(1, 5, 'f');
	nokia.update();
	time::delay_ms(DELAY_MS);

	nokia.invert();
	time::delay_ms(DELAY_MS);

	nokia.normal();
	time::delay_ms(DELAY_MS);

	nokia.write_string(2, 0, "Coucou!");
	nokia.update();
	time::delay_ms(DELAY_MS);

	nokia.write_string(3, 0, F("Coucou!"));
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
}
