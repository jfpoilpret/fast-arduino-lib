//   Copyright 2016-2021 Jean-Francois Poilpret
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
 * Special check for FastArduino utilities (kind of unit tests).
 * Wiring:
 * - Arduino UNO
 *   - Standard USB to console
 */

#include <fastarduino/utilities.h>
#include <fastarduino/uart.h>
#include <fastarduino/streams.h>
#include <fastarduino/iomanip.h>

#ifdef ARDUINO_UNO
static const board::USART USART = board::USART::USART0;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

static const uint8_t BUFFER_SIZE = 10;

using namespace streams;
using namespace utils;

// NOTE: we allow a delta of 1 in accuracy because map_raw_to_physical() loses
// 1 bit of precision due to approximation in division (for performance optimization)
void assert(ostream& out, int16_t expected, int16_t actual, uint16_t delta = 1U)
{
	out << "    Comparison ";
	uint16_t diff;
	if (expected < actual)
		diff = actual - expected;
	else
		diff = expected - actual;
	if (diff <= delta)
		out << " OK exp=" << expected << " act=" << actual << endl;
	else
		out << " KO exp=" << expected << " act=" << actual << endl;
}

void assert_map_raw_to_physical(
	ostream& out, int16_t expected, int16_t input, UnitPrefix prefix, int16_t range, uint8_t precision_bits)
{
	int16_t outut = map_raw_to_physical(input, prefix, range, precision_bits);
	out	<< "map_raw_to_physical(" 
		<< input << ", " 
		<< int8_t(prefix) << ", " 
		<< range << ", " 
		<< precision_bits << ")" << endl;
	assert(out, expected, outut);
}

void assert_map_physical_to_raw(
	ostream& out, int16_t expected, int16_t input, UnitPrefix prefix, int16_t range, uint8_t precision_bits)
{
	int16_t outut = map_physical_to_raw(input, prefix, range, precision_bits);
	out	<< "map_physical_to_raw(" 
		<< input << ", " 
		<< int8_t(prefix) << ", " 
		<< range << ", " 
		<< precision_bits << ")" << endl;
	assert(out, expected, outut);
}

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Start UART
	serial::hard::UATX<USART> uart{output_buffer};
	uart.begin(115200);
	ostream out = uart.out();
	out.flags(ios::boolalpha);

	out << "START..." << endl;

	out << "Checks map_raw_to_physical()" << endl;
	// Checks on identity mapping (output = input)
	assert_map_raw_to_physical(out, 0, 0, UnitPrefix::NONE, 32767, 15);
	assert_map_raw_to_physical(out, 1, 1, UnitPrefix::NONE, 32767, 15);
	assert_map_raw_to_physical(out, -1, -1, UnitPrefix::NONE, 32767, 15);
	assert_map_raw_to_physical(out, 16384, 16384, UnitPrefix::NONE, 32767, 15);
	assert_map_raw_to_physical(out, -16384, -16384, UnitPrefix::NONE, 32767, 15);
	assert_map_raw_to_physical(out, 32767, 32767, UnitPrefix::NONE, 32767, 15);
	assert_map_raw_to_physical(out, -32767, -32767, UnitPrefix::NONE, 32767, 15);
	assert_map_raw_to_physical(out, -32768, -32768, UnitPrefix::NONE, 32767, 15);

	// Examples with possible values from MPU6050 gyro
	// - ranges: 250, 500, 1000 or 2000 (dps)
	// - 15 bits precision
	// - conversion to deci-dps
	assert_map_raw_to_physical(out, 0, 0, UnitPrefix::DECI, 2000, 15);
	assert_map_raw_to_physical(out, 10000, 16384, UnitPrefix::DECI, 2000, 15);
	assert_map_raw_to_physical(out, -10000, -16384, UnitPrefix::DECI, 2000, 15);
	assert_map_raw_to_physical(out, 20000, 32767, UnitPrefix::DECI, 2000, 15);
	assert_map_raw_to_physical(out, -20000, -32767, UnitPrefix::DECI, 2000, 15);
	assert_map_raw_to_physical(out, -20000, -32768, UnitPrefix::DECI, 2000, 15);

	// Examples with possible values from MPU6050 accelerometer
	// - ranges: 250, 500, 1000 or 2000 (dps)
	// - 15 bits precision
	// - conversion to deca-dps
	assert_map_raw_to_physical(out, 0, 0, UnitPrefix::DECA, 2000, 15);
	assert_map_raw_to_physical(out, 100, 16384, UnitPrefix::DECA, 2000, 15);
	assert_map_raw_to_physical(out, -100, -16384, UnitPrefix::DECA, 2000, 15);
	assert_map_raw_to_physical(out, 200, 32767, UnitPrefix::DECA, 2000, 15);
	assert_map_raw_to_physical(out, -200, -32767, UnitPrefix::DECA, 2000, 15);
	assert_map_raw_to_physical(out, -200, -32768, UnitPrefix::DECA, 2000, 15);

	out << endl;
	out << "Checks map_physical_to_raw()" << endl;
	// Checks on identity mapping (output = input)
	assert_map_physical_to_raw(out, 0, 0, UnitPrefix::NONE, 32767, 15);
	assert_map_physical_to_raw(out, 1, 1, UnitPrefix::NONE, 32767, 15);
	assert_map_physical_to_raw(out, -1, -1, UnitPrefix::NONE, 32767, 15);
	assert_map_physical_to_raw(out, 16384, 16384, UnitPrefix::NONE, 32767, 15);
	assert_map_physical_to_raw(out, -16384, -16384, UnitPrefix::NONE, 32767, 15);
	assert_map_physical_to_raw(out, 32767, 32767, UnitPrefix::NONE, 32767, 15);
	assert_map_physical_to_raw(out, -32767, -32767, UnitPrefix::NONE, 32767, 15);
	assert_map_physical_to_raw(out, -32768, -32768, UnitPrefix::NONE, 32767, 15);

	// Examples with possible values from MPU6050 gyro
	// - ranges: 250, 500, 1000 or 2000 (dps)
	// - 15 bits precision
	// - conversion to deci-dps
	assert_map_physical_to_raw(out, 0, 0, UnitPrefix::DECI, 2000, 15);
	assert_map_physical_to_raw(out, 16384, 10000, UnitPrefix::DECI, 2000, 15);
	assert_map_physical_to_raw(out, -16384, -10000, UnitPrefix::DECI, 2000, 15);
	assert_map_physical_to_raw(out, 32767, 20000, UnitPrefix::DECI, 2000, 15);
	assert_map_physical_to_raw(out, -32767, -20000, UnitPrefix::DECI, 2000, 15);

	// Examples with possible values from MPU6050 accelerometer
	// - ranges: 250, 500, 1000 or 2000 (dps)
	// - 15 bits precision
	// - conversion to deca-dps
	assert_map_physical_to_raw(out, 0, 0, UnitPrefix::DECA, 2000, 15);
	assert_map_physical_to_raw(out, 16384, 100, UnitPrefix::DECA, 2000, 15);
	assert_map_physical_to_raw(out, -16384, -100, UnitPrefix::DECA, 2000, 15);
	assert_map_physical_to_raw(out, 32767, 200, UnitPrefix::DECA, 2000, 15);
	assert_map_physical_to_raw(out, -32767, -200, UnitPrefix::DECA, 2000, 15);

	out << "END" << endl;
	return 0;
}
