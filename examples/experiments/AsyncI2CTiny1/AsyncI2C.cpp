/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * This one is a proof of concept on I2C asynchronous handling specificaloly for
 * ATtiny architecture.
 * As a matter of fact, ATtiny USI feature is not very well suited for asynchronous 
 * I2C handling as I2C master (this is easier for slaves).
 * This PoC will try to demonstrate working with DS1307 RTC chip from an ATtiny84 MCU, 
 * using Timer0 as clock source for USI SCL clock.
 */

#include <util/delay_basic.h>

#include <fastarduino/boards/board.h>
#include <fastarduino/i2c.h>
#include <fastarduino/queue.h>
#include <fastarduino/time.h>
#include <fastarduino/interrupts.h>
#include <fastarduino/bits.h>
#include <fastarduino/utilities.h>
#include <fastarduino/iomanip.h>

#ifdef ARDUINO_UNO
#define HARD_UART
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static constexpr uint8_t MAX_FUTURES = 128;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#include <fastarduino/soft_uart.h>
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 16;
static constexpr uint8_t MAX_FUTURES = 32;
#else
#error "Current target is not yet supported!"
#endif

#include "i2c_handler.h"
#include "ds1307.h"

// Setup debugging stuff
// #define DEBUG_STEPS
#define DEBUG_SEND_OK
#define DEBUG_SEND_ERR
#define DEBUG_RECV_OK
#define DEBUG_RECV_ERR
#include "debug.h"

#if defined(DEBUG_STEPS) || defined(DEBUG_SEND_OK) || defined(DEBUG_SEND_ERR) || defined(DEBUG_RECV_OK) || defined(DEBUG_RECV_ERR)
#define DEBUG_STATUS
#endif

// This is used when nothing works at all and this reduces the tests to only once get_ram() call
// #define BASIC_DEBUG

// Actual test example
//=====================

// Callback handler
class RTCCallback
{
public:
	RTCCallback()
	{
		interrupt::register_handler(*this);
	}

	void reset()
	{
		synchronized
		{
			count_commands_ = count_transactions_ = count_errors_ = 0;
		}
	}

	uint16_t count_errors() const
	{
		synchronized return count_errors_;
	}

	uint16_t count_commands() const
	{
		synchronized return count_commands_;
	}

	uint16_t count_transactions() const
	{
		synchronized return count_transactions_;
	}

private:
	void callback(i2c::I2CCallback result)
	{
		switch (result)
		{
			case i2c::I2CCallback::ERROR:
			++count_errors_;
			break;

			case i2c::I2CCallback::END_TRANSACTION:
			++count_transactions_;
			// fallback (to increment comamnds also)

			case i2c::I2CCallback::END_COMMAND:
			++count_commands_;
			break;

			default:
			break;
		}
	}

	uint16_t count_errors_ = 0;
	uint16_t count_commands_ = 0;
	uint16_t count_transactions_ = 0;

	DECL_TWI_FRIENDS
};

// REGISTER_I2C_ISR(i2c::I2CMode::STANDARD)
REGISTER_I2C_ISR_METHOD(i2c::I2CMode::STANDARD, RTCCallback, &RTCCallback::callback)

// Add utility ostream manipulator for FutureStatus
static const flash::FlashStorage* convert(future::FutureStatus s)
{
	switch (s)
	{
		case future::FutureStatus::INVALID:
		return F("INVALID");

		case future::FutureStatus::NOT_READY:
		return F("NOT_READY");

		case future::FutureStatus::READY:
		return F("READY");

		case future::FutureStatus::ERROR:
		return F("ERROR");
	}
}

streams::ostream& operator<<(streams::ostream& out, future::FutureStatus s)
{
	return out << convert(s);
}

void trace(streams::ostream& out)
{
#ifdef DEBUG_STATUS
	trace_states(out);
#endif
}

static i2c::I2CCommand i2c_buffer[I2C_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

using I2CHANDLER = i2c::I2CHandler<i2c::I2CMode::STANDARD>;
using namespace streams;

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	// Initialize debugging output
#ifdef HARD_UART
	serial::hard::UATX<UART> uatx{output_buffer};
#else
	serial::soft::UATX<TX> uatx{output_buffer};
#endif

	// Start UART
	uatx.begin(115200);
	ostream out = uatx.out();

	// Initialize callback handler
	RTCCallback callback{};

	// Initialize FutureManager
	future::FutureManager<MAX_FUTURES> future_manager;

	// Initialize I2C async handler
	I2CHANDLER handler{i2c_buffer, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, debug_hook};
	RTC rtc{handler};
	out << F("Before handler.begin()") << endl;
	out << boolalpha << showbase;

	handler.begin();

	constexpr uint8_t RAM_SIZE = rtc.ram_size();

#ifdef BASIC_DEBUG
	// INITIAL debug test with only one call, normally not part of complete unit tests
	{
		out << F("\nTEST #0 read one RAM byte") << endl;
		RTC::GET_RAM1 data{0};
		int ok = rtc.get_ram(data);
		uint8_t id = data.id();
		future::FutureStatus status = data.status();
		out << F("get_ram()=") << ok << endl;
		out << F("id=") << dec << id << F(" status=") << status << endl;
		// out << F("id=") << dec << data.id() << F(" status=") << data.status() << endl;
		// time::delay_ms(1000);
		out << F("data await()=") << data.await() << endl;
		out << F("error()=") << dec << data.error() << endl;
		uint8_t result = 0;
		data.get(result);
		out << F("get()=") << hex << result << endl;
		trace(out);
		out << F("callback ok = ") << dec << callback.count_ok() 
			<< F(", errors = ") << dec << callback.count_errors() << endl;
		callback.reset();
	}
#else
	{
		out << F("\nTEST #0 read all RAM bytes, one by one") << endl;
		RTC::GET_RAM1 data[RAM_SIZE];
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			data[i] = RTC::GET_RAM1{i};
			int error = rtc.get_ram(data[i]);
			if (error)
				out << F("F") << dec << i << F(" ") << flush;
			// This delay is needed to give time to I2C transactions to finish 
			// and free I2C commands in buffer (only 32) 
			time::delay_us(200);
		}
		out << endl;
		for (uint8_t i = 0 ; i < RAM_SIZE; ++i)
		{
			out << F("data[") << dec << i << F("] await()=") << data[i].await() << endl;
			out << F("error()=") << dec << data[i].error() << endl;
			uint8_t result = 0;
			data[i].get(result);
			out << F("get()=") << hex << result << endl;
		}
		trace(out);
		out << F("callback trans = ") << dec << callback.count_transactions() 
			<< F(", commands = ") << dec << callback.count_commands()
			<< F(", errors = ") << dec << callback.count_errors() << endl;
		callback.reset();
	}
#endif

	handler.end();
}
