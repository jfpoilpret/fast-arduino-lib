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
 * Special check for Queue container.
 */

#include <fastarduino/queue.h>
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
using namespace containers;

template<typename T1, typename T2> void assert(ostream& out, const char* var, T1 expected, T2 actual)
{
	out << "    Comparing " << var;
	if (expected == actual)
		out << " OK: " << expected << endl;
	else
		out << " KO exp=" << expected << " act=" << actual << endl;
}

void assert(ostream& out, const Queue<char>& queue, bool empty, bool full, uint8_t items, uint8_t free)
{
	assert(out, "empty()", empty, queue.empty());
	assert(out, "full()", full, queue.full());
	assert(out, "items()", items, queue.items());
	assert(out, "free()", free, queue.free());
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

	// Create a queue and operate on it
	char buffer[BUFFER_SIZE];
	Queue<char> queue{buffer};
	out << "New empty queue" << endl;
	assert(out, "size()", BUFFER_SIZE - 1, queue.size());
	assert(out, queue, true, false, 0, BUFFER_SIZE - 1);

	out << "Push 1 char" << endl;
	queue.push('a');
	assert(out, queue, false, false, 1, BUFFER_SIZE - 2);

	out << "Pull 1 char" << endl;
	char val;
	assert(out, "pull()", true, queue.pull(val));
	assert(out, queue, true, false, 0, BUFFER_SIZE - 1);

	out << "Push 9 chars" << endl;
	queue.push('1');
	queue.push('2');
	queue.push('3');
	queue.push('4');
	queue.push('5');
	queue.push('6');
	queue.push('7');
	queue.push('8');
	queue.push('9');
	assert(out, queue, false, true, BUFFER_SIZE - 1, 0);

	out << "Pull 8 chars" << endl;
	assert(out, "pull() 1", true, queue.pull(val));
	assert(out, "pull() 2", true, queue.pull(val));
	assert(out, "pull() 3", true, queue.pull(val));
	assert(out, "pull() 4", true, queue.pull(val));
	assert(out, "pull() 5", true, queue.pull(val));
	assert(out, "pull() 6", true, queue.pull(val));
	assert(out, "pull() 7", true, queue.pull(val));
	assert(out, "pull() 8", true, queue.pull(val));
	assert(out, queue, false, false, 1, BUFFER_SIZE - 2);

	out << "Pull last char" << endl;
	assert(out, "pull() 9", true, queue.pull(val));
	assert(out, queue, true, false, 0, BUFFER_SIZE - 1);

	out << "Pull no char" << endl;
	assert(out, "pull()", false, queue.pull(val));
	assert(out, queue, true, false, 0, BUFFER_SIZE - 1);

	return 0;
}
