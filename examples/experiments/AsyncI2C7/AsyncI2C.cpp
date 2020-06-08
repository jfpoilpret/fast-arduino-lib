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

#ifdef TWCR
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

class RTC : public i2c::I2CDevice<i2c::I2CMode::STANDARD>
{
	using PARENT = i2c::I2CDevice<i2c::I2CMode::STANDARD>;

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
	int set_ram(SetRamFuture& future)
	{
		//TODO add argument to write()
		return launch_commands(future, {write(), write(i2c::I2CFinish::FUTURE_FINISH)});
	}

	//TODO Future + method to read all RAM

private:
	static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
	static constexpr const uint8_t RAM_START = 0x08;
	static constexpr const uint8_t RAM_END = 0x40;
	static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;
};

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
#ifdef TWCR
	I2CHANDLER handler{i2c_buffer, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, i2c_hook};
#else
	I2CHANDLER handler{i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, i2c_hook};
#endif
	out << F("Before handler.begin()") << endl;
	out << boolalpha << showbase;

	handler.begin();

	constexpr uint8_t RAM_SIZE = rtc.ram_size();
	constexpr uint8_t MAX_READ = (RAM_SIZE < MAX_FUTURES ? RAM_SIZE : MAX_FUTURES);

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

	handler.end();
}
