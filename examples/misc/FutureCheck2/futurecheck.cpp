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
 * Code size check for Future API default FutureManager (moveable Future).
 * Wiring:
 * - Arduino UNO
 *   - Standard USB to console
 */

#include <fastarduino/flash.h>
#include <fastarduino/future.h>
#include <fastarduino/iomanip.h>

#ifndef ARDUINO_UNO
#error "Current target is not yet supported!"
#endif

#define NO_OUTPUT true

#if NO_OUTPUT
#include <fastarduino/empty_streams.h>
using OSTREAM = streams::null_ostream;
void assert(UNUSED OSTREAM& out, UNUSED const char* message, UNUSED bool condition) {}
void assert(UNUSED OSTREAM& out, UNUSED const flash::FlashStorage* message, UNUSED bool condition) {}
#define ASSERT(OUT, CONDITION) assert(OUT, F("" #CONDITION ""), CONDITION)
template<typename T1, typename T2>
void assert(UNUSED OSTREAM& out, UNUSED const char* var, UNUSED T1 expected, UNUSED T2 actual) {}
template<typename T1, typename T2>
void assert(UNUSED OSTREAM& out, UNUSED const flash::FlashStorage* var, UNUSED T1 expected, UNUSED T2 actual) {}
#else
#include <fastarduino/uart.h>
#include <fastarduino/streams.h>
#include <fastarduino/tests/assertions.h>
static const board::USART USART = board::USART::USART0;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
using OSTREAM = streams::ostream;
static const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];
#endif

using namespace future;
using namespace streams;

static constexpr uint8_t MAX_FUTURES = 1;
using FUTURE_MANAGER = FutureManager<MAX_FUTURES>;
template<typename OUT = void, typename IN = void> using FUTURE = FUTURE_MANAGER::FUTURE<OUT, IN>;

// Utilities for assertions and traces
//=====================================
#define ECHO_ID(OUT, FUTURE) OUT << F("" #FUTURE ".id() = ") << FUTURE.id() << endl
#define ASSERT_STATUS(OUT, STATUS, FUTURE) assert(OUT, F("" #FUTURE ".status()"), FutureStatus:: STATUS, FUTURE.status())
#define ASSERT_ERROR(OUT, ERROR, FUTURE) assert(OUT, F("" #FUTURE ".error()"), ERROR, FUTURE.error())
template<typename T1, typename T2> 
void assert_value(OSTREAM& out, const flash::FlashStorage* name1, const flash::FlashStorage* name2, 
	FUTURE<T1>& future, const T2& expected)
{
	T1 actual{};
	assert(out, name1, future.get(actual));
	assert(out, name2, expected, actual);
}
#define ASSERT_VALUE(OUT, VALUE, FUTURE) assert_value(OUT, F("" #FUTURE ".get()"), F("" #FUTURE ".get() value"), FUTURE, VALUE)
template<typename T> void trace_future(OSTREAM& out, const FUTURE<T>& future)
{
	out << F("Future id = ") << dec << future.id() << F(", status = ") << future.status() << endl;
}

int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

#if NO_OUTPUT
	OSTREAM out;
#else
	// Initialize debugging output
	serial::hard::UATX<board::USART::USART0> uart{output_buffer};
	uart.begin(115200);
	OSTREAM out = uart.out();
#endif
	out << boolalpha << showbase;

	out << F("Before FutureManager instantiation") << endl;
	FUTURE_MANAGER manager{};
	assert(out, F("available futures"), MAX_FUTURES, manager.available_futures());

	{
		// Check normal error context
		out << F("TEST #1 simple Future lifecycle: normal error case") << endl;
		out << F("#1.1 instantiate future") << endl;
		FUTURE<uint16_t> future;
		ASSERT_STATUS(out, INVALID, future);
		out << F("#1.2 register_future()") << endl;
		ASSERT(out, manager.register_future(future));
		ECHO_ID(out, future);
		ASSERT_STATUS(out, NOT_READY, future);
		assert(out, F("available futures"), MAX_FUTURES - 1, manager.available_futures());
		out << F("#1.3 set_future_error()") << endl;
		ASSERT(out, manager.set_future_error(future.id(), 0x1111));
		ASSERT_STATUS(out, ERROR, future);
		ASSERT_ERROR(out, 0x1111, future);
		ASSERT_STATUS(out, INVALID, future);
		out << endl;
	}

	{
		// Check full data set
		out << F("TEST #2 simple Future lifecycle: new Future and full value set") << endl;
		out << F("#2.1 instantiate future") << endl;
		FUTURE<uint16_t> future;
		ASSERT_STATUS(out, INVALID, future);
		out << F("#2.2 register_future()") << endl;
		ASSERT(out, manager.register_future(future));
		ECHO_ID(out, future);
		ASSERT_STATUS(out, NOT_READY, future);
		out << F("#2.3 set_future_value()") << endl;
		const uint16_t VAL = 0x8000u;
		ASSERT(out, manager.set_future_value(future.id(), reinterpret_cast<const uint8_t*>(&VAL), sizeof(VAL)));
		ASSERT_STATUS(out, READY, future);
		ASSERT_ERROR(out, 0, future);
		ASSERT_STATUS(out, READY, future);
		ASSERT_VALUE(out, VAL, future);
		ASSERT_STATUS(out, INVALID, future);
		ASSERT_ERROR(out, errors::EINVAL, future);
		ASSERT_STATUS(out, INVALID, future);
		out << endl;
	}

	{
		// Check reuse of a future in various states
		out << F("TEST #3 check Future status after move assignment") << endl;
		out << F("#3.1 instantiate futures") << endl;
		FUTURE<uint16_t> future1, future2;
		out << F("#3.2 register future") << endl;
		ASSERT(out, manager.register_future(future1));
		ECHO_ID(out, future1);
		ASSERT_STATUS(out, NOT_READY, future1);
		out << F("#3.3 check status (READY, INVALID) -> (INVALID, READY)") << endl;
		const uint16_t VAL = 0x33BB;
		ASSERT(out, manager.set_future_value(future1.id(), reinterpret_cast<const uint8_t*>(&VAL), sizeof(VAL)));
		future2 = std::move(future1);
		ASSERT_STATUS(out, INVALID, future1);
		ASSERT_STATUS(out, READY, future2);
		ASSERT_VALUE(out, VAL, future2);
	}
}
