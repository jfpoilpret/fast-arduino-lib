//   Copyright 2016-2018 Jean-Francois Poilpret
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
 * Blocking EEPROM Read and Writes.
 * This program shows usage of FastArduino EEPROM API.
 * It interfaces with user through the UART console and allows:
 * - writing values to EEPROM
 * - reading values to EEPROM
 * 
 * Wiring: TODO
 * - on ATmega328P based boards (including Arduino UNO):
 * - on Arduino MEGA:
 * - on ATtinyX4 based boards:
 *   - D1: TX output connected to Serial-USB allowing traces display on a PC terminal
 */

#include <fastarduino/time.h>
#include <fastarduino/eeprom.h>
#include <fastarduino/iomanip.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(1)
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
#elif defined (BREADBOARD_ATTINYX5)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PB1;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace eeprom;
using namespace streams;

static void trace_eeprom(ostream& out, uint16_t address, uint16_t loops = 1)
{
	for (uint16_t i = 0; i < loops; ++i)
	{
		out << hex << setw(4) << address << ": ";
		for (uint8_t j = 0 ; j < 16; ++j)
		{
			uint8_t value;
			EEPROM::read(address++, value);
			//FIXME the following lines displays only spaces when value is 2 digits?
			// out << hex << setw(2) << value << ' ' << flush;
			out << hex << value << ' ' << flush;
		}
		out << endl;
	}
}

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
#if HARDWARE_UART
	serial::hard::UATX<UART> uart{output_buffer};
	uart.register_handler();
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);

	ostream out = uart.out();
	out << F("Initial EEPROM content") << endl;	
	trace_eeprom(out, 0, EEPROM::size() / 16);
	
	EEPROM::erase();
	out << F("After EEPROM erase") << endl;
	trace_eeprom(out, 0, EEPROM::size() / 16);

	for (uint16_t i = 0 ; i < 256; ++i)
		EEPROM::write(i, (uint8_t) i);
	out << F("After 256 EEPROM writes") << endl;	
	trace_eeprom(out, 0, EEPROM::size() / 16);
	
	char buffer[] = "abcdefghijklmnopqrstuvwxyz";
	EEPROM::write(256, buffer);
	out << F("After EEPROM string write") << endl;
	trace_eeprom(out, 256, 3);

	EEPROM::write(256 + 64, buffer, 6);
	out << F("After EEPROM partial string write") << endl;
	trace_eeprom(out, 256 + 64, 3);
	
	// Check out of ranges read/writes
	uint8_t value;
	bool ok;
	ok = EEPROM::read(E2END + 1, value);
	if (ok)
		out << F("ERROR! read(E2END + 1) did not fail!") << endl;
	ok = EEPROM::read(E2END, buffer);
	if (ok)
		out << F("ERROR! read(E2END, 27) did not fail!") << endl;
	ok = EEPROM::read(E2END, buffer, 0);
	if (ok)
		out << F("ERROR! read(E2END, x, 0) did not fail!") << endl;
	ok = EEPROM::write(E2END + 1, value);
	if (ok)
		out << F("ERROR! write(E2END + 1) did not fail!") << endl;
	ok = EEPROM::write(E2END, buffer);
	if (ok)
		out << F("ERROR! write(E2END, 27) did not fail!") << endl;
	ok = EEPROM::write(E2END, buffer, 0);
	if (ok)
		out << F("ERROR! write(E2END, x, 0) did not fail!") << endl;
	return 0;
}
