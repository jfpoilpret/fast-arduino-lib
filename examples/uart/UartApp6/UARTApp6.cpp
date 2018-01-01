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
 * Hardware UART example.
 * This program demonstrates usage of FastArduino Hardware UART support (on target supporting it) and formatted
 * input streams.
 * 
 * Wiring:
 * - on Arduino UNO, Arduino NANO and Arduino MEGA:
 *   - Use standard TX/RX
 * - on ATmega328P based boards:
 *   - Use standard TX/RX connected to an Serial-USB converter
 * - on ATtinyX4 based boards:
 *   - NOT SUPPORTED
 */

#include <fastarduino/time.h>
#include <fastarduino/uart.h>

// Define vectors we need in the example
REGISTER_UART_ISR(0)

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

using INPUT = streams::FormattedInput<streams::InputBuffer>;
using OUTPUT = streams::FormattedOutput<streams::OutputBuffer>;

template<typename T>
static void handle(OUTPUT& out, INPUT& in, const flash::FlashStorage* type)
{
	out << type << F(": ") << streams::flush;
	T value{};
	in >> streams::skipws >> value;
	out << value << streams::endl;
}

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	serial::hard::UART<board::USART::USART0> uart{input_buffer, output_buffer};
	uart.register_handler();
	uart.begin(115200);
	INPUT in = uart.fin();
	OUTPUT out = uart.fout();

	// Event Loop
	while (true)
	{
		handle<char>(out, in, F("char"));
		handle<uint16_t>(out, in, F("uint16_t"));
		handle<int16_t>(out, in, F("int16_t"));
		handle<uint32_t>(out, in, F("uint32_t"));
		handle<int32_t>(out, in, F("int32_t"));
		handle<bool>(out, in, F("bool"));
		
		time::delay_ms(1000);
	}
}
