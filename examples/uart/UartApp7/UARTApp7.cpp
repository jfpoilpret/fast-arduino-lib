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
 * Software UART debug example.
 * 
 * It is used to check AbstractUATX timing constants by sending always the
 * same string.
 * It can be modified and recompiled in order to check various serial configurations:
 * - speed (tested up to 115200 bps)
 * - parity (non, odd or even)
 * - stop bits (1 or 2)
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - Use standard TX but without hardware UART
 */

#include <fastarduino/soft_uart.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO)
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PD1;
#else
#error "Current target is not yet supported!"
#endif

REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UATX<TX>)

// Buffers for UART
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	// Setup UART
	serial::soft::UATX<TX> uatx{output_buffer};

	// Start UART
	// Following configurations have been tested successfully
	// OK
	// uatx.begin(115200);
	uatx.begin(230400);
	// uatx.begin(230400, serial::Parity::NONE, serial::StopBits::TWO);
	// uatx.begin(230400, serial::Parity::EVEN, serial::StopBits::TWO);

	// KO
	// uatx.begin(460800);
	// uatx.begin(460800, serial::Parity::EVEN);

	streams::ostream out = uatx.out();
	while (true)
	{
		out << F("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") << streams::endl;
		time::delay_ms(500);
	}
}
