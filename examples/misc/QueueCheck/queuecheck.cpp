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
 * Special check for Queue container (kind of unit tests).
 * Wiring:
 * - Arduino UNO
 *   - Standard USB to console
 */

#include <string.h>

#include <fastarduino/flash.h>
#include <fastarduino/queue.h>
#include <fastarduino/uart.h>
#include <fastarduino/streams.h>
#include <fastarduino/iomanip.h>

#ifdef ARDUINO_UNO
static const board::USART USART = board::USART::USART0;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<USART>)
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

static const uint8_t QUEUE_SIZE = 9;
static const uint8_t BUFFER_SIZE = QUEUE_SIZE + 1;
// static const uint8_t BUFFER_SIZE = QUEUE_SIZE;

using namespace streams;
using namespace containers;

template<typename T1, typename T2> void assert(ostream& out, const flash::FlashStorage* var, T1 expected, T2 actual)
{
	out << F("    Comparing ") << var;
	if (expected == actual)
		out << F(" OK: ") << expected << endl;
	else
		out << F(" KO exp=") << expected << F(" act=") << actual << endl;
}

void assert(ostream& out, const Queue<char>& queue, bool empty, bool full, uint8_t items, uint8_t free)
{
	assert(out, F("empty()"), empty, queue.empty());
	assert(out, F("full()"), full, queue.full());
	assert(out, F("items()"), items, queue.items());
	assert(out, F("free()"), free, queue.free());
}

// 0-initialized buffers for peek(0 checks)
static char peek_buffer5[5];
static char peek_buffer15[15];
static char peek_buffer20[20];

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
	char val;

	out << F("New empty queue") << endl;
	assert(out, F("size()"), QUEUE_SIZE, queue.size());
	assert(out, queue, true, false, 0, QUEUE_SIZE);
	assert(out, F("peek(c)"), false, queue.peek(val));
	assert(out, queue, true, false, 0, QUEUE_SIZE);

	out << F("Push 1 char") << endl;
	queue.push('a');
	assert(out, queue, false, false, 1, QUEUE_SIZE - 1);

	out << F("Pull 1 char") << endl;
	assert(out, F("pull()"), true, queue.pull(val));
	assert(out, queue, true, false, 0, QUEUE_SIZE);

	out << F("Push 9 chars") << endl;
	queue.push('1');
	queue.push('2');
	queue.push('3');
	queue.push('4');
	queue.push('5');
	queue.push('6');
	queue.push('7');
	queue.push('8');
	queue.push('9');
	assert(out, queue, false, true, QUEUE_SIZE, 0);
	out << F("Push extra char") << endl;
	bool result = queue.push('A');
	assert(out, F("1st extra push"), false, result);
	assert(out, queue, false, true, QUEUE_SIZE, 0);
	out << F("Push extra char") << endl;
	result = queue.push('B');
	assert(out, F("2nd extra push"), false, result);
	assert(out, queue, false, true, QUEUE_SIZE, 0);

	out << F("Peek functions") << endl;
	assert(out, F("peek(c)"), true, queue.peek(val));
	assert(out, F("peeked c"), '1', val);
	assert(out, queue, false, true, QUEUE_SIZE, 0);
	assert(out, F("peek(c)"), true, queue.peek(val));
	assert(out, F("peeked c"), '1', val);
	assert(out, queue, false, true, QUEUE_SIZE, 0);

	assert(out, F("peek(buf[5])"), 5, queue.peek(peek_buffer5));
	assert(out, queue, false, true, QUEUE_SIZE, 0);

	assert(out, F("peek(buf[15])"), 9, queue.peek(peek_buffer15));
	assert(out, F("peeked buf[15] Vs \"123456789\""), 0, strcmp(peek_buffer15, "123456789"));
	assert(out, queue, false, true, QUEUE_SIZE, 0);

	assert(out, F("peek(buf, 5)"), 5, queue.peek(peek_buffer20, 5));
	assert(out, F("peeked buf Vs \"12345\""), 0, strcmp(peek_buffer20, "12345"));
	out << F("peek_buffer20 = '") << peek_buffer20 << "'" << endl;
	assert(out, queue, false, true, QUEUE_SIZE, 0);

	out << F("Pull 8 chars") << endl;
	assert(out, F("pull() 1"), true, queue.pull(val));
	assert(out, F("pull() 2"), true, queue.pull(val));
	assert(out, F("pull() 3"), true, queue.pull(val));
	assert(out, F("pull() 4"), true, queue.pull(val));
	assert(out, F("pull() 5"), true, queue.pull(val));
	assert(out, F("pull() 6"), true, queue.pull(val));
	assert(out, F("pull() 7"), true, queue.pull(val));
	assert(out, F("pull() 8"), true, queue.pull(val));
	assert(out, queue, false, false, 1, QUEUE_SIZE - 1);

	// repush new chars to check content after the ring buffer has been "rounded" 
	out << F("Push 3 chars") << endl;
	queue.push('A');
	queue.push('B');
	queue.push('C');
	assert(out, queue, false, false, 4, QUEUE_SIZE - 4);
	memset(peek_buffer5, 0, 5);
	assert(out, F("peek(buf[5])"), 4, queue.peek(peek_buffer5));
	assert(out, F("peeked buf[5] Vs \"9ABC\""), 0, strcmp(peek_buffer5, "9ABC"));
	assert(out, queue, false, false, 4, QUEUE_SIZE - 4);

	// push more chars to ensure we fill up the queue again
	out << F("Push 5 chars") << endl;
	queue.push('D');
	queue.push('E');
	queue.push('F');
	queue.push('G');
	queue.push('H');
	assert(out, queue, false, true, QUEUE_SIZE, 0);
	out << F("Push extra char") << endl;
	result = queue.push('I');
	assert(out, F("extra push"), false, result);
	assert(out, queue, false, true, QUEUE_SIZE, 0);

	out << F("Pull 8 chars") << endl;
	assert(out, F("pull() 9"), true, queue.pull(val));
	assert(out, F("pull() 9"), '9', val);
	assert(out, F("pull() A"), true, queue.pull(val));
	assert(out, F("pull() A"), 'A', val);
	assert(out, F("pull() B"), true, queue.pull(val));
	assert(out, F("pull() B"), 'B', val);
	assert(out, F("pull() C"), true, queue.pull(val));
	assert(out, F("pull() C"), 'C', val);
	assert(out, F("pull() D"), true, queue.pull(val));
	assert(out, F("pull() D"), 'D', val);
	assert(out, F("pull() E"), true, queue.pull(val));
	assert(out, F("pull() E"), 'E', val);
	assert(out, F("pull() F"), true, queue.pull(val));
	assert(out, F("pull() F"), 'F', val);
	assert(out, F("pull() G"), true, queue.pull(val));
	assert(out, F("pull() G"), 'G', val);
	assert(out, queue, false, false, 1, QUEUE_SIZE - 1);

	out << F("Pull last char") << endl;
	assert(out, F("pull() H"), true, queue.pull(val));
	assert(out, F("pull() H"), 'H', val);
	assert(out, queue, true, false, 0, QUEUE_SIZE);

	out << F("Pull no char") << endl;
	assert(out, F("pull()"), false, queue.pull(val));
	assert(out, queue, true, false, 0, QUEUE_SIZE);

	return 0;
}
