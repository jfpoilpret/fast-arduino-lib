//   Copyright 2016-2020 Jean-Francois Poilpret
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
 * This program demonstrates usage of FastArduino Hardware UART support (on 
 * target supporting it) and formatted output streams.
 * 
 * It can be modified and recompiled in order to check various serial configurations:
 * - speed (tested up to 230400 bps)
 * - parity (non, odd or even)
 * - stop bits (1 or 2)
 * 
 * Wiring:
 * - on Arduino UNO, Arduino NANO and Arduino MEGA:
 *   - Use standard TX/RX
 * - on ATmega328P based boards:
 *   - Use standard TX/RX connected to an Serial-USB converter
 * - on ATmega644 based boards:
 *   - D25 (PD1): TX output connected through USB Serial converter to console for display
 * - on Arduino LEONARDO:
 *   - NOT PORTED
 * - on ATtinyX4/ATtinyX5 based boards:
 *   - NOT SUPPORTED
 */

#include <fastarduino/time.h>
#include <fastarduino/uart.h>

// Define vectors we need in the example
REGISTER_UATX_ISR(0)

// Buffers for UART
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	serial::hard::UATX<board::USART::USART0> uart{output_buffer};
	uart.begin(115200);
	streams::ostream out = uart.out();

	// Event Loop
	while (true)
	{
		out.write("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
		out.flush();
		out.write(F("abcdefghijklmnopqrstuvwxyz\n"));
		out.flush();
		time::delay_ms(1000);
	}

}
