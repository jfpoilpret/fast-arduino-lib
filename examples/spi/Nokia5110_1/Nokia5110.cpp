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
 * Wiring: TODO
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (SCK): connected to SPI device SCK pin
 *   - D12 (MISO): connected to SPI device MISO pin (sometimes called Dout)
 *   - D11 (MOSI): connected to SPI device MOSI pin (sometimes called Din)
 *   - D10 (SS): connected to SPI device CS pin
 *   - direct USB access (traces output)
 */

#include <fastarduino/devices/lcd5110.h>
#include <fastarduino/devices/lcd5110_font1.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>

//TODO check board here

using UART = serial::hard::UATX<board::USART::USART0>;

// Define vectors we need in the example
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(UART)

// UART for traces
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

// For testing we use default SS pin as CS
static constexpr const board::DigitalPin CS = board::DigitalPin::D10_PB2;
static constexpr const board::DigitalPin DC = board::DigitalPin::D9_PB1;

using NOKIA = devices::display::LCD5110<CS, DC>;

using streams::endl;
using streams::dec;
using streams::hex;

int main()
{
	board::init();
	sei();

	// Init UART output for traces
	UART uart{output_buffer};
	uart.begin(115200);
	streams::ostream out = uart.out();
	out.width(2);
	
	// Start SPI interface
	spi::init();
	out << F("SPI initialized") << endl;
	
	// Start or init SPI device if needed
	NOKIA nokia;
	out << F("Nokia initialized") << endl;
	nokia.set_font(devices::display::FONT1);
	nokia.power_up();
	out << F("Nokia powered up") << endl;
	time::delay_ms(1000);
	nokia.erase();
	out << F("Nokia erased") << endl;
	time::delay_ms(1000);
	nokia.normal();
	out << F("Nokia normal mode") << endl;

	time::delay_ms(1000);
	nokia.write_char(0, 0, 'A');
	nokia.write_char(0, 1, 'B');
	nokia.write_char(0, 2, 'C');
	nokia.write_char(0, 3, 'D');
	nokia.write_char(0, 4, 'E');
	nokia.write_char(0, 5, 'F');
	out << F("Nokia ABCDEF") << endl;

	time::delay_ms(1000);
	nokia.write_char(1, 0, 'a');
	nokia.write_char(1, 1, 'b');
	nokia.write_char(1, 2, 'c');
	nokia.write_char(1, 3, 'd');
	nokia.write_char(1, 4, 'e');
	nokia.write_char(1, 5, 'f');
	out << F("Nokia abcdef") << endl;

	time::delay_ms(1000);
	nokia.invert();
	out << F("Nokia invert") << endl;

	time::delay_ms(1000);
	nokia.normal();
	out << F("Nokia normal") << endl;

	time::delay_ms(1000);
	nokia.blank();
	out << F("Nokia blank") << endl;

	time::delay_ms(1000);
	nokia.full();
	out << F("Nokia full") << endl;

	// Stop SPI device if needed
	time::delay_ms(1000);
	out << F("End") << endl;
}
