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
 * Hardware UART example.
 * This program demonstrates usage of FastArduino Hardware UART support (on target supporting it) and formatted
 * input streams.
 * 
 * Wiring:
 * - on Arduino UNO, Arduino NANO and Arduino MEGA:
 *   - Use standard TX/RX
 * - on ATmega328P based boards:
 *   - Use standard TX/RX connected to an Serial-USB converter
 * - on ATtinyX4 based boards:
 *   - NOT SUPPORTED
 */

#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/iomanip.h>

// Macros to define what we want to test: just comment/uncomment according to what you want to test
// #define CHECK_OUT_MANIPULATORS
// #define CHECK_OUT_FLOAT
// #define CHECK_OUT_ALIGNMENTS
// #define CHECK_IN_EXTRACTORS
// #define CHECK_IN_MANIPULATORS
// #define CHECK_IN_STRING
// #define CHECK_OUT_UNITBUF
#define CHECK_IN_GET

// Define vectors we need in the example
REGISTER_UART_ISR(0)

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace streams;

using INPUT = istream;
using OUTPUT = ostream;

#ifdef CHECK_IN_EXTRACTORS
template<typename T>
static void handle(OUTPUT& out, INPUT& in, const flash::FlashStorage* type)
{
	out << type << F(": ") << flush;
	T value{};
	in >> skipws >> value;
	out << value << endl;
}
#endif

#if defined(CHECK_OUT_MANIPULATORS) || defined(CHECK_OUT_FLOAT)
template<typename T>
static void display_num(OUTPUT& out, T value)
{
	out << bin << value << endl;
	out << dec << value << endl;
	out << oct << value << endl;
	out << hex << value << endl;
}
#endif

#ifdef CHECK_OUT_MANIPULATORS
template<typename T>
static void handle_num(OUTPUT& out, T value, const flash::FlashStorage* type)
{
	out << F("testing output of ") << type << F(" (") << dec << value << ')' << endl;
	display_num<T>(out, value);

	out << showbase;
	display_num<T>(out, value);
	out << noshowbase;

	out << uppercase;
	display_num<T>(out, value);
	out << nouppercase;

	out << uppercase << showbase;
	display_num<T>(out, value);
	out << nouppercase << noshowbase;

	out << showpos;
	display_num<T>(out, value);
	out << noshowpos;
}
#endif

#ifdef CHECK_OUT_FLOAT
static void handle_float(OUTPUT& out, double value)
{
	out << F("testing output of double (") << defaultfloat << setprecision(6) << value << ')' << endl;
	display_num<double>(out, value);

	out << showbase;
	display_num<double>(out, value);
	out << noshowbase;

	out << fixed << value << endl;
	out << scientific << value << endl;

	out << uppercase;
	out << fixed << value << endl;
	out << scientific << value << endl;
	out << nouppercase;

	out << showpos;
	out << fixed << value << endl;
	out << scientific << value << endl;
	out << noshowpos;

	// check precision too
	out << setprecision(12) << fixed << value << endl;
	out << setprecision(12) << scientific << value << endl;

	out << setprecision(3) << fixed << value << endl;
	out << setprecision(3) << scientific << value << endl;

	out << setprecision(0) << fixed << value << endl;
	out << setprecision(0) << scientific << value << endl;
}
#endif

#ifdef CHECK_OUT_ALIGNMENTS
static void handle_alignments(OUTPUT& out, uint8_t width, char filler, bool is_left = true)
{
	out << F("testing alignments") << endl;
	out << setfill(filler);
	if (is_left) out << left; else out << right;

	out << setw(width) << 'a' << endl;
	out << setw(width) << "abcdefghij" << endl;
	out << setw(width) << F("abcdefghij") << endl;
	out << setw(width) << 1234 << endl;
	out << setw(width) << 1234U << endl;
	out << setw(width) << 123456L << endl;
	out << setw(width) << 123456UL << endl;
	out << setw(width) << 123.456 << endl;
	out << setw(width) << true << endl;
	out << setw(width) << false << endl;
	out << boolalpha;
	out << setw(width) << true << endl;
	out << setw(width) << false << endl;
	out << noboolalpha;

	out << setfill(' ');
}
#endif

