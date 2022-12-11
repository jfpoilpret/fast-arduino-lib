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
 * Example program to check Display errors on Nokia 5110 (driven by chip PCD8544).
 * This tests tries all primitives with arguments to ensure errors will be produced.
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
#include <fastarduino/devices/font_v5x7_default.h>
#include <fastarduino/devices/display.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/tests/assertions.h>

#ifndef ARDUINO_UNO
#error "Current target is not supported!"
#endif

static const uint8_t OUTPUT_BUFFER_SIZE = 128;
constexpr const board::USART UART = board::USART::USART0;
using UART0 = serial::hard::UATX<UART>;
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(UART0)

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

// For testing we use default SS pin as CS
static constexpr const board::DigitalPin CS = board::DigitalPin::D10_PB2;
static constexpr const board::DigitalPin DC = board::DigitalPin::D9_PB1;
static constexpr const board::DigitalPin RES = board::DigitalPin::D8_PB0;

using NOKIA = devices::display::LCD5110<CS, DC, RES>;
using DISPLAY = devices::display::Display<NOKIA>;
using devices::display::Mode, devices::display::Error;

static constexpr uint16_t DELAY_MS = 2000;

using namespace streams;

ostream& operator<<(ostream& out, Error error)
{
	switch (error)
	{
		case Error::NO_ERROR:
		default:
		return out << F("NO_ERROR");
		
		case Error::NO_FONT_SET:
		return out << F("NO_FONT_SET");
		
		case Error::NO_GLYPH_FOUND:
		return out << F("NO_GLYPH_FOUND");
		
		case Error::OUT_OF_DISPLAY:
		return out << F("OUT_OF_DISPLAY");
		
		case Error::COORDS_INVALID:
		return out << F("COORDS_INVALID");
		
		case Error::INVALID_GEOMETRY:
		return out << F("INVALID_GEOMETRY");
	}
}

int main()
{
	board::init();
	sei();

	// Start UART
	UART0 uart{output_buffer};
	uart.begin(115200);
	ostream out = uart.out();
	out << F("Started") << endl;

	// Start SPI interface
	spi::init();
	
	// Start or init SPI device if needed
	DISPLAY nokia;
	nokia.reset();
	nokia.set_color(true);
	nokia.set_display_bias();
	nokia.set_display_contrast();
	nokia.normal();
	nokia.power_up();
	tests::assert_equals(out, F("last_error() after setup"), Error::NO_ERROR, nokia.last_error());

	nokia.erase();
	nokia.update();

	nokia.write_char(0, 0, 'A');
	tests::assert_equals(out, F("last_error() after write_char without font"), Error::NO_FONT_SET, nokia.last_error());

	devices::display::DefaultVerticalFont7x5 font{};
	nokia.set_font(font);
	nokia.write_char(0, 0, 'A');
	tests::assert_equals(out, F("last_error() after write_char with font"), Error::NO_ERROR, nokia.last_error());

	nokia.write_char(0, 0, char(0));
	tests::assert_equals(out, F("last_error() after write_char without glyph"), Error::NO_GLYPH_FOUND, nokia.last_error());

	nokia.write_char(0, 20, 'A');
	tests::assert_equals(out, F("last_error() after write_char at bad y"), Error::COORDS_INVALID, nokia.last_error());

	nokia.write_char(84, 0, 'A');
	tests::assert_equals(out, F("last_error() after write_char at too big x"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.write_char(0, 48, 'A');
	tests::assert_equals(out, F("last_error() after write_char at too big y"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.write_char(80, 20, 'A');
	tests::assert_equals(out, F("last_error() after write_char out of bounds"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.write_char(79, 16, 'A');
	tests::assert_equals(out, F("last_error() after write_char just in bounds"), Error::NO_ERROR, nokia.last_error());

	nokia.draw_pixel(84, 0);
	tests::assert_equals(out, F("last_error() after draw_pixel at too big x"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.draw_pixel(0, 48);
	tests::assert_equals(out, F("last_error() after draw_pixel at too big y"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.draw_pixel(42, 24);
	tests::assert_equals(out, F("last_error() after draw_pixel at center"), Error::NO_ERROR, nokia.last_error());

	nokia.draw_line(0, 0, 84, 0);
	tests::assert_equals(out, F("last_error() after draw_line at too big x"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.draw_line(0, 0, 0, 48);
	tests::assert_equals(out, F("last_error() after draw_line at too big y"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.draw_line(0, 0, 83, 47);
	tests::assert_equals(out, F("last_error() after draw_line in bounds"), Error::NO_ERROR, nokia.last_error());

	nokia.draw_line(0, 0, 0, 0);
	tests::assert_equals(out, F("last_error() after draw_line on same point"), Error::INVALID_GEOMETRY, nokia.last_error());

	nokia.draw_circle(84, 0, 1);
	tests::assert_equals(out, F("last_error() after draw_circle at too big x"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.draw_circle(0, 48, 1);
	tests::assert_equals(out, F("last_error() after draw_circle at too big y"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.draw_circle(42, 24, 10);
	tests::assert_equals(out, F("last_error() after draw_circle in bounds"), Error::NO_ERROR, nokia.last_error());

	nokia.draw_circle(42, 24, 23);
	tests::assert_equals(out, F("last_error() after draw_circle just in bounds"), Error::NO_ERROR, nokia.last_error());

	nokia.draw_circle(42, 24, 24);
	tests::assert_equals(out, F("last_error() after draw_circle just out bounds"), Error::OUT_OF_DISPLAY, nokia.last_error());

	nokia.draw_circle(42, 24, 0);
	tests::assert_equals(out, F("last_error() after draw_circle of radius 0"), Error::INVALID_GEOMETRY, nokia.last_error());

	nokia.draw_rectangle(10, 10, 10, 40);
	tests::assert_equals(out, F("last_error() after draw_rectangle on flat vertical rectangle"), Error::INVALID_GEOMETRY, nokia.last_error());

	nokia.draw_rectangle(10, 10, 60, 10);
	tests::assert_equals(out, F("last_error() after draw_rectangle on flat horizontal rectangle"), Error::INVALID_GEOMETRY, nokia.last_error());

	nokia.draw_rectangle(10, 10, 60, 40);
	tests::assert_equals(out, F("last_error() after draw_rectangle on normal rectangle"), Error::NO_ERROR, nokia.last_error());

	nokia.draw_rectangle(60, 40, 10, 10);
	tests::assert_equals(out, F("last_error() after draw_rectangle on normal rectangle"), Error::NO_ERROR, nokia.last_error());

	nokia.update();
	out << F("Finished") << endl;
}
