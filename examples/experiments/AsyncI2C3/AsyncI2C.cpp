/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * This one is a proof of concept on I2C asynchronous handling, to be later integrated
 * to FastArduino library.
 * For tests, I just use a DS1307 connected through I2C (SDA/SCL) to an Arduino UNO.
 */

//TODO improvements
// - Add sync RTC based on Async RTC

#include <util/delay_basic.h>

#include <fastarduino/i2c.h>
#include <fastarduino/future.h>
#include <fastarduino/queue.h>
#include <fastarduino/time.h>
#include <fastarduino/interrupts.h>
#include <fastarduino/bits.h>
#include <fastarduino/utilities.h>

#include <fastarduino/uart.h>
#include <fastarduino/iomanip.h>

// #define DEBUG_STEPS
// #define DEBUG_REGISTER_OK
// #define DEBUG_REGISTER_ERR
// #define DEBUG_SEND_OK
// #define DEBUG_SEND_ERR
// #define DEBUG_RECV_OK
// #define DEBUG_RECV_ERR
// #define DEBUG_DATA_RECV
// #define DEBUG_DATA_SEND
#if defined(DEBUG_STEPS) || defined(DEBUG_REGISTER_OK) || defined(DEBUG_REGISTER_ERR) || defined(DEBUG_SEND_OK) || defined(DEBUG_SEND_ERR) || defined(DEBUG_RECV_OK) || defined(DEBUG_RECV_ERR)
#define DEBUG_STATUS
#endif

#include "array.h"
#include "i2c_handler.h"
#include "ds1307.h"

// This is used when nothing works at all and this reduces the tests to only once get_ram() call
// #define BASIC_DEBUG

// Register vector for UART (used for debug)
REGISTER_UATX_ISR(0)

// Actual test example
//=====================
using I2CHANDLER = i2c::I2CHandler<i2c::I2CMode::STANDARD>;

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

void display_time(streams::ostream& out, const tm& time)
{
	out	<< streams::dec << F("RTC: [") 
		<< uint8_t(time.tm_wday) << ']'
		<< time.tm_mday << '.'
		<< time.tm_mon << '.'
		<< time.tm_year << ' '
		<< time.tm_hour << ':'
		<< time.tm_min << ':'
		<< time.tm_sec
		<< streams::endl;
}

void trace(streams::ostream& out, bool reset = true)
{
#ifdef DEBUG_STATUS
	trace_states(out, reset);
#endif
#ifdef DEBUG_DATA_SEND
	trace_send_data(out, reset);
#endif
#ifdef DEBUG_DATA_RECV
	trace_recv_data(out, reset);
#endif
}

// REGISTER_I2C_ISR_METHOD(i2c::I2CMode::STANDARD, RTCCallback, &RTCCallback::callback)
REGISTER_I2C_ISR(i2c::I2CMode::STANDARD)

static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static i2c::I2CCommand i2c_buffer[I2C_BUFFER_SIZE];

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

static constexpr uint8_t MAX_FUTURES = 128;

