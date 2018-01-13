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
#else
#error "Current target is not yet supported!"
#endif

REGISTER_EEPROM_ISR()

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

static constexpr const uint8_t EEPROM_BUFFER_SIZE = 64;
static uint8_t eeprom_buffer[EEPROM_BUFFER_SIZE];

using namespace eeprom;

struct Dummy
{
	uint16_t a;
	uint8_t b;
	bool c;
	int16_t d;
	char e;
};

using OUTPUT = streams::ostream;

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

Dummy sample1 EEMEM = {54321, 123, true, -22222, 'z'};
Dummy sample2 EEMEM = {12345, 231, false, -11111, 'A'};

static void trace_eeprom(OUTPUT& out)
{
	Dummy value;
	EEPROM::read(&sample1, value);
	out << F("sample1 = ") << value;
	EEPROM::read(&sample2, value);
	out << F("sample2 = ") << value;
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

	OUTPUT out = uart.out();

	out << F("\nInitial EEPROM content") << streams::endl;
	trace_eeprom(out);
	
	// Write values
	EEPROM::write(&sample1, Dummy{1, 2, true, -1, '9'});
	EEPROM::write(&sample2, Dummy{0, 0, false, 0, '0'});
	
	out << F("\nEEPROM after sync. write") << streams::endl;
	trace_eeprom(out);
	
	// Async. write
	QueuedWriter writer{eeprom_buffer};
	writer.write(&sample1, Dummy{10, 20, true, -10, '5'});
	writer.write(&sample2, Dummy{15, 25, true, -15, '8'});
	writer.wait_until_done();
	
	out << F("\nEEPROM after async. write") << streams::endl;
	trace_eeprom(out);
	
	return 0;
}
