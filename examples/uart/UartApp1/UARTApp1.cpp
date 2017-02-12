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

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/uart.h>

// Define vectors we need in the example
REGISTER_UART_ISR(0)

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	UART<Board::USART::USART0> uart{input_buffer, output_buffer};
	uart.register_handler();
	uart.begin(115200);
//	uart.begin(230400);
	InputBuffer& in = uart.in();
//	FormattedInput<InputBuffer> in = uart.fin();
	FormattedOutput<OutputBuffer> out = uart.fout();

	// Event Loop
	while (true)
	{
		out.puts("Enter a letter: ");
		out.flush();
//		int input = in.get();
		int input = ::get(in);
		out.put(input);
		out.put('\n');
		out << (char) input << ' ' << dec << input << ' ' << oct << input << ' ' << hex << input << ' ' << bin << input << endl;
		out.flush();
		_delay_ms(1000.0);
	}
}
