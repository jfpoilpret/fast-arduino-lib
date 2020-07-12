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
 * Special check for Future API (kind of unit tests).
 * Wiring:
 * - Arduino UNO
 *   - Standard USB to console
 */

#include <fastarduino/flash.h>
#include <fastarduino/future.h>
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

using namespace future;
using namespace streams;

// Add utility ostream manipulator for FutureStatus
//==================================================
static const flash::FlashStorage* convert(FutureStatus s)
{
	switch (s)
	{
		case FutureStatus::NOT_READY:
		return F("NOT_READY");

		case FutureStatus::READY:
		return F("READY");

		case FutureStatus::ERROR:
		return F("ERROR");
	}
}
ostream& operator<<(ostream& out, FutureStatus s)
{
	return out << convert(s);
}

// Buffers for UART
//==================
static const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

// Future subclass for specific check
//====================================
class MyFuture : public Future<uint16_t>
{
public:
	MyFuture() : Future<uint16_t>{} {}
	~MyFuture() = default;

	MyFuture(MyFuture&& that) : Future<uint16_t>{std::move(that)}
	{
	}
	MyFuture& operator=(MyFuture&& that)
	{
		if (this == &that) return *this;
		(Future<uint16_t>&) *this = std::move(that);
		return *this;
	}

	MyFuture(const MyFuture&) = delete;
	MyFuture& operator=(const MyFuture&) = delete;

	// The following method is blocking until this Future is ready
	bool get(uint16_t& result)
	{
		uint16_t temp = 0;
		if (this->Future<uint16_t>::get(temp))
		{
			result = temp * 10;
			return true;
		}
		return false;
	}
};

