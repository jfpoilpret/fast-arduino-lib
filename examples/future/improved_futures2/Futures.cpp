/*
 * This program demonstrates the use Futures listeners and the new FuturesGroup API.
 * 
 * It just uses an Arduino UNO with USB console.
 */

#include <fastarduino/future.h>

#include <fastarduino/uart.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/flash.h>

// Register vector for UART (used for debug)
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<board::USART::USART0>)

// Comment to use FakeFuture instead of real Future
#define REAL_FUTURE

// Example starts here
//=====================

using namespace future;
using namespace streams;
using namespace containers;

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

#ifdef REAL_FUTURE
#define ABSTRACTFUTURE AbstractFuture
#define FUTURE Future
#else
#define ABSTRACTFUTURE AbstractFakeFuture
#define FUTURE FakeFuture
#endif

struct FutureListener
{
	explicit FutureListener(ostream& out) : out_{out}
	{
		interrupt::register_handler(*this);
	}
	
	void on_output_change(const ABSTRACTFUTURE& f)
	{
		uint8_t size;
		synchronized size = f.get_future_value_size_();
		out_ << F("on_output_change() status = ") << size << endl;
	}

	ostream& out_;
};

class MyFuture : public FUTURE<uint32_t, uint8_t>
{
	using PARENT = FUTURE<uint32_t, uint8_t>;
	static constexpr uint8_t REG_INDEX = 0x34;

public:
	MyFuture() : PARENT{REG_INDEX, future::FutureNotification::OUTPUT} {}
	MyFuture(MyFuture&&) = default;
	MyFuture& operator=(MyFuture&&) = default;
};

REGISTER_FUTURE_STATUS_NO_LISTENERS()
REGISTER_FUTURE_OUTPUT_LISTENERS(ABSTRACTFUTURE, FutureListener)

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
	MyFuture f1;
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
	MyFuture f2;
	f2.set_future_value_(uint8_t(0x55));
	f2.set_future_finish_();
	f2.set_future_error_(-10);
	out << F("f2.status() = ") << f2.status() << endl;
	out << F("f2.error() = ") << dec << f2.error() << endl;
}
