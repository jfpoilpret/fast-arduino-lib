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

#include <fastarduino/time.h>
#include <fastarduino/eeprom.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t EEPROM_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t EEPROM_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t EEPROM_BUFFER_SIZE = 64;
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1;
#else
#error "Current target is not yet supported!"
#endif

REGISTER_EEPROM_ISR()

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];
// Buffers for EEPROM queued writes
static uint8_t eeprom_buffer[EEPROM_BUFFER_SIZE];

using namespace eeprom;

static void trace_eeprom(FormattedOutput<OutputBuffer>& out, uint16_t address)
{
	out << "Read EEPROM at address " << address << '\n';
	for (uint16_t i = 0 ; i < 16; ++i)
	{
		uint8_t value;
		EEPROM::read(address + i, value);
		out << value << ' ' << flush;
	}
	out << '\n';
}

struct Content
{
	uint8_t content[16];
};

static void write_eeprom(FormattedOutput<OutputBuffer>& out, QueuedWriter& writer, uint16_t address, Content content)
{
//	writer.write(address, content);
	if (writer.write(address, content))
	{
		writer.wait_until_done();
		trace_eeprom(out, address);
	}
}

int main()
{
	// Enable interrupts at startup time
	sei();
#if HARDWARE_UART
	UATX<Board::USART::USART0> uart{output_buffer};
	uart.register_handler();
#else
	Soft::UATX<RX, TX> uart{output_buffer};
#endif
	uart.begin(115200);

	FormattedOutput<OutputBuffer> out = uart.fout();
//	out << "Start...\n";
	
	QueuedWriter writer{eeprom_buffer};
	
	Content content;
	for (uint16_t i = 0 ; i < 16; ++i)
		content.content[i] = i;

	for (uint16_t address = 0; address < 1024; address += 16)
		write_eeprom(out, writer, address, content);
	
//	out << "Finish\n";
	return 0;
}
