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
 * Special compile check for iostreams: can normal streams and empty streams be mixed in program?
 * Also, empty streams should generate no code at all.
 * This program is not aimed for upload, just build.
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/streams.h>
#include <fastarduino/empty_streams.h>
#include <fastarduino/iomanip.h>

// Buffer for normal stream
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace streams;

template<typename STREAM> void check(STREAM& out)
{
	// 1. Check extractors exist for all types
	out << F("Hello") << ' ' << "World" << endl;
	out << (void*) 0 << true << ' ' << 123U << 123456UL << -123 << -123456L << 123.456 << endl; 
	// 2. Check all simple output manipulators are supported
	out << flush;
	out << dec << 123 << endl;
	out << hex << 123 << endl;
	out << bin << 123 << endl;
	out << oct << 123 << endl;
	out << boolalpha << false << endl;
	out << noboolalpha << true << endl;
	out << hex << showbase << 123 << endl;
	out << hex << noshowbase << 123 << endl;
	out << showpos << 123 << endl;
	out << noshowpos << 123 << endl;
	out << hex << uppercase << 123 << endl;
	out << hex << nouppercase << 123 << endl;
	out << unitbuf << "123\n";
	out << nouppercase << "123\n" << flush;
	out << fixed << 123.0 << endl;
	out << scientific << 123.0 << endl;
	out << defaultfloat << 123.0 << endl;
	// 3. Check paramterized manipulators
	out << setw(10) << setfill('~') << left << "left" << endl;
	out << setw(10) << setfill(' ') << right << "right" << endl;
	out << setbase(10) << 123 << endl;
	out << setprecision(5) << 123.4 << endl;
	out << setiosflags(ios::boolalpha | ios::uppercase); 
	out << resetiosflags(ios::boolalpha | ios::uppercase); 
}

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	ostreambuf buf{output_buffer};
	ostream out{buf};
	check(out);

	null_ostream nul;
	check(nul);

	return 0;
}
