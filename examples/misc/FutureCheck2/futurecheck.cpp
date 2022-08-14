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
 * Special check for Future API (kind of unit tests) along with LifeCycle.
 * Wiring:
 * - Arduino UNO
 *   - Standard USB to console
 */

#include <fastarduino/flash.h>
#include <fastarduino/future.h>
#include <fastarduino/lifecycle.h>
#include <fastarduino/uart.h>
#include <fastarduino/streams.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/tests/assertions.h>

#ifdef ARDUINO_UNO
static const board::USART USART = board::USART::USART0;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<USART>)

using namespace future;
using namespace lifecycle;
using namespace streams;
using namespace tests;

// Buffers for UART
//==================
static const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

template<typename OUT = void, typename IN = void>
using LCFuture = LifeCycle<Future<OUT, IN>>;

// Utilities for assertions and traces
//=====================================
#define ASSERT_PROXY_STATUS(OUT, STATUS, FUTURE) assert_equals(OUT, F("" #FUTURE "->status()"), FutureStatus:: STATUS, FUTURE->status())
#define ASSERT_PROXY_ERROR(OUT, ERROR, FUTURE) assert_equals(OUT, F("" #FUTURE "->error()"), ERROR, FUTURE->error())
#define ASSERT_STATUS(OUT, STATUS, FUTURE) assert_equals(OUT, F("" #FUTURE ".status()"), FutureStatus:: STATUS, FUTURE.status())
#define ASSERT_ERROR(OUT, ERROR, FUTURE) assert_equals(OUT, F("" #FUTURE ".error()"), ERROR, FUTURE.error())
template<typename T1, typename T2> 
void assert_value(ostream& out, const flash::FlashStorage* name1, const flash::FlashStorage* name2, 
	Future<T1>& future, const T2& expected)
{
	T1 actual{};
	assert_true(out, name1, future.get(actual));
	assert_equals(out, name2, expected, actual);
}
#define ASSERT_VALUE(OUT, VALUE, FUTURE) assert_value(OUT, F("" #FUTURE ".get()"), F("" #FUTURE ".get() value"), FUTURE, VALUE)
#define ASSERT_PROXY_VALUE(OUT, VALUE, FUTURE) assert_value(OUT, F("" #FUTURE "->get()"), F("" #FUTURE "->get() value"), *FUTURE, VALUE)

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

	out << F("Before FutureManager instantiation") << endl;
	LifeCycleManager<MAX_FUTURES> manager;
	assert_equals(out, F("available slots"), MAX_FUTURES, manager.available_());

	// Check operations with move-constructor
	{
		// Check normal error context
		out << F("TEST #1 simple Future lifecycle: normal error case") << endl;
		out << F("#1.1 instantiate future") << endl;
		LCFuture<uint16_t> future1;
		ASSERT_STATUS(out, NOT_READY, future1);
		out << F("#1.2 register future") << endl;
		uint8_t id = manager.register_(future1);
		// Trace ID!
		ASSERT(out, id);
		ASSERT(out, future1.id() == id);
		ASSERT_STATUS(out, NOT_READY, future1);
		out << F("#1.3 move-constructor") << endl;
		LCFuture<uint16_t> future1b = std::move(future1);
		ASSERT_STATUS(out, INVALID, future1);
		ASSERT_STATUS(out, NOT_READY, future1b);
		out << F("#1.4 set_future_error()") << endl;
		ASSERT(out, future1b.set_future_error_(0x1111));
		ASSERT_STATUS(out, INVALID, future1);
		ASSERT_STATUS(out, ERROR, future1b);
		ASSERT_ERROR(out, 0x1111, future1b);
		ASSERT_STATUS(out, INVALID, future1);
		ASSERT_STATUS(out, ERROR, future1b);
		out << endl;
	}
	{
		// Check full data set
		out << F("TEST #2 simple Future lifecycle: new Future and full value set") << endl;
		out << F("#2.1 instantiate future") << endl;
		LCFuture<uint16_t> future2;
		ASSERT_STATUS(out, NOT_READY, future2);
		out << F("#2.2 register future") << endl;
		uint8_t id = manager.register_(future2);
		// Trace ID!
		ASSERT(out, id);
		ASSERT(out, future2.id() == id);
		ASSERT_STATUS(out, NOT_READY, future2);
		out << F("#2.3 set_future_value()") << endl;
		ASSERT(out, future2.set_future_value_(0x8000));
		out << F("#2.4 move-constructor") << endl;
		LCFuture<uint16_t> future2b = std::move(future2);
		ASSERT_STATUS(out, READY, future2);
		ASSERT_STATUS(out, READY, future2b);
		ASSERT_ERROR(out, 0, future2b);
		ASSERT_VALUE(out, 0x8000u, future2b);
		ASSERT_STATUS(out, READY, future2b);
		out << endl;
	}
	{
		// Check set value by chunks
		out << F("TEST #3 simple Future lifecycle: new Future and partial value set") << endl;
		out << F("#3.1 instantiate future") << endl;
		LCFuture<uint16_t> future3;
		ASSERT_STATUS(out, NOT_READY, future3);
		out << F("#3.2 register future") << endl;
		uint8_t id = manager.register_(future3);
		// Trace ID!
		ASSERT(out, id);
		ASSERT(out, future3.id() == id);
		ASSERT_STATUS(out, NOT_READY, future3);
		out << F("#3.3 set_future_value() chunk1") << endl;
		ASSERT(out, future3.set_future_value_(uint8_t(0x11)));
		ASSERT_STATUS(out, NOT_READY, future3);
		out << F("#3.4 move-constructor") << endl;
		LCFuture<uint16_t> future3b = std::move(future3);
		ASSERT_STATUS(out, INVALID, future3);
		ASSERT_STATUS(out, NOT_READY, future3b);
		out << F("#3.5 set_future_value() chunk2") << endl;
		ASSERT(out, future3b.set_future_value_(uint8_t(0x22)));
		ASSERT_STATUS(out, READY, future3b);
		ASSERT_VALUE(out, 0x2211u, future3b);
		ASSERT_STATUS(out, READY, future3b);
		out << endl;
	}

	// Check operations with move-assignment
	{
		// Check normal error context
		out << F("TEST #4 simple Future lifecycle: normal error case") << endl;
		out << F("#4.1 instantiate future") << endl;
		LCFuture<uint16_t> future1;
		ASSERT_STATUS(out, NOT_READY, future1);
		out << F("#4.2 register future") << endl;
		uint8_t id = manager.register_(future1);
		// Trace ID!
		ASSERT(out, id);
		ASSERT(out, future1.id() == id);
		ASSERT_STATUS(out, NOT_READY, future1);
		out << F("#4.3 move-assignment") << endl;
		LCFuture<uint16_t> future1b;;
		future1b = std::move(future1);
		ASSERT_STATUS(out, INVALID, future1);
		ASSERT_STATUS(out, NOT_READY, future1b);
		out << F("#4.4 set_future_error()") << endl;
		ASSERT(out, future1b.set_future_error_(0x1111));
		ASSERT_STATUS(out, INVALID, future1);
		ASSERT_STATUS(out, ERROR, future1b);
		ASSERT_ERROR(out, 0x1111, future1b);
		ASSERT_STATUS(out, INVALID, future1);
		ASSERT_STATUS(out, ERROR, future1b);
		out << endl;
	}
	{
		// Check full data set
		out << F("TEST #5 simple Future lifecycle: new Future and full value set") << endl;
		out << F("#5.1 instantiate future") << endl;
		LCFuture<uint16_t> future2;
		ASSERT_STATUS(out, NOT_READY, future2);
		out << F("#5.2 register future") << endl;
		uint8_t id = manager.register_(future2);
		// Trace ID!
		ASSERT(out, id);
		ASSERT(out, future2.id() == id);
		ASSERT_STATUS(out, NOT_READY, future2);
		out << F("#5.3 set_future_value()") << endl;
		ASSERT(out, future2.set_future_value_(0x8000));
		out << F("#5.4 move-assignment") << endl;
		LCFuture<uint16_t> future2b;
		future2b = std::move(future2);
		ASSERT_STATUS(out, READY, future2);
		ASSERT_STATUS(out, READY, future2b);
		ASSERT_ERROR(out, 0, future2b);
		ASSERT_VALUE(out, 0x8000u, future2b);
		ASSERT_STATUS(out, READY, future2b);
		out << endl;
	}
	{
		// Check set value by chunks
		out << F("TEST #6 simple Future lifecycle: new Future and partial value set") << endl;
		out << F("#6.1 instantiate future") << endl;
		LCFuture<uint16_t> future3;
		ASSERT_STATUS(out, NOT_READY, future3);
		out << F("#6.2 register future") << endl;
		uint8_t id = manager.register_(future3);
		// Trace ID!
		ASSERT(out, id);
		ASSERT(out, future3.id() == id);
		ASSERT_STATUS(out, NOT_READY, future3);
		out << F("#6.3 set_future_value() chunk1") << endl;
		ASSERT(out, future3.set_future_value_(uint8_t(0x11)));
		ASSERT_STATUS(out, NOT_READY, future3);
		out << F("#6.4 move-constructor") << endl;
		LCFuture<uint16_t> future3b;
		future3b = std::move(future3);
		ASSERT_STATUS(out, INVALID, future3);
		ASSERT_STATUS(out, NOT_READY, future3b);
		out << F("#6.5 set_future_value() chunk2") << endl;
		ASSERT(out, future3b.set_future_value_(uint8_t(0x22)));
		ASSERT_STATUS(out, READY, future3b);
		ASSERT_VALUE(out, 0x2211u, future3b);
		ASSERT_STATUS(out, READY, future3b);
		out << endl;
	}

	// Add checks with Proxy
	{
		// Check normal error context
		out << F("TEST #7 simple Future lifecycle: normal error case") << endl;
		out << F("#7.1 instantiate future") << endl;
		LCFuture<uint16_t> future1;
		ASSERT_STATUS(out, NOT_READY, future1);
		out << F("#7.2 register future") << endl;
		uint8_t id = manager.register_(future1);
		Proxy<Future<uint16_t>> proxy{future1};
		// Trace ID!
		ASSERT(out, id);
		ASSERT(out, proxy.id() == id);
		ASSERT_PROXY_STATUS(out, NOT_READY, proxy);
		out << F("#7.3 move-assignment") << endl;
		LCFuture<uint16_t> future1b;;
		future1b = std::move(future1);
		ASSERT_PROXY_STATUS(out, NOT_READY, proxy);
		out << F("#7.4 set_future_error()") << endl;
		ASSERT(out, proxy->set_future_error_(0x1111));
		ASSERT_PROXY_STATUS(out, ERROR, proxy);
		ASSERT_PROXY_ERROR(out, 0x1111, proxy);
		out << endl;
	}

	out << F("Finished!") << endl;
}