// Utilities for assertions and traces
//=====================================
#define ASSERT_STATUS(OUT, STATUS, FUTURE) assert(OUT, F("" #FUTURE ".status()"), FutureStatus:: STATUS, FUTURE.status())
#define ASSERT_ERROR(OUT, ERROR, FUTURE) assert(OUT, F("" #FUTURE ".error()"), ERROR, FUTURE.error())
template<typename T1, typename T2> 
void assert_value(ostream& out, const flash::FlashStorage* name1, const flash::FlashStorage* name2, 
	Future<T1>& future, const T2& expected)
{
	T1 actual{};
	assert(out, name1, future.get(actual));
	assert(out, name2, expected, actual);
}
#define ASSERT_VALUE(OUT, VALUE, FUTURE) assert_value(OUT, F("" #FUTURE ".get()"), F("" #FUTURE ".get() value"), FUTURE, VALUE)

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

	// Check normal error context
	out << F("TEST #1 simple Future lifecycle: normal error case") << endl;
	out << F("#1.1 instantiate future") << endl;
	Future<uint16_t> future1;
	ASSERT_STATUS(out, NOT_READY, future1);
	out << F("#1.3 set_future_error()") << endl;
	ASSERT(out, future1.set_future_error_(0x1111));
	ASSERT_STATUS(out, ERROR, future1);
	ASSERT_ERROR(out, 0x1111, future1);
	ASSERT_STATUS(out, ERROR, future1);
	out << endl;

	// Check full data set
	out << F("TEST #2 simple Future lifecycle: new Future and full value set") << endl;
	out << F("#2.1 instantiate future") << endl;
	Future<uint16_t> future2;
	ASSERT_STATUS(out, NOT_READY, future2);
	out << F("#2.3 set_future_value()") << endl;
	ASSERT(out, future2.set_future_value_(0x8000));
	ASSERT_STATUS(out, READY, future2);
	ASSERT_ERROR(out, 0, future2);
	ASSERT_STATUS(out, READY, future2);
	ASSERT_VALUE(out, 0x8000u, future2);
	ASSERT_STATUS(out, READY, future2);
	ASSERT_ERROR(out, 0, future2);
	ASSERT_STATUS(out, READY, future2);
	out << endl;

	// Check set value by chunks
	out << F("TEST #3 simple Future lifecycle: new Future and partial value set") << endl;
	out << F("#3.1 instantiate future") << endl;
	Future<uint16_t> future3;
	ASSERT_STATUS(out, NOT_READY, future3);
	out << F("#3.3 set_future_value() chunk1") << endl;
	ASSERT(out, future3.set_future_value_(uint8_t(0x11)));
	ASSERT_STATUS(out, NOT_READY, future3);
	out << F("#3.4 set_future_value() chunk2") << endl;
	ASSERT(out, future3.set_future_value_(uint8_t(0x22)));
	ASSERT_STATUS(out, READY, future3);
	ASSERT_VALUE(out, 0x2211u, future3);
	ASSERT_STATUS(out, READY, future3);
	out << endl;

	// Check set value by data pointer once
	out << F("TEST #4 simple Future lifecycle: new Future and full value pointer set") << endl;
	out << F("#4.1 instantiate future") << endl;
	Future<uint16_t> future4;
	ASSERT_STATUS(out, NOT_READY, future4);
	out << F("#4.3 set_future_value() from ptr") << endl;
	const uint16_t constant1 = 0x4433;
	ASSERT(out, future4.set_future_value_((const uint8_t*) &constant1, sizeof(constant1)));
	ASSERT_STATUS(out, READY, future4);
	ASSERT_VALUE(out, 0x4433u, future4);
	ASSERT_STATUS(out, READY, future4);
	out << endl;

	// Check set value by data pointer twice
	out << F("TEST #5 simple Future lifecycle: new Future and part value pointer set") << endl;
	out << F("#5.1 instantiate future") << endl;
	Future<uint16_t> future5;
	ASSERT_STATUS(out, NOT_READY, future5);
	out << F("#5.3 set_future_value() from ptr (1 byte)") << endl;
	const uint16_t constant2 = 0x5566;
	ASSERT(out, future5.set_future_value_((const uint8_t*) &constant2, 1));
	ASSERT_STATUS(out, NOT_READY, future5);
	out << F("#5.4 set_future_value() from ptr (2nd byte)") << endl;
	ASSERT(out, future5.set_future_value_(((const uint8_t*) &constant2) + 1, 1));
	ASSERT_STATUS(out, READY, future5);
	ASSERT_VALUE(out, 0x5566u, future5);
	ASSERT_STATUS(out, READY, future5);
	out << endl;

	// Check further updates do not do anything (and do not crash either!)
	out << F("TEST #6 simple Future lifecycle: check no more updates possible after first set complete") << endl;
	out << F("#6.1 instantiate future") << endl;
	Future<uint16_t> future6;
	ASSERT_STATUS(out, NOT_READY, future6);
	out << F("#6.3 set_future_value() from full value") << endl;
	ASSERT(out, future6.set_future_value_(0x8899));
	ASSERT_STATUS(out, READY, future6);
	out << F("#6.4 set_future_value() additional byte") << endl;
	ASSERT(out, !future6.set_future_value_(uint8_t(0xAA)));
	ASSERT_STATUS(out, READY, future6);
	ASSERT_VALUE(out, 0x8899u, future6);
	ASSERT_STATUS(out, READY, future6);
	out << F("#6.5 set_future_value() after get() additional byte") << endl;
	ASSERT(out, !future6.set_future_value_(uint8_t(0xBB)));
	ASSERT_STATUS(out, READY, future6);
	out << endl;

	// Check reuse of a future in various states
	out << F("TEST #7 check Future status after move constructor") << endl;
	out << F("#7.1 instantiate future") << endl;
	Future<uint16_t> future7;
	ASSERT_STATUS(out, NOT_READY, future7);
	out << F("#7.3 check status (NOT_READY, INVALID) -> (INVALID, NOT_READY)") << endl;
	Future<uint16_t> future8 = std::move(future7);
	// ASSERT_STATUS(out, INVALID, future7);
	ASSERT_STATUS(out, NOT_READY, future8);
	out << F("#7.4 check status (READY, INVALID) -> (INVALID, READY)") << endl;
	ASSERT(out, future8.set_future_value_(0xFFFFu));
	Future<uint16_t> future9 = std::move(future8);
	// ASSERT_STATUS(out, INVALID, future8);
	ASSERT_STATUS(out, READY, future9);
	ASSERT_VALUE(out, 0xFFFFu, future9);
	out << F("#7.5 check status (ERROR, INVALID) -> (INVALID, ERROR)") << endl;
	Future<uint16_t> future10;
	ASSERT(out, future10.set_future_error_(-10000));
	Future<uint16_t> future11 = std::move(future10);
	// ASSERT_STATUS(out, INVALID, future10);
	ASSERT_STATUS(out, ERROR, future11);
	ASSERT_ERROR(out, -10000, future11);
	out << F("#7.6 check status (INVALID, INVALID) -> (INVALID, INVALID)") << endl;
	Future<uint16_t> future12;
	Future<uint16_t> future13 = std::move(future12);
	// ASSERT_STATUS(out, INVALID, future12);
	// ASSERT_STATUS(out, INVALID, future13);
	out << F("#7.7 check status (partial NOT_READY, INVALID) -> (INVALID, partial NOT_READY)") << endl;
	Future<uint16_t> future14;
	ASSERT(out, future14.set_future_value_(uint8_t(0xBB)));
	Future<uint16_t> future15 = std::move(future14);
	// ASSERT_STATUS(out, INVALID, future14);
	ASSERT_STATUS(out, NOT_READY, future15);
	ASSERT(out, future15.set_future_value_(uint8_t(0xCC)));
	out << F("#7.8 Complete set value") << endl;
	ASSERT_STATUS(out, READY, future15);
	ASSERT_ERROR(out, 0, future15);
	ASSERT_VALUE(out, 0xCCBBu, future15);
	out << endl;

	// Check reuse of a future in various states
	out << F("TEST #8 check Future status after move assignment") << endl;
	out << F("#8.1 instantiate futures") << endl;
	Future<uint16_t> future17, future18, future19, future20, future21, future22, future23, future24, future25;
	out << F("#8.2 register future") << endl;
	ASSERT_STATUS(out, NOT_READY, future17);
	out << F("#8.3 check status (NOT_READY, INVALID) -> (INVALID, NOT_READY)") << endl;
	future18 = std::move(future17);
	// ASSERT_STATUS(out, INVALID, future17);
	ASSERT_STATUS(out, NOT_READY, future18);
	out << F("#8.4 check status (READY, INVALID) -> (INVALID, READY)") << endl;
	ASSERT(out, future18.set_future_value_(0xFFFFu));
	future19 = std::move(future18);
	// ASSERT_STATUS(out, INVALID, future18);
	ASSERT_STATUS(out, READY, future19);
	ASSERT_VALUE(out, 0xFFFFu, future19);
	out << F("#8.5 check status (ERROR, INVALID) -> (INVALID, ERROR)") << endl;
	ASSERT(out, future20.set_future_error_(-10000));
	future21 = std::move(future20);
	// ASSERT_STATUS(out, INVALID, future20);
	ASSERT_STATUS(out, ERROR, future21);
	ASSERT_ERROR(out, -10000, future21);
	out << F("#8.6 check status (INVALID, INVALID) -> (INVALID, INVALID)") << endl;
	future23 = std::move(future22);
	// ASSERT_STATUS(out, INVALID, future22);
	// ASSERT_STATUS(out, INVALID, future23);
	out << F("#8.7 check status (partial NOT_READY, INVALID) -> (INVALID, partial NOT_READY)") << endl;
	ASSERT(out, future24.set_future_value_(uint8_t(0xBB)));
	future25 = std::move(future24);
	// ASSERT_STATUS(out, INVALID, future24);
	ASSERT_STATUS(out, NOT_READY, future25);
	out << F("#8.8 after complete set value, status shall be READY") << endl;
	ASSERT(out, future25.set_future_value_(uint8_t(0xCC)));
	ASSERT_STATUS(out, READY, future25);
	ASSERT_ERROR(out, 0, future25);
	ASSERT_VALUE(out, 0xCCBBu, future25);
	out << endl;

	// Check Future subclassing
	out << F("TEST #9 Future subclassing...") << endl;
	out << F("#9.1 instantiate future") << endl;
	MyFuture my_future;
	ASSERT_STATUS(out, NOT_READY, my_future);
	out << F("#9.3 set_future_value()") << endl;
	ASSERT(out, my_future.set_future_value_(123));
	ASSERT_STATUS(out, READY, my_future);
	out << F("#9.4 get()") << endl;
	uint16_t actual = 0;
	ASSERT(out, my_future.get(actual));
	assert(out, F("myfuture.get() value"), 1230u, actual);
	ASSERT_STATUS(out, READY, my_future);
	out << endl;

	// Check value storage in Future
	out << F("TEST #10 Future value storage...") << endl;
	out << F("#10.1 instantiate future") << endl;
	Future<uint16_t, uint16_t> future26{12345};
	ASSERT_STATUS(out, NOT_READY, future26);
	out << F("#10.3 get storage value") << endl;
	uint16_t input = 0;
	ASSERT(out, future26.get_storage_value_((uint8_t*) &input, sizeof(input)));
	assert(out, F("get_storage_value((future26.id())"), 12345u, input);
	ASSERT_STATUS(out, NOT_READY, future26);
	out << F("#10.4 set_future_value()") << endl;
	ASSERT(out, future26.set_future_value_(123));
	ASSERT_STATUS(out, READY, future26);
	out << F("#10.5 get()") << endl;
	actual = 0;
	ASSERT(out, future26.get(actual));
	assert(out, F("future26.get() value"), 123u, actual);
	ASSERT_STATUS(out, READY, future26);
	out << endl;

	// Check Future without value (just done or error or not)
	out << F("TEST #11 Future without value...") << endl;
	out << F("#11.1 instantiate future") << endl;
	Future<> future27;
	ASSERT_STATUS(out, NOT_READY, future27);
	out << F("#11.3 set finish()") << endl;
	ASSERT(out, future27.set_future_finish_());
	ASSERT_STATUS(out, READY, future27);
	ASSERT(out, future27.get());
	out << endl;
}
