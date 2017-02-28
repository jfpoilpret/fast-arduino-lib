//   Copyright 2016-2017 Jean-Francois Poilpret
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

#include <fastarduino/utilities.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

struct Dummy
{
	uint16_t a;
	uint8_t b;
	bool c;
	int16_t d;
	char e;
};

using OUTPUT = streams::FormattedOutput<streams::OutputBuffer>;

OUTPUT& operator<< (OUTPUT& out, const Dummy& item)
{
	out	<< streams::dec << F("{\n\ta: ") << item.a 
		<< F("\n\tb: ") << item.b 
		<< F("\n\tc: ") << item.c 
		<< F("\n\td: ") << item.d 
		<< F("\n\te: ") << item.e 
		<< F("\n}\n") << streams::flush;
	return out;
}

const Dummy sample1 PROGMEM = {54321, 123, true, -22222, 'z'};
const Dummy sample2 PROGMEM = {12345, 231, false, -11111, 'A'};

int main()
{
	// Enable interrupts at startup time
	sei();
#if HARDWARE_UART
	serial::UATX<board::USART::USART0> uart{output_buffer};
	uart.register_handler();
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);
	streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();

	Dummy value;
	out << F("sample1 = ") << read_flash(&sample1, value);
	out << F("sample2 = ") << read_flash(&sample2, value);
	return 0;
}