#ifdef CHECK_IN_STRING
template<uint8_t WIDTH>
void input_string(OUTPUT& out, INPUT& in)
{
	out << F("string of length ") << dec << WIDTH << F(": ") << flush;
	char buffer[WIDTH + 1];
	in >> noskipws >> setw(WIDTH + 1) >> buffer;
	out << buffer << endl;
	in >> skipws;
}
#endif

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	serial::hard::UART<board::USART::USART0> uart{input_buffer, output_buffer};
	uart.register_handler();
	uart.begin(115200);
	INPUT in = uart.in();
	OUTPUT out = uart.out();

#ifdef CHECK_OUT_MANIPULATORS
	// Check all output manipulators
	handle_num<uint16_t>(out, 1234, F("uint16_t"));
	handle_num<int16_t>(out, 1234, F("int16_t"));
	handle_num<int16_t>(out, -1234, F("int16_t"));

	handle_num<uint32_t>(out, 123456, F("uint32_t"));
	handle_num<int32_t>(out, 123456, F("int32_t"));
	handle_num<int32_t>(out, -123456, F("int32_t"));
#endif

#ifdef CHECK_OUT_FLOAT
	// check floats
	handle_float(out, 123.456);
	handle_float(out, -123.456);
	handle_float(out, -12345678901234567890.12345);
#endif

#ifdef CHECK_OUT_ALIGNMENTS
	// check justification: setw(), setfill(), left, right...
	handle_alignments(out, 5, ' ', false);
	handle_alignments(out, 5, ' ', true);
	handle_alignments(out, 5, '~', false);
	handle_alignments(out, 5, '~', true);

	handle_alignments(out, 10, ' ', false);
	handle_alignments(out, 10, ' ', true);
	handle_alignments(out, 10, '~', false);
	handle_alignments(out, 10, '~', true);

	handle_alignments(out, 30, ' ', false);
	handle_alignments(out, 30, ' ', true);
	handle_alignments(out, 30, '~', false);
	handle_alignments(out, 30, '~', true);
#endif

	// Event Loop
	while (true)
	{
#ifdef CHECK_IN_EXTRACTORS
		handle<char>(out, in, F("char"));
		handle<uint16_t>(out, in, F("uint16_t"));
		handle<int16_t>(out, in, F("int16_t"));
		handle<uint32_t>(out, in, F("uint32_t"));
		handle<int32_t>(out, in, F("int32_t"));
		handle<bool>(out, in, F("bool"));
#endif
		
#ifdef CHECK_IN_MANIPULATORS
		// check formatted inputs: bool
		bool v1;
		out << F("bool as alpha: ") << flush;
		in >> boolalpha >> skipws >> v1;
		out << v1 << endl;
		out << F("bool as num: ") << flush;
		in >> noboolalpha >> skipws >> v1;
		out << v1 << endl;

		// check formatted inputs: numeric with base
		uint16_t v2;
		out << F("num as dec: ") << flush;
		in >> dec >> skipws >> v2;
		out << v2 << endl;
		out << F("num as hex: ") << flush;
		in >> hex >> skipws >> v2;
		out << v2 << endl;
		out << F("num as bin: ") << flush;
		in >> bin >> skipws >> v2;
		out << v2 << endl;
		out << F("num as oct: ") << flush;
		in >> oct >> skipws >> v2;
		out << v2 << endl;
#endif

#ifdef CHECK_IN_STRING
		// check input to char*
		input_string<10>(out, in);
		input_string<50>(out, in);
		input_string<200>(out, in);
#endif

#ifdef CHECK_IN_GET
		// check istream get(), getline(), ignore()
		out << F("get 1 char: ") << flush;
		int c = in.get();
		out << char(c) << endl;
		in.ignore(0, '\n');
		out << F("get 10 char max: ") << flush;
		char buf[10 + 1];
		in.get(buf, sizeof buf);
		out << buf << endl;
		in.ignore(0, '\n');
		out << F("getline 10 char max: ") << flush;
		in.getline(buf, sizeof buf);
		out << buf << endl;
		out << F("ignore 5 then get 10 char max: ") << flush;
		in.ignore(5).get(buf, sizeof buf);
		in.ignore(0, '\n');
		out << buf << endl;
#endif

		time::delay_ms(1000);
	}

#ifdef CHECK_OUT_UNITBUF
	// check unitbuf: with unitbuf the program should not exit until every character has been output
	out << unitbuf << F("abcdefghijklmnopqrstuvwxyz\n");
	out << F("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
	out << F("1234567890\n");

	// check nounitbuf: with nounitbuf the program will exit before all characters have been output
	out << nounitbuf << F("abcdefghijklmnopqrstuvwxyz\n");
	out << F("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
	out << F("1234567890\n");
#endif
}
