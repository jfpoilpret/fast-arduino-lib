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
 * output streams.
 * 
 * It can be modified and recompiled in order to check various serial configurations:
 * - speed (tested up to 230400 bps)
 * - parity (non, odd or even)
 * - stop bits (1 or 2)
 * 
 * Wiring:
 * - on Arduino UNO and Arduino MEGA:
 *   - Use standard TX/RX
 * - on ATmega328P based boards:
 *   - Use standard TX/RX connected to an Serial-USB converter
 * - on ATtinyX4 based boards:
 *   - NOT SUPPORTED
 */

#include <fastarduino/time.h>
#include <fastarduino/uart.h>

#ifdef ARDUINO_LEONARDO
// Define vectors we need in the example
REGISTER_UART_ISR(1)
static const board::USART USART = board::USART::USART1;
#else
// Define vectors we need in the example
REGISTER_UART_ISR(0)
static const board::USART USART = board::USART::USART0;
#endif

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	serial::hard::UART<USART> uart{input_buffer, output_buffer};
	uart.register_handler();
	uart.begin(115200);
//	uart.begin(230400);
	streams::InputBuffer& in = uart.in();
//	streams::FormattedInput<streams::InputBuffer> in = uart.fin();
	streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();

	// Event Loop
	while (true)
	{
		out.puts("Enter a letter: ");
		out.flush();
//		int input = in.get();
		int input = streams::get(in);
		out.put(input);
		out.put('\n');
		out << (char) input << ' ' 
			<< streams::dec << input << ' ' 
			<< streams::oct << input << ' ' 
			<< streams::hex << input << ' ' 
			<< streams::bin << input << streams::endl;
		time::delay_ms(1000);
	}
}
