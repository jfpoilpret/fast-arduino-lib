//   Copyright 2016-2019 Jean-Francois Poilpret
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
 * It is used to check AbstractUARX timing constants by receiving always the
 * same string.
 * It can be modified and recompiled in order to check various serial configurations:
 * - speed (tested up to 115200 bps)
 * - parity (non, odd or even)
 * - stop bits (1 or 2)
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - Use standard RX but without hardware UART
 */

#include <fastarduino/gpio.h>
#include <fastarduino/soft_uart.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
constexpr const board::InterruptPin RX = board::InterruptPin::D0_PD0_PCI2;
#define PCI_NUM 2
#else
#error "Current target is not yet supported!"
#endif

// Define vectors we need in the example
REGISTER_UARX_PCI_ISR(RX, PCI_NUM)

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];

constexpr const uint8_t BUF_SIZE =  32;
constexpr const char* EXPECTED = "abcdefghijklmnopqrstuvwxyz";

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	gpio::FastPinType<board::DigitalPin::LED>::TYPE PinLED{gpio::PinMode::OUTPUT, false};
	time::delay_ms(2000);

	// Setup UART
	serial::soft::UARX_PCI<RX> uarx{input_buffer};
	typename interrupt::PCIType<RX>::TYPE pci;
	pci.enable();

	// Start UART
	// Following configurations have been tested successfully
	// OK
	// uarx.begin(pci, 115200);
	// uarx.begin(pci, 115200, serial::Parity::EVEN);

	streams::istream in = uarx.in();

	while (true)
	{
		//TODO
		char buffer[BUF_SIZE + 1];
		in.getline(buffer, BUF_SIZE + 1, '\n');
		if (strcmp(buffer, EXPECTED) == 0)
			PinLED.set();
		else
			PinLED.clear();
		time::delay_ms(10);
	}
}
