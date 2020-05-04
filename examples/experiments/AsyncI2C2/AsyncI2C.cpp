/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * This one is a proof of concept on I2C asynchronous handling, to be later integrated
 * to FastArduino library.
 * For tests, I just use a DS1307 connected through I2C (SDA/SCL) to an Arduino UNO.
 */

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
#define DEBUG_REGISTER_ERR
// #define DEBUG_SEND_OK
#define DEBUG_SEND_ERR
// #define DEBUG_RECV_OK
#define DEBUG_RECV_ERR

#include "array.h"
#include "i2c_handler.h"

// Register vector for UART (used for debug)
REGISTER_UATX_ISR(0)

//TODO add more API to RTC (same as official DS1307) and check further

// RTC-specific definitions (for testing purpose)
//================================================
class RTC
{
	public:
	RTC(I2CHandler<i2c::I2CMode::STANDARD>& handler) : handler_{handler} {}

	static constexpr uint8_t ram_size()
	{
		return RAM_SIZE;
	}

	using SET_RAM = future::Future<void, array<uint8_t, 2>>;
	int set_ram(uint8_t address, uint8_t data, SET_RAM& future)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return errors::EINVAL;
		auto& manager = future::AbstractFutureManager::instance();
		synchronized
		{
			// pre-conditions (must be synchronized)
			if (manager.available_futures_() == 0) return errors::EAGAIN;
			if (!handler_.ensure_num_commands_(1)) return errors::EAGAIN;
			// prepare future and I2C transaction
			SET_RAM temp{{address, data}};
			// NOTE: normally 3 following calls should never return false!
			if (!manager.register_future_(temp)) return errors::EAGAIN;
			if (!handler_.write_(DEVICE_ADDRESS, temp.id(), true, true)) return errors::EAGAIN;
			future = std::move(temp);
			return 0;
		}
	}

	using GET_RAM1 = future::Future<uint8_t, uint8_t>;
	int get_ram(uint8_t address, GET_RAM1& future)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return errors::EINVAL;
		auto& manager = future::AbstractFutureManager::instance();
		//TODO maybe an abstract device class could encapsulate all that block with a list of commands in args?
		synchronized
		{
			// pre-conditions (must be synchronized)
			if (manager.available_futures_() == 0) return errors::EAGAIN;
			if (!handler_.ensure_num_commands_(2)) return errors::EAGAIN;
			// prepare future and I2C transaction
			GET_RAM1 temp{address};
			// NOTE: normally 3 following calls should never return false!
			if (!manager.register_future_(temp)) return errors::EAGAIN;
			if (!handler_.write_(DEVICE_ADDRESS, temp.id(), false, false)) return errors::EAGAIN;
			if (!handler_.read_(DEVICE_ADDRESS, temp.id(), true, false)) return errors::EAGAIN;
			future = std::move(temp);
			return 0;
		}
	}

	template<uint8_t SIZE, typename T = uint8_t>
	using GET_RAM = future::Future<uint8_t, array<T, SIZE>>;
	template<uint8_t SIZE>
	int get_ram(uint8_t address, GET_RAM<SIZE>& future)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return errors::EINVAL;
		auto& manager = future::AbstractFutureManager::instance();
		//TODO maybe an abstract device class could encapsulate all that?
		synchronized
		{
			// pre-conditions (must be synchronized)
			if (manager.available_futures_() == 0) return errors::EAGAIN;
			if (!handler_.ensure_num_commands_(2)) return errors::EAGAIN;
			// prepare future and I2C transaction
			GET_RAM<SIZE> temp{address};
			// NOTE: normally 3 following calls should never return false!
			if (!manager.register_future_(temp)) return errors::EAGAIN;
			if (!handler_.write_(DEVICE_ADDRESS, temp.id(), false, false)) return errors::EAGAIN;
			if (!handler_.read_(DEVICE_ADDRESS, temp.id(), true, false)) return errors::EAGAIN;
			future = std::move(temp);
			return 0;
		}
	}

	private:
	static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
	static constexpr const uint8_t RAM_START = 0x08;
	static constexpr const uint8_t RAM_END = 0x40;
	static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;

	I2CHandler<i2c::I2CMode::STANDARD>& handler_;
};

// Actual test example
//=====================
using I2CHANDLER = I2CHandler<i2c::I2CMode::STANDARD>;

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

// REGISTER_I2C_ISR_METHOD(i2c::I2CMode::STANDARD, RTCCallback, &RTCCallback::callback)
REGISTER_I2C_ISR(i2c::I2CMode::STANDARD)