using namespace streams;

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

	// Initialize FutureManager
	future::FutureManager<MAX_FUTURES> future_manager;

	// Initialize I2C async handler
	I2CHANDLER handler{i2c_buffer};
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
	}

	time::delay_ms(1000);

	{
		out << F("\nTEST #1.1 write RAM bytes (val: i+2), one by one") << endl;
		RTC::SET_RAM1 set[RAM_SIZE];
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			set[i] = RTC::SET_RAM1{i, i + 2};
			int error1 = rtc.set_ram(set[i]);
			if (error1)
				out << F("S") << dec << i << F(" ") << flush;
			// This delay is needed to give time to I2C transactions to finish 
			// and free I2C commands in buffer (only 32) 
			time::delay_us(100);
		}
		out << endl;
		for (uint8_t i = 0 ; i < RAM_SIZE; ++i)
		{
			out << F("set[") << dec << i << F("] await()=") << set[i].await() << endl;
			out << F("error()=") << dec << set[i].error() << endl;
		}
		trace(out);
	}

	time::delay_ms(1000);

	{
		out << F("\nTEST #1.2 read RAM bytes, one by one (val should be i+2)") << endl;
		RTC::GET_RAM1 get[RAM_SIZE];
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			get[i] = RTC::GET_RAM1{i};
			int error2 = rtc.get_ram(get[i]);
			if (error2)
				out << F("G") << dec << i << F(" ") << flush;
			// This delay is needed to give time to I2C transactions to finish 
			// and free I2C commands in buffer (only 32) 
			time::delay_us(1000);
		}
		out << endl;
		for (uint8_t i = 0 ; i < RAM_SIZE; ++i)
		{
			out << F("get[") << dec << i << F("] await()=") << get[i].await() << endl;
			out << F("error()=") << dec << get[i].error() << endl;
			uint8_t result = 0;
			get[i].get(result);
			out << F("get()=") << hex << result << endl;
		}
		trace(out);
	}

	time::delay_ms(1000);

	out << F("sizeof(RTC::GET_RAM1)=") << dec << sizeof(RTC::GET_RAM1) << endl;
	out << F("sizeof(RTC::SET_RAM<1>)=") << dec << sizeof(RTC::SET_RAM<1>) << endl;

	{
		out << F("\nTEST #1.3 read all RAM bytes in one transaction (val should i+2)") << endl;
		RTC::GET_RAM<RAM_SIZE> get{0};
		int error = rtc.get_ram(get);
		if (error)
			out << F("G") << endl;
		out << F("get await()=") << get.await() << endl;
		out << F("error()=") << dec << get.error() << endl;
		RTC::GET_RAM<RAM_SIZE>::OUT result{};
		get.get(result);
		for (uint8_t i = 0 ; i < RAM_SIZE; ++i)
			out << F("get(") << dec << i << F(")=") << hex << result[i] << endl;
		trace(out);
	}

	time::delay_ms(1000);

	{
		out << F("\nTEST #1.4 write all RAM bytes (val: 3i+10) in one transaction") << endl;
		uint8_t values[RAM_SIZE];
		for (uint8_t i = 0 ; i < RAM_SIZE; ++i)
			values[i] = i * 3 + 10;
		RTC::SET_RAM<RAM_SIZE> set{0, values};
		int error = rtc.set_ram(set);
		if (error)
			out << F("S") << endl;
		time::delay_ms(1000);
		trace(out);
		out << F("set await()=") << set.await() << endl;
		out << F("error()=") << dec << set.error() << endl;
		trace(out);
	}

	time::delay_ms(1000);

	{
		out << F("\nTEST #2 set datetime (Wed 06.05.2020 20:00:00)") << endl;
		tm datetime;
		datetime.tm_year = 20;
		datetime.tm_mon = 5;
		datetime.tm_mday = 6;
		datetime.tm_wday = WeekDay::WEDNESDAY;
		datetime.tm_hour = 20;
		datetime.tm_min = 0;
		datetime.tm_sec = 0;
		RTC::SET_DATETIME set{datetime};
		int error = rtc.set_datetime(set);
		if (error)
			out << F("S") << endl;
		out << F("set await()=") << set.await() << endl;
		out << F("error()=") << dec << set.error() << endl;
		trace(out);
	}

	time::delay_ms(13000);

	{
		out << F("\nTEST #3 get datetime (should be: Wed 06.05.2020 20:00:13") << endl;
		RTC::GET_DATETIME get;
		int error = rtc.get_datetime(get);
		if (error)
			out << F("G") << endl;
		out << F("get await()=") << get.await() << endl;
		out << F("error()=") << dec << get.error() << endl;
		tm datetime;
		out << F("get()=") << dec << get.get(datetime) << endl;
		trace(out);
		display_time(out, datetime);
	}

	time::delay_ms(1000);

	{
		out << F("\nTEST #4.1 halt clock") << endl;
		RTC::HALT_CLOCK halt;
		int error = rtc.halt_clock(halt);
		if (error)
			out << F("H") << endl;
		out << F("halt await()=") << halt.await() << endl;
		out << F("error()=") << dec << halt.error() << endl;
		trace(out);
		time::delay_ms(10000);

		out << F("\nTEST #4.2 get datetime (should be: Wed 06.05.2020 20:00:14)") << endl;
		RTC::GET_DATETIME get;
		error = rtc.get_datetime(get);
		if (error)
			out << F("G") << endl;
		out << F("get await()=") << get.await() << endl;
		out << F("error()=") << dec << get.error() << endl;
		tm datetime;
		out << F("get()=") << dec << get.get(datetime) << endl;
		trace(out);
		display_time(out, datetime);
	}
#endif

	handler.end();
}
