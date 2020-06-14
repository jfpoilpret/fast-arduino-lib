/*
 * This program is used for progressively migrating async I2C stuff to FastArduino.
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
#include <fastarduino/new_i2c_handler.h>
#include <fastarduino/devices/new_ds1307.h>

#ifdef ARDUINO_UNO
#define HARD_UART
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static constexpr uint8_t MAX_FUTURES = 128;
static i2c::I2CCommand i2c_buffer[I2C_BUFFER_SIZE];
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t MAX_FUTURES = 8;
#else
#error "Current target is not yet supported!"
#endif

// Uncomment when device does nto work properly and we want to trace in "real time"
// every I2C step
// #define TRACE_PROTOCOL

// This is used when nothing works at all and this reduces the tests to only one get_ram() call
// #define BASIC_DEBUG

#if I2C_TRUE_ASYNC
REGISTER_I2C_ISR(i2c::I2CMode::STANDARD)
#endif

static char output_buffer[OUTPUT_BUFFER_SIZE];

using I2CHANDLER = i2c::I2CManager<i2c::I2CMode::STANDARD>;
using namespace streams;
using namespace devices::rtc;

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

ostream* pout = nullptr;
#define COUT (*pout)

#ifdef TRACE_PROTOCOL
static void i2c_hook(i2c::DebugStatus status, uint8_t data)
{
	switch (status)
	{
		case i2c::DebugStatus::START:
		COUT << F("St ") << flush;
		break;

		case i2c::DebugStatus::REPEAT_START:
		COUT << F("RS ") << flush;
		break;

		case i2c::DebugStatus::STOP:
		COUT << F("Sp ") << flush;
		break;

		case i2c::DebugStatus::SLAW:
		COUT << F("AW ") << hex << data << ' ' << flush;
		break;

		case i2c::DebugStatus::SLAR:
		COUT << F("AR ") << hex << data << ' ' << flush;
		break;

		case i2c::DebugStatus::SEND:
		COUT << F("S ") << hex << data << ' ' << flush;
		break;

		case i2c::DebugStatus::SEND_OK:
		COUT << F("So ") << flush;
		break;

		case i2c::DebugStatus::SEND_ERROR:
		COUT << F("Se ") << flush;
		break;

		case i2c::DebugStatus::RECV:
		COUT << F("R ") << flush;
		break;

		case i2c::DebugStatus::RECV_LAST:
		COUT << F("RL ") << flush;
		break;

		case i2c::DebugStatus::RECV_OK:
		COUT << F("Ro ") << flush;
		break;

		case i2c::DebugStatus::RECV_ERROR:
		COUT << F("Re ") << flush;
		break;
	}
}
#else
i2c::I2C_DEBUG_HOOK i2c_hook = nullptr; 
#endif

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
	pout = &out;
	out << F("Starting...") << endl;

	// Initialize FutureManager
	future::FutureManager<MAX_FUTURES> future_manager;

	// Initialize I2C async handler
#if I2C_TRUE_ASYNC
	I2CHANDLER handler{i2c_buffer, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, i2c_hook};
#else
	I2CHANDLER handler{i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, i2c_hook};
#endif
	DS1307 rtc{handler};
	out << F("Before handler.begin()") << endl;
	out << boolalpha << showbase;

	handler.begin();

	constexpr uint8_t RAM_SIZE = rtc.ram_size();
	constexpr uint8_t MAX_READ = (RAM_SIZE < MAX_FUTURES ? RAM_SIZE : MAX_FUTURES);

#ifdef BASIC_DEBUG
	// INITIAL debug test with only one call, normally not part of complete unit tests
	{
		out << F("\nTEST #0 read one RAM byte") << endl;
		RTC::GET_RAM1 data{0};
		int ok = rtc.get_ram(data);
		out << F("get_ram()=") << dec << ok << endl;
		out << F("handler.status()=") << hex << handler.status() << endl;
		uint8_t id = data.id();
		future::FutureStatus status = data.status();
		out << F("id=") << dec << id << F(" status=") << status << endl;
		// out << F("id=") << dec << data.id() << F(" status=") << data.status() << endl;
		// time::delay_ms(1000);
		out << F("data await()=") << data.await() << endl;
		out << F("error()=") << dec << data.error() << endl;
		uint8_t result = 0;
		data.get(result);
		out << F("get()=") << hex << result << endl;
	}
#else
	{
		out << F("\nTEST #0 read all RAM bytes, one by one") << endl;
		DS1307::GET_RAM1 data[MAX_READ];
		for (uint8_t i = 0; i < MAX_READ; ++i)
		{
			data[i] = DS1307::GET_RAM1{i};
			int error = rtc.get_ram(data[i]);
			if (error)
				out << F("F") << dec << i << F(" ") << flush;
			// This delay is needed to give time to I2C transactions to finish 
			// and free I2C commands in buffer (only 32) 
			time::delay_us(200);
		}
		out << endl;
		for (uint8_t i = 0 ; i < MAX_READ; ++i)
		{
			out << F("data[") << dec << i << F("] await()=") << data[i].await() << endl;
			out << F("error()=") << dec << data[i].error() << endl;
			uint8_t result = 0;
			data[i].get(result);
			out << F("get()=") << hex << result << endl;
		}
	}

	{
		out << F("\nTEST #1 read all RAM bytes, all at once") << endl;
		DS1307::GET_RAM<RAM_SIZE> data{0};
		int error = rtc.get_ram(data);
		if (error)
			out << F("F") << flush;
		out << endl;
		out << F("data await()=") << data.await() << endl;
		out << F("error()=") << dec << data.error() << endl;
		DS1307::GET_RAM<RAM_SIZE>::OUT result;
		data.get(result);
		out << F("result") << endl;
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			out << dec << i << '=' << hex << result[i] << endl;
		}
	}

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
		DS1307::SET_DATETIME set{datetime};
		int error = rtc.set_datetime(set);
		if (error)
			out << F("S") << endl;
		out << F("set await()=") << set.await() << endl;
		out << F("error()=") << dec << set.error() << endl;
	}

	time::delay_ms(13000);

	{
		out << F("\nTEST #3 get datetime (should be: Wed 06.05.2020 20:00:13") << endl;
		DS1307::GET_DATETIME get;
		int error = rtc.get_datetime(get);
		if (error)
			out << F("G") << endl;
		out << F("get await()=") << get.await() << endl;
		out << F("error()=") << dec << get.error() << endl;
		tm datetime;
		out << F("get()=") << dec << get.get(datetime) << endl;
		display_time(out, datetime);
	}
#endif

	handler.end();
}