static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static I2CCommand i2c_buffer[I2C_BUFFER_SIZE];

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

	// INITIAL debug test with only one call, normally not part of complete unit tests
	// {
	// 	RTC::GET_RAM data;
	// 	out << F("TEST #0 read one RAM byte") << endl;
	// 	bool ok = rtc.get_ram(0, data);
	// 	uint8_t id = data.id();
	// 	future::FutureStatus status = data.status();
	// 	out << F("get_ram()=") << ok << endl;
	// 	out << F("id=") << dec << id << F(" status=") << status << endl;
	// 	// out << F("id=") << dec << data.id() << F(" status=") << data.status() << endl;
	// 	// time::delay_ms(1000);
	// 	out << F("data await()=") << data.await() << endl;
	// 	out << F("error()=") << dec << data.error() << endl;
	// 	uint8_t result = 0;
	// 	data.get(result);
	// 	out << F("get()=") << hex << result << endl;
	// 	trace_states(out);
	// }

	{
		RTC::GET_RAM1 data[RAM_SIZE];
		out << F("TEST #0 read all RAM bytes, one by one") << endl;
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			int error = rtc.get_ram(i, data[i]);
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
		trace_states(out);
	}

	time::delay_ms(1000);

	{
		RTC::SET_RAM set[RAM_SIZE];

		out << F("TEST #1.1 write RAM bytes, one by one") << endl;
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			int error1 = rtc.set_ram(i, i + 2, set[i]);
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
		trace_states(out);
	}

	time::delay_ms(1000);

	{
		RTC::GET_RAM1 get[RAM_SIZE];

		out << F("TEST #1.2 read RAM bytes, one by one") << endl;
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			int error2 = rtc.get_ram(i, get[i]);
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
		trace_states(out);
	}

	time::delay_ms(1000);

	out << F("sizeof(RTC::GET_RAM1)=") << dec << sizeof(RTC::GET_RAM1) << endl;
	out << F("sizeof(RTC::SET_RAM)=") << dec << sizeof(RTC::SET_RAM) << endl;

	//FIXME the following crashes the MCU immediately, probably because of stack trace
	// {
	// 	RTC::SET_RAM set[RAM_SIZE];
	// 	RTC::GET_RAM get[RAM_SIZE];

	// 	out << F("TEST #1.3 write and read RAM bytes, one by one") << endl;
	// 	for (uint8_t i = 0; i < RAM_SIZE; ++i)
	// 	{
	// 		int error1 = rtc.set_ram(i, i + 1, set[i]);
	// 		if (error1)
	// 			out << F("S") << dec << i << F(" ") << flush;
	// 		int error2 = rtc.get_ram(i, get[i]);
	// 		if (error2)
	// 			out << F("G") << dec << i << F(" ") << flush;
	// 		// This delay is needed to give time to I2C transactions to finish 
	// 		// and free I2C commands in buffer (only 32) 
	// 		time::delay_us(1000);
	// 	}
	// 	out << endl;
	// 	for (uint8_t i = 0 ; i < RAM_SIZE; ++i)
	// 	{
	// 		out << F("set[") << dec << i << F("] await()=") << set[i].await() << endl;
	// 		out << F("error()=") << dec << set[i].error() << endl;
	// 		out << F("get[") << dec << i << F("] await()=") << get[i].await() << endl;
	// 		out << F("error()=") << dec << get[i].error() << endl;
	// 		uint8_t result = 0;
	// 		get[i].get(result);
	// 		out << F("get()=") << hex << result << endl;
	// 	}
	// 	trace_states(out);
	// }

	// The following test works properly
	// out << F("TEST #2 write all RAM bytes, one by one, then read all, one by one") << endl;
	// for (uint8_t i = 0; i < RAM_SIZE; ++i)
	// {
	// 	bool ok = rtc.set_ram(i, i * 2 + 1);
	// 	out << F("set_ram(") << dec << i << F(") => ") << ok << endl;
	// }
	// for (uint8_t i = 0; i < RAM_SIZE; ++i)
	// {
	// 	uint8_t data = 0;
	// 	bool ok = rtc.get_ram(i, data);
	// 	out << F("get_ram(") << dec << i << F(") => ") << ok << endl;
	// 	out << F("get_ram() data = ") << dec << data << endl;
	// }
	// time::delay_ms(1000);

	// out << F("TEST #3 write and read RAM bytes, one by one, without delay") << endl;
	// for (uint8_t i = 0; i < RAM_SIZE; ++i)
	// 	data1[i] = 0;
	// for (uint8_t i = 0; i < RAM_SIZE; ++i)
	// {
	// 	bool ok1 = rtc.set_ram(i, i + 1);
	// 	bool ok2 = rtc.get_ram(i, data1[i]);
	// 	if (!ok1)
	// 		out << F("KO1 on ") << i << endl;
	// 	if (!ok2)
	// 		out << F("KO2 on ") << i << endl;
	// }
	// time::delay_ms(1000);
	// out << F("all data after 1s = [") << data1[0] << flush;
	// for (uint8_t i = 1; i < RAM_SIZE; ++i)
	// 	out << F(", ") << data1[i] << flush;
	// out << ']' << endl;

	handler.end();
}
