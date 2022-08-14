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
 * Software UART example. take #2
 * This program demonstrates usage of FastArduino Software (emulated) UART support 
 * and formatted output streams.
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
 * - on Arduino UNO and NANO:
 *   - Use standard TX/RX but without hardware UART
 * - on ATmega328P based boards:
 *   - Use standard TX/RX but without hardware UART, connected to an Serial-USB 
 *   converter
 * - on Arduino LEONARDO:
 *   - Use D9/D8 as TX/RX, connected to an Serial-USB converter
 * - on Arduino MEGA:
 *   - Use D52/D53 as TX/RX, connected to an Serial-USB converter
 * - on ATtinyX4 based boards:
 *   - Use D1-D0 as TX-RX, connected to an Serial-USB converter
 * - on ATtinyX5 based boards:
 *   - Use D1-D0 as TX-RX, connected to an Serial-USB converter
 * - on ATmega644 based boards:
 *   - Use D1(PA1)-D0(PA0) as TX-RX, connected to an Serial-USB converter
 */

#include <fastarduino/soft_uart.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PD1;
constexpr const board::InterruptPin RX = board::InterruptPin::D0_PD0_PCI2;
#define PCI_NUM 2
#elif defined (ARDUINO_LEONARDO)
constexpr const board::DigitalPin TX = board::DigitalPin::D9_PB5;
constexpr const board::InterruptPin RX = board::InterruptPin::D8_PB4_PCI0;
#define PCI_NUM 0
#elif defined (ARDUINO_MEGA)
constexpr const board::DigitalPin TX = board::DigitalPin::D52_PB1;
constexpr const board::InterruptPin RX = board::InterruptPin::D53_PB0_PCI0;
#define PCI_NUM 0
#elif defined (BREADBOARD_ATTINYX4)
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
constexpr const board::InterruptPin RX = board::InterruptPin::D0_PA0_PCI0;
#define PCI_NUM 0
#elif defined (BREADBOARD_ATTINYX5)
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PB1;
constexpr const board::InterruptPin RX = board::InterruptPin::D0_PB0_PCI0;
#define PCI_NUM 0
#elif defined (BREADBOARD_ATMEGAXX4P)
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
constexpr const board::InterruptPin RX = board::InterruptPin::D0_PA0_PCI0;
#define PCI_NUM 0
#else
#error "Current target is not yet supported!"
#endif

// Define vectors we need in the example
REGISTER_UART_PCI_ISR(RX, TX, PCI_NUM)
REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UART_PCI<RX, TX>)

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
	interrupt::PCI_SIGNAL<RX> pci;
	serial::soft::UART_PCI<RX, TX> uart{input_buffer, output_buffer, pci};
	pci.enable();

	// Start UART
	// Uncomment the line with the configuration you want to test
//	uart.begin(9600);
	uart.begin(57600);
	// uart.begin(115200);
//	uart.begin(230400);
//	uart.begin(230400, Serial::Parity::NONE, Serial::StopBits::TWO);
//	uart.begin(230400, Serial::Parity::EVEN, Serial::StopBits::TWO);
//	uart.begin(230400, Serial::Parity::EVEN);
//	uart.begin(115200, Serial::Parity::ODD);
//	uart.begin(115200, Serial::Parity::EVEN);

	streams::istream in = uart.in();
	streams::ostream out = uart.out();

	while (true)
	{
		int value = in.get();
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
		// time::delay_ms(10);
	}
}
