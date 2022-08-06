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
 * Blocking EEPROM Read and Writes.
 * This program shows usage of FastArduino EEPROM API.
 * It interfaces with user through the UART console and allows:
 * - writing values to EEPROM
 * - reading values to EEPROM
 * 
 * Wiring: 
 * - on Arduino boards: direct USB access
 * - on ATtinyX4 based boards:
 *   - D1 (PA1): TX output connected to Serial-USB allowing traces display on a PC terminal
 * - on ATmega644 based boards:
 *   - D25 (PD1): TX output connected to Serial-USB allowing traces display on a PC terminal
 */

#include <fastarduino/time.h>
#include <fastarduino/eeprom.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t EEPROM_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t EEPROM_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(1)
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t EEPROM_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t EEPROM_BUFFER_SIZE = 64;
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t EEPROM_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

REGISTER_EEPROM_ISR()

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];
// Buffers for EEPROM queued writes
static uint8_t eeprom_buffer[EEPROM_BUFFER_SIZE];

using namespace eeprom;

static void trace_eeprom(streams::ostream& out, uint16_t address, uint16_t loops = 1)
{
	for (uint16_t i = 0; i < loops; ++i)
	{
		out.width(4);
		out << address << ": ";
		out.width(2);
		for (uint8_t j = 0 ; j < 16; ++j)
		{
			uint8_t value;
			EEPROM::read(address++, value);
			out << value << ' ' << streams::flush;
		}
		out << streams::endl;
	}
}

struct Content
{
	uint8_t content[16];
};

template<typename T>
static void write_eeprom(streams::ostream& out, QueuedWriter& writer, uint16_t address, const T& content)
{
	if (!writer.write(address, content))
	{
		out << "Could not write to " << address << streams::endl;
		writer.wait_until_done();
		if (!writer.write(address, content))
			out << "Could not again write to " << address << streams::endl;
	}
}

int main()
{
	// Enable interrupts at startup time
	sei();
#if HARDWARE_UART
	serial::hard::UATX<UART> uart{output_buffer};
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);

	streams::ostream out = uart.out();
	out << streams::hex;
	out << "\nInitial EEPROM content" << streams::endl;	
	trace_eeprom(out, 0, EEPROM::size() / 16);
	
	QueuedWriter writer{eeprom_buffer};
	
	writer.erase();
	out << "After EEPROM erase" << streams::endl;
	writer.wait_until_done();
	trace_eeprom(out, 0, EEPROM::size() / 16);

	Content content;
	for (uint8_t i = 0 ; i < 16; ++i)
		content.content[i] = i;

	for (uint16_t address = 0; address < 512; address += 16)
		write_eeprom(out, writer, address, content);
	out << "After 512 EEPROM writes" << streams::endl;
	writer.wait_until_done();
	trace_eeprom(out, 0, 32);

	char buffer[] = "abcdefghijklmnopqrstuvwxyz";
	write_eeprom(out, writer, 512, buffer);
	out << "After EEPROM string write" << streams::endl;
	writer.wait_until_done();
	trace_eeprom(out, 512, 3);

	writer.write(768, buffer, 6);
	out << "After EEPROM partial string write" << streams::endl;
	writer.wait_until_done();
	trace_eeprom(out, 768, 3);
	
	// Check out of ranges writes
	uint8_t value = 0;
	bool ok;
	ok = writer.write(E2END + 1, value);
	if (ok)
		out << "ERROR! write(E2END + 1) did not fail!" << streams::flush;
	ok = writer.write(E2END, buffer);
	if (ok)
		out << "ERROR! write(E2END, 27) did not fail!" << streams::flush;
	ok = writer.write(E2END, buffer, 0);
	if (ok)
		out << "ERROR! write(E2END, x, 0) did not fail!" << streams::flush;
	return 0;
}
