//   Copyright 2016-2018 Jean-Francois Poilpret
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

#include <fastarduino/time.h>
#include <fastarduino/i2c_manager.h>
#include <fastarduino/devices/ds1307.h>
#include <fastarduino/iomanip.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
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

using devices::rtc::DS1307;
using devices::rtc::WeekDay;
using devices::rtc::tm;
using devices::rtc::SquareWaveFrequency;
using namespace streams;

//FIXME out is not a global variable anymore hence it must be "passed" somehow to trace_status
void trace_status(uint8_t expected_status, uint8_t actual_status)
{
//	out << hex << F("status expected = ") << expected_status << F(", actual = ") << actual_status << endl;
}

void display_status(ostream& out, char index, uint8_t status)
{
	out << hex << F("status #") << index << ' ' << status << endl;
}

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
	uart.register_handler();
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);
	ostream out = uart.out();
	out << F("Start") << endl;
	
	// Start TWI interface
	//====================
	i2c::I2CManager<> manager{trace_status};
	manager.begin();
	out << F("I2C interface started") << endl;
	display_status(out, '1', manager.status());
	time::delay_ms(1000);
	
	DS1307 rtc{manager};
	
	// read RAM content and print out
	uint8_t data[DS1307::ram_size()];
	memset(data, 0,sizeof data);
	rtc.get_ram(0, data, sizeof data);
	display_status(out, '2', manager.status());
	display_ram(out, data, sizeof data);
	
	tm time1;
	time1.tm_hour = 8;
	time1.tm_min = 45;
	time1.tm_sec = 30;
	time1.tm_wday = WeekDay::Tuesday;
	time1.tm_mday = 13;
	time1.tm_mon = 6;
	time1.tm_year = 17;
	
	// Initialize clock date
	//=======================
	rtc.set_datetime(time1);
	display_status(out, '3', manager.status());

	time::delay_ms(2000);
	
	// Read clock
	//============
	tm time2;
	memset(&time2, 0,sizeof time2);
	rtc.get_datetime(time2);
	display_status(out, '4', manager.status());
	display_time(out, time2);
	
	// Enable output clock
	//====================
	rtc.enable_output(SquareWaveFrequency::FREQ_1HZ);
	display_status(out, '5', manager.status());
	
	// Provide 10 seconds delay to allow checking square ave output with an oscilloscope
	time::delay_ms(10000);
	
	rtc.disable_output(false);
	display_status(out, '6', manager.status());

	// write RAM content
	for (uint8_t i = 0; i < sizeof(data); ++i)
		data[i] = i;
	rtc.set_ram(0, data, sizeof data);
	display_status(out, '7', manager.status());
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End") << endl;
}
