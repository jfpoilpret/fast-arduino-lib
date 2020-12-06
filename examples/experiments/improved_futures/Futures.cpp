/*
 * This program is a Proof of Concept for enhancement of Futures in order to:
 * - support Future dependencies (callbacks)
 * - support Futures grouping
 * 
 * The constraints:
 * - limit extra code
 * - limit extra time
 * - support both Future and FakeFuture
 * - compatibility with current usage of futures
 * 
 * It just uses an Arduino UNO with USB console.
 */

// Possible ways for callbacks:
// - one or more listener ABC with virtual method(s) for various future changes
//		- provided at construction time (additional args with default nullptr)
//	+ easy to implement
//	- virtual overhead of code size and speed (particularly from inside ISR)
// - one functor on all Future and FakeFuture templates
//		- default functor type (empty)
//		- default arg value
//	+ impact size/speed only when not empty and to the strict minimum
//	- much harder to implement properly

#include <fastarduino/future.h>
#include <fastarduino/initializer_list.h>

#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/flash.h>
#include <fastarduino/array.h>

#include <fastarduino/tests/assertions.h>

// Register vector for UART (used for debug)
REGISTER_UATX_ISR(0)

// Example starts here
//=====================

using namespace future;
using namespace streams;
using namespace containers;

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

//TODO - example: self listening Future to change its INPUT based on OUTPUT

// template<typename F, uint8_t SIZE> class FuturesGroup : public FutureListener<F>
// {
// 	//TODO ensure F is a proper future (need specific traits...)
// public:
// 	//TODO how to deal with move constructor?
// 	FutureStatus status() const
// 	{
// 		return status_;
// 	}

// 	FutureStatus await() const
// 	{
// 		while (true)
// 		{
// 			FutureStatus temp_status = status();
// 			if (temp_status != FutureStatus::NOT_READY)
// 				return temp_status;
// 			time::yield();
// 		}
// 	}

// 	int error() const
// 	{
// 		return error_;
// 	}

// protected:
// 	FuturesGroup(std::initializer_list<F&> futures)
// 	{
// 		static_assert(
// 			futures.size() == SIZE, "FuturesGroup constructor must be initialized with exactly SIZE parameters");
// 		F** temp = futures_;
// 		for (F& future: futures)
// 			*temp++ = future;
// 	}

// 	void on_status_change(const F& future) override
// 	{
// 		const FutureStatus status = future.status();
// 		switch (status)
// 		{
// 			case FutureStatus::ERROR:
// 			error_ = future.error();
// 			// Intentional Fallthrough

// 			case FutureStatus::INVALID:
// 			status_ = status;
// 			break;

// 			case FutureStatus::READY:
// 			// check if all futures ready
// 			if (++count_ready_ == SIZE)
// 				status_ = FutureStatus::READY;
// 			break;

// 			case FutureStatus::NOT_READY:
// 			// do nothing
// 			break;
// 		}
// 	}

// private:
// 	F* futures_[SIZE];
// 	uint8_t count_ready_ = 0;
// 	volatile FutureStatus status_ = FutureStatus::NOT_READY;
// 	int error_ = 0;
// };

struct FutureListener : FutureStatusListener<AbstractFuture>, FutureOutputListener<AbstractFuture>
{
	explicit FutureListener(ostream& out) : out_{out} {}
	
	void on_status_change(UNUSED const AbstractFuture& future, FutureStatus new_status) override
	{
		out_ << F("on_status_change() status = ") << new_status << endl;
	}
	void on_output_change(UNUSED const AbstractFuture& future, uint8_t* output_data, uint8_t* output_current) override
	{
		out_	<< F("on_output_change() data = ") << hex << output_data << F(", current = ") 
				<< hex << output_current << endl;
	}

	ostream& out_;
};

class MyFuture : public Future<uint32_t, uint8_t>
{
	using PARENT = Future<uint32_t, uint8_t>;
	static constexpr uint8_t REG_INDEX = 0x34;

public:
	MyFuture(FutureListener& listener) : PARENT{REG_INDEX, &listener, &listener} {}
	MyFuture(MyFuture&&) = default;
	MyFuture& operator=(MyFuture&&) = default;
};

struct UpdateRegister
{
	UpdateRegister(uint8_t reg_index)
	{
		data[0] = data[1] = reg_index;
	}

	uint8_t data[3] = {0 ,0, 0};
};
class UpdateRegisterFuture : public Future<uint8_t, UpdateRegister>, public FutureOutputListener<AbstractFuture>
{
	using PARENT = Future<uint8_t, UpdateRegister>;

public:
	UpdateRegisterFuture(uint8_t reg_index, uint8_t set_mask)
		:	PARENT{UpdateRegister{reg_index}, nullptr, this}, set_mask_{set_mask} {}
	UpdateRegisterFuture(UpdateRegisterFuture&&) = default;
	UpdateRegisterFuture& operator=(UpdateRegisterFuture&&) = default;

	void on_output_change(
		UNUSED const AbstractFuture& future, UNUSED uint8_t* output_data, uint8_t* output_current) override
	{
		// Only one call expected, directly get output value and change it
		uint8_t value = *(output_current - 1);
		value |= set_mask_;
		//TODO const_cast not good here, maybe overload Future get_input() to provide const and non-const flavours
		const_cast<UpdateRegister&>(get_input()).data[2] = value;
	}

private:
	uint8_t set_mask_;
};

int main() __attribute__((OS_main));
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

	FutureListener listener{out};

	// Start feeding future and check output
	MyFuture f1{listener};
	out << F("set_future_value(0x11)") << endl;
	f1.set_future_value_(uint8_t(0x11));

	out << F("set_future_value(0x22)") << endl;
	f1.set_future_value_(uint8_t(0x22));

	out << F("set_future_value(0x33)") << endl;
	f1.set_future_value_(uint8_t(0x33));

	out << F("set_future_value(0x44)") << endl;
	f1.set_future_value_(uint8_t(0x44));

	out << F("f1.status() = ") << f1.status() << endl;
	uint32_t result = 0;
	out << F("f1.get(result) = ") << f1.get(result) << endl;
	out << F("result = ") << hex << result << endl;

	// Start feeding future, force error and check output
	MyFuture f2{listener};
	f2.set_future_value_(uint8_t(0x55));
	f2.set_future_finish_();
	f2.set_future_error_(-10);
	out << F("f2.status() = ") << f2.status() << endl;
	out << F("f2.error() = ") << dec << f2.error() << endl;

	UpdateRegisterFuture f3{0xF7, 0x12};
	uint8_t data = 0;
	out << F("f3.get_storage_value_(data) = ") << f3.get_storage_value_(data) << endl;
	out << F("data = ") << hex << data << endl;
	out << F("f3.set_future_value_(0x40) = ") << f3.set_future_value_(uint8_t(0x40)) << endl;
	out << F("f3.get_storage_value_(data) = ") << f3.get_storage_value_(data) << endl;
	out << F("data = ") << hex << data << endl;
	out << F("f3.get_storage_value_(data) = ") << f3.get_storage_value_(data) << endl;
	out << F("data = ") << hex << data << endl;
	out << F("f3.status() = ") << f3.status() << endl;
}
