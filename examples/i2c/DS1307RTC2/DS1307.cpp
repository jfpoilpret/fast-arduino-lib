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
 * This program uses FastArduino DS1307 support API.
 * 
 * Wiring:
 * NB: you should add pullup resistors (10K-22K typically) on both SDA and SCL lines.
 * WARNING: wiring is very sensitive for I2C connections! When using breadboard, ensure
 * wires connections are tight and stable.
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
#include <fastarduino/i2c_handler.h>
#include <fastarduino/devices/ds1307.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/i2c_debug.h>
#include <fastarduino/i2c_status.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t DEBUG_SIZE = 128;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t DEBUG_SIZE = 128;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t DEBUG_SIZE = 128;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(1)
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const uint8_t DEBUG_SIZE = 32;
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 32;
#elif defined(BREADBOARD_ATTINYX5)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const uint8_t DEBUG_SIZE = 32;
static constexpr const board::DigitalPin TX = board::DigitalPin::D3_PB3;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 32;
#elif defined (BREADBOARD_ATMEGA644P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t DEBUG_SIZE = 128;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

#define DEBUG_I2C
#define FORCE_SYNC

// UART buffer for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];

using devices::rtc::DS1307;
using devices::rtc::WeekDay;
using devices::rtc::tm;
using devices::rtc::SquareWaveFrequency;
using namespace streams;

#ifdef DEBUG_I2C
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
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
#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncStatusManager<
	i2c::I2CMode::STANDARD, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, STATUS&>;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using MANAGER = i2c::I2CSyncStatusManager<i2c::I2CMode::STANDARD, STATUS&>;
#	endif
#define DEBUG(OUT)
#define SHOW_STATUS(OUT) OUT << streams::hex << status_holder.latest_status() << streams::endl
#endif

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
REGISTER_I2C_ISR(MANAGER)
#endif

void display_ram(ostream& out, const uint8_t* data, uint8_t size)
{
	out << hex << F("RAM content\n");
	for (uint8_t i = 0; i < size; ++i)
	{
		if (!(i % 8)) out << endl;
		out << setw(2) << data[i] << ' ';
	}
	out << endl;
}

void display_time(ostream& out, const tm& time)
{
	out	<< dec << F("RTC: [") 
		<< uint8_t(time.tm_wday) << ']'
		<< time.tm_mday << '.'
		<< time.tm_mon << '.'
		<< time.tm_year << ' '
		<< time.tm_hour << ':'
		<< time.tm_min << ':'
		<< time.tm_sec
		<< endl;
}

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
	out << F("Start") << endl;

	// Start TWI interface
	//====================
#ifdef DEBUG_I2C
	DEBUGGER debugger;
#else
	STATUS status_holder;
#endif

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
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
	out << F("I2C interface started") << endl;
	SHOW_STATUS(out);
	time::delay_ms(1000);
	
	DS1307<MANAGER> rtc{manager};
	
	// read RAM content and print out
	uint8_t data[DS1307<MANAGER>::ram_size()];
	memset(data, 0, sizeof data);
	rtc.get_ram(0, data);
	SHOW_STATUS(out);
	display_ram(out, data, sizeof data);
	DEBUG(out);
	
	tm time1;
	time1.tm_hour = 8;
	time1.tm_min = 45;
	time1.tm_sec = 30;
	time1.tm_wday = WeekDay::TUESDAY;
	time1.tm_mday = 13;
	time1.tm_mon = 6;
	time1.tm_year = 17;
	
	// Initialize clock date
	//=======================
	rtc.set_datetime(time1);
	SHOW_STATUS(out);
	DEBUG(out);

	time::delay_ms(2000);
	
	// Read clock
	//============
	tm time2;
	rtc.get_datetime(time2);
	display_time(out, time2);
	SHOW_STATUS(out);
	DEBUG(out);
	
	// Enable output clock
	//====================
	rtc.enable_output(SquareWaveFrequency::FREQ_1HZ);
	SHOW_STATUS(out);
	DEBUG(out);
	
	// Provide 10 seconds delay to allow checking square wave output with an oscilloscope
	time::delay_ms(10000);
	
	rtc.disable_output(false);
	SHOW_STATUS(out);
	DEBUG(out);

	// write RAM content
	for (uint8_t i = 0; i < sizeof(data); ++i)
		data[i] = i;
	rtc.set_ram(0, data);
	SHOW_STATUS(out);
	DEBUG(out);
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End") << endl;
}
