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
 * Software UART example. take #2
 * This program demonstrates usage of FastArduino Software (emulated) UART support and formatted output streams.
 * For RX pin we use PCI ISR.
 * In this example, we used just a single UART instead of individual UATX and UARX.
 * Serial errors are traced as they occur.
 * 
 * It can be modified and recompiled in order to check various serial configurations:
 * - speed (tested up to 115200 bps)
 * - parity (non, odd or even)
 * - stop bits (1 or 2)
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - Use standard TX/RX but without hardware UART
 * - on Arduino MEGA:
 *   - TODO
 * - on ATmega328P based boards:
 *   - Use standard TX/RX but without hardware UART, connected to an Serial-USB converter
 * - on ATtinyX4 based boards:
 *   - Use D1-D0 as TX-RX, connected to an Serial-USB converter
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/soft_uart.h>
#include <fastarduino/utilities.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1_PD1;
constexpr const Board::DigitalPin RX = Board::InterruptPin::D0_PD0_PCI2;
#define PCI_NUM 2
#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin TX = Board::DigitalPin::D52;
constexpr const Board::DigitalPin RX = Board::InterruptPin::D53_PCI0;
#define PCI_NUM 0
#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1;
constexpr const Board::DigitalPin RX = Board::InterruptPin::D0_PCI0;
#define PCI_NUM 0
#else
#error "Current target is not yet supported!"
#endif

// Define vectors we need in the example
REGISTER_UART_PCI_ISR(RX, PCI_NUM)

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
	
	// Setup UART
	Soft::UART<RX, TX> uart{input_buffer, output_buffer};
	uart.register_rx_handler();
	typename PCIType<RX>::TYPE pci;
	pci.enable();

	// Start UART
	// Uncomment the line with the configuration you want to test
//	uart.begin(pci, 9600);
	uart.begin(pci, 115200);
//	uart.begin(pci, 230400);
//	uart.begin(pci, 230400, Serial::Parity::NONE, Serial::StopBits::TWO);
//	uart.begin(pci, 230400, Serial::Parity::EVEN, Serial::StopBits::TWO);
//	uart.begin(pci, 230400, Serial::Parity::EVEN);
//	uart.begin(pci, 115200, Serial::Parity::ODD);
//	uart.begin(pci, 115200, Serial::Parity::EVEN);

	InputBuffer& in = uart.in();
	FormattedOutput<OutputBuffer> out = uart.fout();

	while (true)
	{
		int value = in.get();
		if (value != InputBuffer::EOF)
			out.put(value);
		if (uart.has_errors())
		{
			out.put(' ');
			out.put(uart.frame_error() ?  'F' : '-');
			out.put(uart.data_overrun() ?  'O' : '-');
			out.put(uart.parity_error() ?  'P' : '-');
			out.put(uart.queue_overflow() ?  'Q' : '-');
			out.put('\n');
			uart.clear_errors();
		}
		_delay_ms(10.0);
	}
}
