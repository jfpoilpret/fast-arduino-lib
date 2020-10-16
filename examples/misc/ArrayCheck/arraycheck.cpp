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
 * Special check of initializer_list (kind of unit tests).
 * Wiring:
 * - on Arduino UNO
 *   - Standard USB to console
 * - on ATtinyX4 based boards
 *   - D8 (PB0): TX conencted to a Serial2USB converter
 */

#include <fastarduino/flash.h>
#include <fastarduino/array.h>
#include <fastarduino/streams.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/tests/assertions.h>

#ifdef ARDUINO_UNO
#define HARD_UART
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#else
#error "Current target is not yet supported!"
#endif

using namespace streams;
using containers::array;

// Buffers for UART
//==================
static char output_buffer[OUTPUT_BUFFER_SIZE];

// Utilities for assertions and traces
//=====================================

template<typename T, uint8_t SIZE>
void assert_array(ostream& out, const flash::FlashStorage* message, 
	const T (&expected)[SIZE], const array<T, SIZE>& actual)
{
	out << message << endl;
	assert(out, F("size"), SIZE, actual.size());
	const uint8_t size = (SIZE <= actual.size() ? SIZE : actual.size());
	uint8_t i = 0;
	for (T t : actual)
	{
		out << t << endl;
		assert(out, "", expected[i], t);
		if (++i == size) break;
	}
}

int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	// Initialize debugging output
#ifdef HARD_UART
	serial::hard::UATX<UART> uatx{output_buffer};
#else
	serial::soft::UATX<TX> uatx{output_buffer};
#endif
	uatx.begin(115200);
	ostream out = uatx.out();
	out << boolalpha << showbase;

	// {
	// 	const char expected[0] = {};
	// 	array<char, 0> actual = {};
	// 	assert_array(out, F("check char[]"), expected, actual);
	// }

	{
		const char expected[] = {'a', 'b', 'c', 'd', 'e'};
		assert_array(out, F("check char[]"), expected, array<char, 5>{{'a', 'b', 'c', 'd', 'e'}});
	}

	{
		const int expected[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
		assert_array(out, F("check int[]"), expected, array<int, 10>{{0, 1, 2 , 3, 4, 5, 6, 7, 8, 9}});
	}

	out << endl;
}
