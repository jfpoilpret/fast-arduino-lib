//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * Software UART debug example.
 * 
 * It is used to check UART timing constants by receiving and echoing lines at high rate.
 * It can be modified and recompiled in order to check various serial configurations:
 * - speed (tested up to 230400 bps)
 * - parity (non, odd or even)
 * - stop bits (1 or 2)
 * 
 * IMPORTANT: at max rate (230400 bps), on reception, there must be a delay between each 
 * received character (I use 1ms on CuteCom, smaller values may work, but CuteCom does not 
 * allow delays in us). This is probably due to the fact that the operations following 
 * stop bit in reception are rather long (push char to queue and return from ISR).
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - Use standard TX/RX but without hardware UART
 */

#include <fastarduino/soft_uart.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PD1;
constexpr const board::InterruptPin RX = board::InterruptPin::D0_PD0_PCI2;
#define PCI_NUM 2
#else
#error "Current target is not yet supported!"
#endif

// Define vectors we need in the example
REGISTER_UART_PCI_ISR(RX, TX, PCI_NUM)
REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UART_PCI<RX, TX>)

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 100;
static char input_buffer[INPUT_BUFFER_SIZE];
static const uint8_t OUTPUT_BUFFER_SIZE = 100;
static char output_buffer[OUTPUT_BUFFER_SIZE];

constexpr const uint8_t BUF_SIZE =  100;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	// Setup UART
	interrupt::PCI_SIGNAL<RX> pci;
	serial::soft::UART_PCI<RX, TX> uart{input_buffer, output_buffer, pci};
	pci.enable();

	// Start UART
	// Following configurations have been tested successfully
	// OK
	// uart.begin(115200);
	uart.begin(230400);
	// uart.begin(115200, serial::Parity::EVEN);

	streams::istream in = uart.in();
	streams::ostream out = uart.out();

	while (true)
	{
		char buffer[BUF_SIZE + 1];
		in.getline(buffer, BUF_SIZE + 1, '\n');
		out << buffer << streams::endl;
	}
}
