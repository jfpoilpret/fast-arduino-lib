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
 * Special checks for iostreams on ATtiny.
 * This program is not aimed for upload, just build.
 */

#include <fastarduino/streams.h>

#if defined(BREADBOARD_ATTINYX4)
#include <fastarduino/soft_uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace streams;

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	serial::soft::UATX<TX> uart{output_buffer};
	uart.begin(9600);
	ostream out = uart.out();

	const double v = 123.456;
	out << fixed << v << endl;
	out << scientific << v << endl;

	return 0;
}
