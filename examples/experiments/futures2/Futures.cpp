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
 * Proof of Concept to improve Futre implementation code size.
 * Wiring:
 * - Arduino UNO
 *   - Standard USB to console
 */

//TODO rework AbstractFuture to add bool template parameter

#include <fastarduino/flash.h>
#include <fastarduino/uart.h>
#include <fastarduino/streams.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/tests/assertions.h>
#include "future.h"

#ifdef ARDUINO_UNO
static const board::USART USART = board::USART::USART0;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

#define STATIC

using namespace future;
using namespace streams;

// Buffers for UART
//==================
static const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

// Utilities for assertions and traces
//=====================================
#define ECHO_ID(OUT, FUTURE) OUT << F("" #FUTURE ".id() = ") << FUTURE.id() << endl
#define ASSERT_STATUS(OUT, STATUS, FUTURE) assert(OUT, F("" #FUTURE ".status()"), FutureStatus:: STATUS, FUTURE.status())
#define ASSERT_ERROR(OUT, ERROR, FUTURE) assert(OUT, F("" #FUTURE ".error()"), ERROR, FUTURE.error())
template<typename T1, typename T2, typename T3> 
void assert_value(ostream& out, const flash::FlashStorage* name1, const flash::FlashStorage* name2, 
	Future<T1, T3>& future, const T2& expected)
{
	T1 actual{};
	assert(out, name1, future.get(actual));
	assert(out, name2, expected, actual);
}
#define ASSERT_VALUE(OUT, VALUE, FUTURE) assert_value(OUT, F("" #FUTURE ".get()"), F("" #FUTURE ".get() value"), FUTURE, VALUE)
template<typename T> void trace_future(ostream& out, const Future<T>& future)
{
	out << F("Future id = ") << dec << future.id() << F(", status = ") << future.status() << endl;
}

static constexpr uint8_t MAX_FUTURES = 64;

int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	// Initialize debugging output
	serial::hard::UATX<board::USART::USART0> uart{output_buffer};
	uart.begin(115200);
	ostream out = uart.out();
	out << boolalpha << showbase;

	FutureManager<MAX_FUTURES> manager{};

	{
		out << F("TEST #1 Future<uint16_t, void, STATIC>") << endl;
		Future<uint16_t, void> future;
		ASSERT_STATUS(out, INVALID, future);
		ASSERT(out, manager.register_future(future));
		ECHO_ID(out, future);
		ASSERT_STATUS(out, NOT_READY, future);
		ASSERT(out, manager.set_future_value(future.id(), uint8_t(0x00)));
		ASSERT_STATUS(out, NOT_READY, future);
		ASSERT(out, manager.set_future_value(future.id(), uint8_t(0x80)));
		ASSERT_STATUS(out, READY, future);
		ASSERT_ERROR(out, 0, future);
		ASSERT_STATUS(out, READY, future);
		ASSERT_VALUE(out, 0x8000u, future);
		ASSERT_STATUS(out, INVALID, future);
		ASSERT_ERROR(out, errors::EINVAL, future);
		ASSERT_STATUS(out, INVALID, future);
		out << endl;
	}

	{
		out << F("TEST #2 Future<void, uint16_t, STATIC>") << endl;
		Future<void, uint16_t> future;
		ASSERT_STATUS(out, INVALID, future);
		ASSERT(out, manager.register_future(future));
		ECHO_ID(out, future);
		ASSERT_STATUS(out, NOT_READY, future);
		uint8_t value;
		ASSERT(out, manager.get_storage_value(future.id(), value));
		ASSERT_STATUS(out, NOT_READY, future);
		ASSERT(out, manager.set_future_finish(future.id()));
		ASSERT_STATUS(out, READY, future);
		ASSERT_ERROR(out, 0, future);
		ASSERT_STATUS(out, READY, future);
		out << endl;
	}

	{
		out << F("TEST #3 Future<uint16_t, uint16_t, STATIC>") << endl;
		Future<uint16_t, uint16_t> future;
		ASSERT_STATUS(out, INVALID, future);
		ASSERT(out, manager.register_future(future));
		ECHO_ID(out, future);
		ASSERT_STATUS(out, NOT_READY, future);
		uint8_t value;
		ASSERT(out, manager.get_storage_value(future.id(), value));
		ASSERT_STATUS(out, NOT_READY, future);
		ASSERT(out, manager.set_future_value(future.id(), uint8_t(0x00)));
		ASSERT_STATUS(out, NOT_READY, future);
		ASSERT(out, manager.set_future_value(future.id(), uint8_t(0x80)));
		ASSERT_STATUS(out, READY, future);
		ASSERT_ERROR(out, 0, future);
		ASSERT_STATUS(out, READY, future);
		ASSERT_VALUE(out, 0x8000u, future);
		ASSERT_STATUS(out, INVALID, future);
		ASSERT_ERROR(out, errors::EINVAL, future);
		ASSERT_STATUS(out, INVALID, future);
		out << endl;
	}

}
