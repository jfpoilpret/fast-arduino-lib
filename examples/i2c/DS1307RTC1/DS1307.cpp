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
 */

#include <fastarduino/time.h>
#include <fastarduino/new_i2c_device.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static i2c::I2CCommand i2c_buffer[I2C_BUFFER_SIZE];
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static i2c::I2CCommand i2c_buffer[I2C_BUFFER_SIZE];
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
#else
#error "Current target is not yet supported!"
#endif

// UART buffer for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];

// DS1307 specific
static constexpr uint8_t DEVICE_ADDRESS = 0x68 << 1;

// Subclass I2CDevice to make protected methods available
class PublicDevice: public i2c::I2CDevice<i2c::I2CMode::STANDARD>
{
public:
	PublicDevice(MANAGER& manager): i2c::I2CDevice<i2c::I2CMode::STANDARD>{manager, DEVICE_ADDRESS} {}
	friend int main();
};

union BCD
{
	struct
	{
		uint8_t units	:4;
		uint8_t tens	:4;
	};
	uint8_t two_digits;
};

struct RealTime 
{
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
	
	// Initialize FutureManager
	future::FutureManager<MAX_FUTURES> future_manager;

	// Start TWI interface
	//====================
	// Initialize I2C async handler
#if I2C_TRUE_ASYNC
	i2c::I2CManager<i2c::I2CMode::STANDARD> manager{i2c_buffer};
#else
	i2c::I2CManager<i2c::I2CMode::STANDARD> manager;
#endif
	manager.begin();
	out << "I2C interface started" << endl;
	out << "status #1 " << manager.status() << endl;
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
	future::Future<void, uint8_t> f1{0};
	int error1 = rtc.launch_commands(f1, {rtc.write(0, i2c::I2CFinish::FUTURE_FINISH)});
	future::Future<void, RealTime> f2{init_time};
	int error2 = rtc.launch_commands(f2, {rtc.write(0, i2c::I2CFinish::FUTURE_FINISH)});
	out << "status #2 " << manager.status() << endl;
	out << "error1 " << error1 << endl;
	out << "error2 " << error2 << endl;
	out << f1.await() << endl;
	out << f2.await() << endl;

	time::delay_ms(2000);
	
	// Read clock
	//============
	RealTime time;
	f1 = future::Future<void, uint8_t>{0};
	error1 = rtc.launch_commands(f1, {rtc.write()});
	out << "status #3 " << manager.status() << endl;
	future::Future<RealTime, void> f3{};
	error2 = rtc.launch_commands(f3, {rtc.read()});
	out << "status #4 " << manager.status() << endl;
	out << "error1 " << error1 << endl;
	out << "error2 " << error2 << endl;
	out << f1.await() << endl;
	out << f3.await() << endl;

	f3.get(time);
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
	out << "status #5 " << manager.status() << endl;
	out << "End" << endl;
}
