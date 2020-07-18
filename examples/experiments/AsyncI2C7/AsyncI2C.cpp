/*
 * This program is used test improvements to the new I2C async API.
 * The first imprvoement tested is the possibility to perform several
 * consecutive writes, each filling a certain number of bytes of a future.
 * This allows e.g. consecutive writes at different addresses, separated by 
 * a REPEAT START.
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
#include <fastarduino/array.h>
#include <fastarduino/new_i2c_handler.h>
#include <fastarduino/new_i2c_device.h>

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
#define TRACE_PROTOCOL

#if I2C_TRUE_ASYNC
REGISTER_I2C_ISR(i2c::I2CMode::STANDARD)
#endif

static char output_buffer[OUTPUT_BUFFER_SIZE];

using I2CHANDLER = i2c::I2CManager<i2c::I2CMode::STANDARD>;
using namespace streams;

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
		COUT << F("Ro ") << hex << data << ' ' << flush;
		break;

		case i2c::DebugStatus::RECV_ERROR:
		COUT << F("Re ") << hex << data << ' ' << flush;
		break;
	}
}
#else
i2c::I2C_DEBUG_HOOK i2c_hook = nullptr; 
#endif

class RTC : public i2c::I2CDevice<i2c::I2CMode::STANDARD>
{
	using PARENT = i2c::I2CDevice<i2c::I2CMode::STANDARD>;
	template<typename T> using PROXY = lifecycle::LightProxy<T>;

	static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
	static constexpr const uint8_t RAM_START = 0x08;
	static constexpr const uint8_t RAM_END = 0x40;
	static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;

public:
	RTC(PARENT::MANAGER& manager) : PARENT{manager, DEVICE_ADDRESS} {}

	class SetRamFuture : public future::Future<void, containers::array<uint8_t, 3+4>>
	{
		using PARENT = future::Future<void, containers::array<uint8_t, 3+4>>;
	public:
		SetRamFuture() : PARENT{{RAM_START + 10, 1, 2, RAM_START + 20, 3, 4, 5}} {}
		SetRamFuture(SetRamFuture&&) = default;
		SetRamFuture& operator=(SetRamFuture&&) = default;
	};
	int set_ram(PROXY<SetRamFuture> future)
	{
		return launch_commands(
			// future, {write(3), write(4, i2c::I2CFinish::FUTURE_FINISH)});
			future, {write(3, i2c::I2CFinish::FORCE_STOP), write(4, i2c::I2CFinish::FUTURE_FINISH)});
	}

	class GetRamFuture : public future::Future<containers::array<uint8_t, 2+3>, containers::array<uint8_t, 1+1>>
	{
		using PARENT = future::Future<containers::array<uint8_t, 2+3>, containers::array<uint8_t, 1+1>>;
	public:
		GetRamFuture() : PARENT{{RAM_START + 10, RAM_START + 20}} {}
		GetRamFuture(GetRamFuture&&) = default;
		GetRamFuture& operator=(GetRamFuture&&) = default;
	};
	int get_ram(PROXY<GetRamFuture> future)
	{
		return launch_commands(
			// future, {write(1), read(2), write(1), read(3)});
			future, {write(1), read(2, i2c::I2CFinish::FORCE_STOP), write(1), read(3)});
	}

	class GetAllRamFuture : public future::Future<containers::array<uint8_t, RAM_SIZE>, uint8_t>
	{
		using PARENT = future::Future<containers::array<uint8_t, RAM_SIZE>, uint8_t>;
	public:
		GetAllRamFuture() : PARENT{RAM_START} {}
		GetAllRamFuture(GetAllRamFuture&&) = default;
		GetAllRamFuture& operator=(GetAllRamFuture&&) = default;
	};
	int get_all_ram(PROXY<GetAllRamFuture> future)
	{
		return launch_commands(future, {write(1), read(0)});
	}

	class SetAllRamFuture : public future::Future<void, containers::array<uint8_t, 1 + RAM_SIZE>>
	{
		using PARENT = future::Future<void, containers::array<uint8_t, 1 + RAM_SIZE>>;
	public:
		SetAllRamFuture(uint8_t value)
		{
			SetAllRamFuture::IN input;
			input[0] = RAM_START;
			for (uint8_t i = 1; i < input.size(); ++i)
				input[i] = value;
			reset_input_(input);
		}
		SetAllRamFuture(SetAllRamFuture&&) = default;
		SetAllRamFuture& operator=(SetAllRamFuture&&) = default;
	};
	int set_all_ram(PROXY<SetAllRamFuture> future)
	{
		return launch_commands(future, {write(0, i2c::I2CFinish::FUTURE_FINISH)});
	}
};

using namespace lifecycle;

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

	// Initialize I2C async handler
#if I2C_TRUE_ASYNC
	I2CHANDLER handler{i2c_buffer, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS};
	//NOTE cannot use i2c_hook because it shall not be executed from inside interrupts!
	// I2CHANDLER handler{i2c_buffer, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, i2c_hook};
#else
	I2CHANDLER handler{i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, i2c_hook};
#endif
	out << F("Before handler.begin()") << endl;
	out << boolalpha << showbase;
	handler.begin();

	RTC rtc{handler};
	{
		out << F("\nTEST #0.1 clear all RAM bytes") << endl;
		RTC::SetAllRamFuture output{0};
		int ok = rtc.set_all_ram(LightProxy<RTC::SetAllRamFuture>{output});
		out << F("\nset_all_ram()=") << dec << ok << endl;
		out << F("handler.status()=") << hex << handler.status() << endl;
		future::FutureStatus status = output.status();
		out << F("status=") << status << endl;
		out << F("data await()=") << output.await() << endl;
		out << F("error()=") << dec << output.error() << endl;
	}

	{
		out << F("\nTEST #0.2 get all RAM bytes") << endl;
		RTC::GetAllRamFuture input;
		int ok = rtc.get_all_ram(LightProxy<RTC::GetAllRamFuture>{input});
		out << F("\nget_all_ram()=") << dec << ok << endl;
		out << F("handler.status()=") << hex << handler.status() << endl;
		future::FutureStatus status = input.status();
		out << F("status=") << status << endl;
		out << F("data await()=") << input.await() << endl;
		out << F("error()=") << dec << input.error() << endl;
		RTC::GetAllRamFuture::OUT result;
		if (input.get(result))
		{
			out << F("get() OK") << endl;
			for (uint16_t i = 0; i < result.size(); ++i)
				out << F("result[") << dec << i << F("] = ") << result[i] << endl;
		}
		else
		{
			out << F("get() KO") << endl;
		}
	}

	{
		out << F("\nTEST #1 set 2 then 3 RAM bytes") << endl;
		RTC::SetRamFuture output;
		int ok = rtc.set_ram(LightProxy<RTC::SetRamFuture>{output});
		out << F("\nset_ram()=") << dec << ok << endl;
		out << F("handler.status()=") << hex << handler.status() << endl;
		future::FutureStatus status = output.status();
		out << F("status=") << status << endl;
		out << F("data await()=") << output.await() << endl;
		out << F("error()=") << dec << output.error() << endl;
	}

	{
		out << F("\nTEST #2 get all RAM bytes") << endl;
		RTC::GetAllRamFuture input;
		int ok = rtc.get_all_ram(LightProxy<RTC::GetAllRamFuture>{input});
		out << F("\nget_all_ram()=") << dec << ok << endl;
		out << F("handler.status()=") << hex << handler.status() << endl;
		future::FutureStatus status = input.status();
		out << F("status=") << status << endl;
		out << F("data await()=") << input.await() << endl;
		out << F("error()=") << dec << input.error() << endl;
		RTC::GetAllRamFuture::OUT result;
		if (input.get(result))
		{
			out << F("get() OK") << endl;
			for (uint16_t i = 0; i < result.size(); ++i)
				out << F("result[") << dec << i << F("] = ") << result[i] << endl;
		}
		else
		{
			out << F("get() KO") << endl;
		}
	}

	{
		out << F("\nTEST #3 get 2 then 3 RAM bytes") << endl;
		RTC::GetRamFuture input;
		int ok = rtc.get_ram(LightProxy<RTC::GetRamFuture>{input});
		out << F("\nget_ram()=") << dec << ok << endl;
		out << F("handler.status()=") << hex << handler.status() << endl;
		future::FutureStatus status = input.status();
		out << F("status=") << status << endl;
		out << F("data await()=") << input.await() << endl;
		out << F("error()=") << dec << input.error() << endl;
		RTC::GetRamFuture::OUT result;
		if (input.get(result))
		{
			out << F("get() OK") << endl;
			for (uint16_t i = 0; i < result.size(); ++i)
				out << F("result[") << dec << i << F("] = ") << result[i] << endl;
		}
		else
		{
			out << F("get() KO") << endl;
		}
	}

	handler.end();
}
