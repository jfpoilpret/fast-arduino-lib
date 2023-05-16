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
 * Special check for LinkedList container (kind of unit tests).
 * Wiring:
 * - Arduino UNO
 *   - Standard USB to console
 */

#include <fastarduino/linked_list.h>
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
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

static const uint8_t BUFFER_SIZE = 10;

using namespace streams;
using namespace containers;

// Link items classes
//====================
class Link1 : public Link<Link1>
{
public:
	Link1(uint32_t value) : value{value} {}

	const uint32_t value;
};

class Link2
{
public:
	Link2(uint32_t value) : value{value} {}

	const uint32_t value;
};

// Define the 3 types of links used on this test
//===============================================

// Link items obtained by subclassing Link
using LINK1 = Link1;
// Link items obtained by wrapping in LinkWrapper
using LINK2 = LinkWrapper<Link2>;
// Simple chars as wrapped links
using LINK3 = LinkWrapper<char, char, char>;

// Operators required for assertions
//===================================
ostream& operator<<(ostream& out, const LINK1& item)
{
	return out << item.value;
}
ostream& operator<<(ostream& out, const LINK2& item)
{
	return out << item.item().value;
}
ostream& operator<<(ostream& out, const LINK3& item)
{
	return out << item.item();
}
bool operator==(const LINK1& item1, const LINK1& item2)
{
	return item1.value == item2.value;
}
bool operator==(const LINK2& item1, const LINK2& item2)
{
	return item1.item().value == item2.item().value;
}
bool operator==(const LINK3& item1, const LINK3& item2)
{
	return item1.item() == item2.item();
}

// Generic assertion
//===================
template<typename T1, typename T2> void assert(ostream& out, const char* var, const T1& expected, const T2& actual)
{
	out << "    Comparing " << var;
	if (expected == actual)
		out << " OK: " << expected << endl;
	else
		out << " KO exp=" << expected << " act=" << actual << endl;
}

// Traveral class running assertions against expected content
//============================================================
template<typename ITEM> class TraversalAssert
{
public:
	TraversalAssert(ostream& out)
	: out{out}, index{}, expected{}, size{} {}

	TraversalAssert(ostream& out, uint8_t size, ITEM** expected)
	: out{out}, index{}, expected{expected}, size{size} {}

	bool operator()(ITEM& item)
	{
		if (index < size)
		{
			char buf[] = "item[x]";
			buf[5] = '0' + index;
			assert(out, buf, *expected[index], item);
		}
		else
		{
			out << "    KO -> Too many items in list!" << endl;
		}
		++index;
		return false;
	}

private:
	ostream& out;
	uint8_t index;
	ITEM** expected;
	uint8_t size;
};

// Variables used in tests
//=========================
static LINK1 links1[] = {123456UL, 0UL, 123UL, 456UL, 654321UL};
static LINK2 links2[] = {LINK2(123456UL), LINK2(0UL), LINK2(123UL), LINK2(456UL), LINK2(654321UL)};
static LINK3 links3[] = {'a', 'b', 'c', 'd', 'e'};

// List-checking functions
//=========================
template<typename LINK> void check_link_list(ostream& out, LINK* links)
{
	LinkedList<LINK> list;
	list.traverse(TraversalAssert<LINK>{out});

	out << "after insert() #1" << endl;
	list.insert(links[0]);
	LINK* expected1[] = {&links[0]};
	list.traverse(TraversalAssert<LINK>{out, 1, expected1});

	out << "after insert() #2" << endl;
	list.insert(links[1]);
	LINK* expected2[] = {&links[1], &links[0]};
	list.traverse(TraversalAssert<LINK>{out, 2, expected2});

	out << "after insert() #3,4,5" << endl;
	list.insert(links[2]);
	list.insert(links[3]);
	list.insert(links[4]);
	LINK* expected3[] = {&links[4], &links[3], &links[2], &links[1], &links[0]};
	list.traverse(TraversalAssert<LINK>{out, 5, expected3});

	out << "after remove() #3" << endl;
	list.remove(links[2]);
	LINK* expected4[] = {&links[4], &links[3], &links[1], &links[0]};
	list.traverse(TraversalAssert<LINK>{out, 4, expected4});

	out << "after remove() #3 second time" << endl;
	list.remove(links[2]);
	list.traverse(TraversalAssert<LINK>{out, 4, expected4});

	out << "after remove() #1" << endl;
	list.remove(links[0]);
	LINK* expected5[] = {&links[4], &links[3], &links[1]};
	list.traverse(TraversalAssert<LINK>{out, 3, expected5});

	out << "after remove() #5" << endl;
	list.remove(links[4]);
	LINK* expected6[] = {&links[3], &links[1]};
	list.traverse(TraversalAssert<LINK>{out, 2, expected6});

	out << "after remove all" << endl;
	list.remove(links[1]);
	list.remove(links[3]);
	list.traverse(TraversalAssert<LINK>{out});

	out << endl;
}

void check_link1(ostream& out)
{
	// Create a list of Link1 and operate on it
	out << "list of Link<uint32_t>" << endl;
	check_link_list(out, links1);
}

void check_link2(ostream& out)
{
	// Create a list of Link1 and operate on it
	out << "list of LinkWrapper<Link1>" << endl;
	check_link_list(out, links2);
}

void check_link3(ostream& out)
{
	// Create a list of Link1 and operate on it
	out << "list of LinkWrapper<char>" << endl;
	check_link_list(out, links3);
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

	check_link1(out);
	check_link2(out);
	check_link3(out);

	return 0;
}
