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
 * Special check for iostreams: error handling.
 * Wiring:
 * - Arduino UNO
 *   - Standard USB to console
 */

#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/streams.h>
#include <fastarduino/iomanip.h>

#ifdef ARDUINO_UNO
static const board::USART USART = board::USART::USART0;
// Define vectors we need in the example
REGISTER_UART_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UART<USART>)
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 16;
static const uint8_t OUTPUT_BUFFER_SIZE = 32;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace streams;

void trace_state(ostream& out, const flash::FlashStorage* type, ios::iostate state)
{
	out << type << F(" state=") << hex << setw(2) << right << setfill('0') << state << endl;
}

static void trace_state(ostream& out)
{
	trace_state(out, F("out"), out.rdstate());
	out.clear();
}

static void trace_state(ostream& out, ios::iostate state)
{
	trace_state(out, F("out"), state);
	out.clear();
}

static void trace_state(ostream& out, istream& in)
{
	trace_state(out, F("in"), in.rdstate());
	in.clear();
}

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Start UART
	serial::hard::UART<USART> uart{input_buffer, output_buffer};
	uart.begin(9600);

	istream in = uart.in();
	ostream out = uart.out();

	// Show initial state value of out stream
	trace_state(out);

	// Show out state after normal output and flush
	out << "abcdefghijklmnopqrstuvwxyz" << endl;
	trace_state(out);

	// Show out state after overflowed output
	out	<< "abcdefghijklmnopqrstuvwxyz" 
		<< "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
	ios::iostate state = out.rdstate();
	out << flush << endl;
	trace_state(out, state);

	out << endl;

	// Show initial state value of in stream
	trace_state(out, in);

	// Show in state after normal input
	uint16_t value;
	out << "Enter correct uint16_t: " << flush;
	in >> value;
	out << dec << value << endl;
	trace_state(out, in);

	// Show in state after incorrect (malformed) input
	out << "Enter incorrect uint16_t: " << flush;
	in >> value;
	out << dec << value << endl;
	trace_state(out, in);

	// Show in state when trying to get() a character but streambuf currently empty
	out << "Enter EOF" << endl;
	int val = in.get();
	out << hex << val << endl;
	trace_state(out, in);

	return 0;
}
