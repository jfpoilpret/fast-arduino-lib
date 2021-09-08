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
 * This program uses FastArduino DS1307 support API in asynchronous mode (ATmega only)
 * along with an ISR callback.
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
 * - on ATmega644 based boards:
 *   - D17 (PC1, SDA): connected to DS1307 SDA pin
 *   - D16 (PC0, SCL): connected to DS1307 SCL pin
 *   - D25 (PD1): TX output connected to SerialUSB converter
 */

#include <fastarduino/time.h>
#include <fastarduino/i2c_handler.h>
#include <fastarduino/devices/ds1307.h>
#include <fastarduino/future.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/i2c_debug.h>
#include <fastarduino/i2c_status.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_MEGA)
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
#elif defined (BREADBOARD_ATMEGA644P)
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

#define DEBUG_I2C

// UART buffer for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];

using devices::rtc::DS1307;
using devices::rtc::WeekDay;
using devices::rtc::tm;
using devices::rtc::SquareWaveFrequency;
using future::FutureStatus;
using namespace streams;

#ifdef DEBUG_I2C
static constexpr const uint8_t DEBUG_SIZE = 32;
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
using MANAGER = i2c::I2CAsyncStatusDebugManager<
	i2c::I2CMode::STANDARD, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, DEBUGGER&, DEBUGGER&>;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#define DEBUG(OUT) debugger.trace(OUT)
#define SHOW_STATUS(OUT)
#else
using STATUS = i2c::status::I2CLatestStatusHolder;
using MANAGER = i2c::I2CAsyncStatusManager<
	i2c::I2CMode::STANDARD, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, STATUS&>;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#define DEBUG(OUT)
#define SHOW_STATUS(OUT) OUT << streams::hex << status_holder.latest_status() << streams::endl
#endif

using RTC = DS1307<MANAGER>;

class RTCAsyncHandler
{
public:
	RTCAsyncHandler()
	{
		interrupt::register_handler(*this);
	}

	FutureStatus status() const
	{
		return status_;
	}

	void reset()
	{
		status_ = FutureStatus::NOT_READY;
	}

private:
	void i2c_change(UNUSED i2c::I2CCallback callback, MANAGER::FUTURE_PROXY proxy)
	{
		status_ = proxy()->status();
	}

	volatile FutureStatus status_ = FutureStatus::NOT_READY;
	
	DECL_I2C_ISR_HANDLERS_FRIEND
};

REGISTER_I2C_ISR_METHOD(MANAGER, RTCAsyncHandler, &RTCAsyncHandler::i2c_change)

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

	RTCAsyncHandler handler;
	
	manager.begin();
	out << F("I2C interface started") << endl;
	SHOW_STATUS(out);
	time::delay_ms(1000);
	
	RTC rtc{manager};
	
	// Initialize clock date
	//=======================
	tm time1;
	time1.tm_hour = 8;
	time1.tm_min = 45;
	time1.tm_sec = 30;
	time1.tm_wday = WeekDay::TUESDAY;
	time1.tm_mday = 13;
	time1.tm_mon = 6;
	time1.tm_year = 17;
	RTC::SetDatetimeFuture set_date_future{time1};
	int error = rtc.set_datetime(set_date_future);
	out << F("set_datetime called asynchronously, error = ") << error << endl;

	out << F("await asynchronous set_datetime...") << endl;
	FutureStatus status;
	while ((status = handler.status()) == FutureStatus::NOT_READY)
	{
		time::delay_us(100);
	}
	out << F("set_datetime status = ") << status << endl;
	SHOW_STATUS(out);
	DEBUG(out);

	time::delay_ms(2000);
	
	// Read clock
	//============
	RTC::GetDatetimeFuture get_date_future;
	handler.reset();
	error = rtc.get_datetime(get_date_future);
	out << F("get_datetime called asynchronously, error = ") << error << endl;

	out << F("await asynchronous get_datetime...") << endl;
	while ((status = handler.status()) == FutureStatus::NOT_READY)
	{
		time::delay_us(100);
	}
	out << F("get_datetime status = ") << status << endl;

	tm time2;
	bool ok = get_date_future.get(time2);
	out << F("get() return ") << ok << endl;
	display_time(out, time2);
	SHOW_STATUS(out);
	DEBUG(out);
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End") << endl;
}
