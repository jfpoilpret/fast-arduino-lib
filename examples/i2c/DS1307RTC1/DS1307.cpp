//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * Check DS1307 I2C device (real-time clock) and display output to the UART console.
 * This program does not use FastArduino DS1307 device and directly uses 
 * i2c::I2CDevice instead; it was originally used as a way to understand the device
 * and then build the FastArduino DS1307 API.
 * 
 * Wiring:
 * NB: you should add pullup resistors (10K-22K typically) on both SDA and SCL lines.
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to DS1307 SDA pin
 *   - A5 (PC5, SCL): connected to DS1307 SCL pin
 *   - direct USB access
 * - on Arduino LEONARDO:
 *   - D2 (PD1, SDA): connected to DS1307 SDA pin
 *   - D3 (PD0, SCL): connected to DS1307 SCL pin
 *   - direct USB access
 * - on Arduino MEGA:
 *   - D20 (PD1, SDA): connected to DS1307 SDA pin
 *   - D21 (PD0, SCL): connected to DS1307 SCL pin
 *   - direct USB access
 * - on ATtinyX4 based boards:
 *   - D6 (PA6, SDA): connected to DS1307 SDA pin
 *   - D4 (PA4, SCL): connected to DS1307 SCL pin
 *   - D8 (PB0, TX): connected to SerialUSB converter
 * - on ATtinyX5 based boards:
 *   - D0 (PB0, SDA): connected to DS1307 SDA pin
 *   - D2 (PB2, SCL): connected to DS1307 SCL pin
 *   - D3 (PB3, TX): connected to SerialUSB converter
 * - on ATmega644 based boards:
 *   - D17 (PC1, SDA): connected to DS1307 SDA pin
 *   - D16 (PC0, SCL): connected to DS1307 SCL pin
 *   - D25 (PD1): TX output connected to SerialUSB converter
 */

#include <fastarduino/time.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/i2c_debug.h>
#include <fastarduino/i2c_status.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(1)
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#elif defined(BREADBOARD_ATTINYX5)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D3_PB3;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

#if HARDWARE_UART
	REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#else
	REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UATX<TX>)
#endif

#define DEBUG_I2C

// UART buffer for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];

#ifdef DEBUG_I2C
static constexpr const uint8_t DEBUG_SIZE = 32;
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
#	if I2C_TRUE_ASYNC
using MANAGER = i2c::I2CAsyncStatusDebugManager<
	i2c::I2CMode::STANDARD, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, DEBUGGER&, DEBUGGER&>;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using MANAGER = i2c::I2CSyncStatusDebugManager<i2c::I2CMode::STANDARD, DEBUGGER&, DEBUGGER&>;
#	endif
#define DEBUG(OUT) debugger.trace(OUT)
#define SHOW_STATUS(OUT)

#else

using STATUS = i2c::status::I2CLatestStatusHolder;
#	if I2C_TRUE_ASYNC
using MANAGER = i2c::I2CAsyncStatusManager<
	i2c::I2CMode::STANDARD, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, STATUS&>;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using MANAGER = i2c::I2CSyncStatusManager<i2c::I2CMode::STANDARD, STATUS&>;
#	endif
#define DEBUG(OUT)
#define SHOW_STATUS(OUT) OUT << streams::hex << status_holder.latest_status() << streams::endl
#endif

#if I2C_TRUE_ASYNC
REGISTER_I2C_ISR(MANAGER)
#endif

// DS1307 specific
static constexpr uint8_t DEVICE_ADDRESS = 0x68 << 1;

using PARENT = i2c::I2CDevice<MANAGER>;
template<typename T> using PROXY = typename PARENT::template PROXY<T>;
template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

// Subclass I2CDevice to make protected methods available
class PublicDevice: public PARENT
{
public:
	PublicDevice(MANAGER& manager): PARENT{manager, DEVICE_ADDRESS, i2c::I2C_STANDARD, true} {}
	friend int main();
};

union BCD
{
	BCD() = default;
	struct
	{
		uint8_t units	:4;
		uint8_t tens	:4;
	};
	uint8_t two_digits = 0;
};

struct RealTime 
{
	RealTime() = default;
	BCD seconds;
	BCD minutes;
	BCD hours;
	uint8_t weekday;
	BCD day;
	BCD month;
	BCD	year;
};

const uint32_t I2C_FREQUENCY = 100000;

using namespace streams;

static constexpr uint8_t MAX_FUTURES = 8;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
#if HARDWARE_UART
	serial::hard::UATX<UART> uart{output_buffer};
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);
	ostream out = uart.out();
	out.width(0);
	out.setf(ios::hex, ios::basefield);
	out << "Start" << endl;
	
	// Start TWI interface
	//====================
	// Initialize I2C async handler
#ifdef DEBUG_I2C
	DEBUGGER debugger;
#else
	STATUS status_holder;
#endif

#if I2C_TRUE_ASYNC
#	ifdef DEBUG_I2C
	MANAGER manager{i2c_buffer, debugger, debugger};
#	else
	MANAGER manager{i2c_buffer, status_holder};
#	endif
#else
#	ifdef DEBUG_I2C
	MANAGER manager{debugger, debugger};
#	else
	MANAGER manager{status_holder};
#	endif
#endif

	manager.begin();
	out << "I2C interface started" << endl;
	time::delay_ms(1000);
	
	PublicDevice rtc{manager};
	
	RealTime init_time;
	init_time.day.two_digits = 0x11;
	init_time.month.two_digits = 0x06;
	init_time.year.two_digits = 0x17;
	init_time.weekday = 1;
	init_time.hours.two_digits = 0x12;
	init_time.minutes.two_digits = 0;
	init_time.seconds.two_digits = 0;
	
	// Initialize clock date
	//=======================
	//FIXME this is incorrect, to split one write transaction into 2 write transactions!
	FUTURE<void, uint8_t> f1{0};
	int error1 = rtc.launch_commands(f1, {rtc.write(0, true)});
	DEBUG(out);
	SHOW_STATUS(out);
	FUTURE<void, RealTime> f2{init_time};
	int error2 = rtc.launch_commands(f2, {rtc.write(0, true)});
	DEBUG(out);
	SHOW_STATUS(out);
	out << "error1 " << error1 << endl;
	out << "error2 " << error2 << endl;
	out << f1.await() << endl;
	out << f2.await() << endl;

	time::delay_ms(2000);
	
	// Read clock
	//============
	RealTime time;
	FUTURE<void, uint8_t> f3{0};
	error1 = rtc.launch_commands(f3, {rtc.write()});
	DEBUG(out);
	SHOW_STATUS(out);
	FUTURE<RealTime, void> f4{};
	error2 = rtc.launch_commands(f4, {rtc.read()});
	DEBUG(out);
	SHOW_STATUS(out);
	out << "error1 " << error1 << endl;
	out << "error2 " << error2 << endl;
	out << f3.await() << endl;
	out << f4.await() << endl;

	f4.get(time);
	out	<< "RTC: " 
		<< time.day.tens << time.day.units << '.'
		<< time.month.tens << time.month.units << '.'
		<< time.year.tens << time.year.units << ' '
		<< time.hours.tens << time.hours.units << ':'
		<< time.minutes.tens << time.minutes.units << ':'
		<< time.seconds.tens << time.seconds.units
		<< endl;
	
	// Stop TWI interface
	//===================
	manager.end();
	out << "End" << endl;
}
