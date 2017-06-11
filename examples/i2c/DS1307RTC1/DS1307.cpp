//   Copyright 2016-2017 Jean-Francois Poilpret
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
 * Blocking EEPROM Read and Writes.
 * This program shows usage of FastArduino EEPROM API.
 * It interfaces with user through the UART console and allows:
 * - writing values to EEPROM
 * - reading values to EEPROM
 * 
 * Wiring: TODO
 * - on ATmega328P based boards (including Arduino UNO):
 * - on Arduino MEGA:
 * - on ATtinyX4 based boards:
 *   - D1: TX output connected to Serial-USB allowing traces display on a PC terminal
 */

#include <fastarduino/time.h>
#include <fastarduino/i2c.h>

#if defined(ARDUINO_UNO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

// UART for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<UART> uart{output_buffer};
static streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();

// DS1307 specific
const uint8_t DEVICE_ADDRESS = 0x68 << 1;
union BCD
{
	struct
	{
		uint8_t units	:4;
		uint8_t tens	:4;
	};
	uint8_t two_digits;
};

//TODO check if packed attribute is really necessary
struct RealTime 
{
	BCD seconds;
	BCD minutes;
	BCD hours;
	uint8_t weekday;
	BCD day;
	BCD month;
	BCD	year;
} __attribute__ ((packed));

const uint32_t I2C_FREQUENCY = 100000;

// NOTE we use prescaler = 1 everywhere
constexpr uint8_t calculate_TWBR(uint32_t frequency)
{
	return (F_CPU / frequency - 16UL) / 2;
}

// TODO traits for: SDA, SCL pins
int main() __attribute__((OS_main));
int main()
{
	sei();
	
	uart.register_handler();
	uart.begin(115200);
	out.width(0);
	out.base(streams::FormatBase::Base::hex);
	out << "Start\n" << streams::flush;
	
	// Start TWI interface
	//====================
	i2c::I2CManager manager;
	manager.begin();
	out << "I2C interface started\n" << streams::flush;
	out << "status #1 " << manager.error() << '\n' << streams::flush;
	time::delay_ms(1000);
	
	i2c::I2CDevice rtc{manager};
	
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
	rtc.write(DEVICE_ADDRESS, uint8_t(0), true);
	rtc.write(DEVICE_ADDRESS, init_time, false, true);
	out << "status #2 " << manager.error() << '\n' << streams::flush;

	time::delay_ms(2000);
	
	// Read clock
	//============
	RealTime time;
	rtc.write(DEVICE_ADDRESS, uint8_t(0), true);
	out << "status #3 " << manager.error() << '\n' << streams::flush;
	rtc.read(DEVICE_ADDRESS, time);
	out << "status #4 " << manager.error() << '\n' << streams::flush;
	
	out	<< "RTC: " 
		<< time.day.tens << time.day.units << '.'
		<< time.month.tens << time.month.units << '.'
		<< time.year.tens << time.year.units << ' '
		<< time.hours.tens << time.hours.units << ':'
		<< time.minutes.tens << time.minutes.units << ':'
		<< time.seconds.tens << time.seconds.units << '\n'
		<< streams::flush;
	
	// Stop TWI interface
	//===================
	manager.end();
	out << "status #5 " << manager.error() << '\n' << streams::flush;
	out << "End\n" << streams::flush;
}
