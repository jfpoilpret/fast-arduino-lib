//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * MCP3X0X SPI multiple ADC example.
 * This program just checks compilation of all MCP3x0x chips support.
 * 
 * No Wiring (compilation only):
 */

#include <fastarduino/devices/mcp3001.h>
#include <fastarduino/devices/mcp3002.h>
#include <fastarduino/devices/mcp3004.h>
#include <fastarduino/devices/mcp3008.h>
#include <fastarduino/devices/mcp3201.h>
#include <fastarduino/devices/mcp3202.h>
#include <fastarduino/devices/mcp3204.h>
#include <fastarduino/devices/mcp3208.h>
#include <fastarduino/devices/mcp3301.h>
#include <fastarduino/devices/mcp3302.h>
#include <fastarduino/devices/mcp3304.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PD7;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#elif defined(ARDUINO_LEONARDO)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PE6;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART1;
REGISTER_UATX_ISR(1)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#elif defined (ARDUINO_MEGA)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PH4;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#elif defined (BREADBOARD_ATTINYX4)
#include <fastarduino/soft_uart.h>
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PA7;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UATX<TX>)
#elif defined (BREADBOARD_ATTINYX5)
#include <fastarduino/soft_uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D3_PB3;
constexpr const board::DigitalPin TX = board::DigitalPin::D4_PB4;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UATX<TX>)
#elif defined (BREADBOARD_ATMEGAXX4P)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PA7;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace streams;
using devices::mcp3x0x::MCP3008Channel;
using devices::mcp3x0x::MCP3004Channel;
using devices::mcp3x0x::MCP3002Channel;
using devices::mcp3x0x::MCP3001Channel;
using devices::mcp3x0x::MCP3208Channel;
using devices::mcp3x0x::MCP3204Channel;
using devices::mcp3x0x::MCP3202Channel;
using devices::mcp3x0x::MCP3201Channel;
using devices::mcp3x0x::MCP3301Channel;
using devices::mcp3x0x::MCP3302Channel;
using devices::mcp3x0x::MCP3304Channel;

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Start UART
#if defined (BREADBOARD_ATTINYX4) || defined (BREADBOARD_ATTINYX5)
	serial::soft::UATX<TX> uart{output_buffer};
#else
	serial::hard::UATX<UART> uart{output_buffer};
#endif
	uart.begin(115200);
	ostream out = uart.out();
	
	out << F("Started") << endl;

	spi::init();

	devices::mcp3x0x::MCP3008<CS> adc3008;
	out << F("MCP3008") << endl;
	out << F("CH0 = ") << adc3008.read_channel(MCP3008Channel::CH0) << endl;
	out << F("CH1 = ") << adc3008.read_channel(MCP3008Channel::CH1) << endl;
	out << F("CH2 = ") << adc3008.read_channel(MCP3008Channel::CH2) << endl;
	out << F("CH3 = ") << adc3008.read_channel(MCP3008Channel::CH3) << endl;
	out << F("CH4 = ") << adc3008.read_channel(MCP3008Channel::CH4) << endl;
	out << F("CH5 = ") << adc3008.read_channel(MCP3008Channel::CH5) << endl;
	out << F("CH6 = ") << adc3008.read_channel(MCP3008Channel::CH6) << endl;
	out << F("CH7 = ") << adc3008.read_channel(MCP3008Channel::CH7) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3004<CS> adc3004;
	out << F("MCP3004") << endl;
	out << F("CH0 = ") << adc3004.read_channel(MCP3004Channel::CH0) << endl;
	out << F("CH1 = ") << adc3004.read_channel(MCP3004Channel::CH1) << endl;
	out << F("CH2 = ") << adc3004.read_channel(MCP3004Channel::CH2) << endl;
	out << F("CH3 = ") << adc3004.read_channel(MCP3004Channel::CH3) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3002<CS> adc3002;
	out << F("MCP3002") << endl;
	out << F("CH0 = ") << adc3002.read_channel(MCP3002Channel::CH0) << endl;
	out << F("CH1 = ") << adc3002.read_channel(MCP3002Channel::CH1) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3001<CS> adc3001;
	out << F("MCP3001") << endl;
	out << F("CH0 = ") << adc3001.read_channel(MCP3001Channel::CH0) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3208<CS> adc3208;
	out << F("MCP3208") << endl;
	out << F("CH0 = ") << adc3208.read_channel(MCP3208Channel::CH0) << endl;
	out << F("CH1 = ") << adc3208.read_channel(MCP3208Channel::CH1) << endl;
	out << F("CH2 = ") << adc3208.read_channel(MCP3208Channel::CH2) << endl;
	out << F("CH3 = ") << adc3208.read_channel(MCP3208Channel::CH3) << endl;
	out << F("CH4 = ") << adc3208.read_channel(MCP3208Channel::CH4) << endl;
	out << F("CH5 = ") << adc3208.read_channel(MCP3208Channel::CH5) << endl;
	out << F("CH6 = ") << adc3208.read_channel(MCP3208Channel::CH6) << endl;
	out << F("CH7 = ") << adc3208.read_channel(MCP3208Channel::CH7) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3204<CS> adc3204;
	out << F("MCP3204") << endl;
	out << F("CH0 = ") << adc3204.read_channel(MCP3204Channel::CH0) << endl;
	out << F("CH1 = ") << adc3204.read_channel(MCP3204Channel::CH1) << endl;
	out << F("CH2 = ") << adc3204.read_channel(MCP3204Channel::CH2) << endl;
	out << F("CH3 = ") << adc3204.read_channel(MCP3204Channel::CH3) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3202<CS> adc3202;
	out << F("MCP3202") << endl;
	out << F("CH0 = ") << adc3202.read_channel(MCP3202Channel::CH0) << endl;
	out << F("CH1 = ") << adc3202.read_channel(MCP3202Channel::CH1) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3201<CS> adc3201;
	out << F("MCP3201") << endl;
	out << F("CH0 = ") << adc3201.read_channel(MCP3201Channel::CH0) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3301<CS> adc3301;
	out << F("MCP3301") << endl;
	out << F("DIFF = ") << adc3301.read_channel(MCP3301Channel::DIFF) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3302<CS> adc3302;
	out << F("MCP3302") << endl;
	out << F("CH0 = ") << adc3302.read_channel(MCP3302Channel::CH0) << endl;
	out << F("CH1 = ") << adc3302.read_channel(MCP3302Channel::CH1) << endl;
	out << F("CH2 = ") << adc3302.read_channel(MCP3302Channel::CH2) << endl;
	out << F("CH3 = ") << adc3302.read_channel(MCP3302Channel::CH3) << endl;
	out << F("CH0-CH1 = ") << adc3302.read_channel(MCP3302Channel::CH0_CH1) << endl;
	out << F("CH1-CH0 = ") << adc3302.read_channel(MCP3302Channel::CH1_CH0) << endl;
	out << F("CH2-CH3 = ") << adc3302.read_channel(MCP3302Channel::CH2_CH3) << endl;
	out << F("CH3-CH2 = ") << adc3302.read_channel(MCP3302Channel::CH3_CH2) << endl;
	time::delay_ms(1000);

	devices::mcp3x0x::MCP3304<CS> adc3304;
	out << F("MCP3304") << endl;
	out << F("CH0 = ") << adc3304.read_channel(MCP3304Channel::CH0) << endl;
	out << F("CH1 = ") << adc3304.read_channel(MCP3304Channel::CH1) << endl;
	out << F("CH2 = ") << adc3304.read_channel(MCP3304Channel::CH2) << endl;
	out << F("CH3 = ") << adc3304.read_channel(MCP3304Channel::CH3) << endl;
	out << F("CH4 = ") << adc3304.read_channel(MCP3304Channel::CH4) << endl;
	out << F("CH5 = ") << adc3304.read_channel(MCP3304Channel::CH5) << endl;
	out << F("CH6 = ") << adc3304.read_channel(MCP3304Channel::CH6) << endl;
	out << F("CH7 = ") << adc3304.read_channel(MCP3304Channel::CH7) << endl;
	out << F("CH0-CH1 = ") << adc3304.read_channel(MCP3304Channel::CH0_CH1) << endl;
	out << F("CH1-CH0 = ") << adc3304.read_channel(MCP3304Channel::CH1_CH0) << endl;
	out << F("CH2-CH3 = ") << adc3304.read_channel(MCP3304Channel::CH2_CH3) << endl;
	out << F("CH3-CH2 = ") << adc3304.read_channel(MCP3304Channel::CH3_CH2) << endl;
	out << F("CH4-CH5 = ") << adc3304.read_channel(MCP3304Channel::CH4_CH5) << endl;
	out << F("CH5-CH4 = ") << adc3304.read_channel(MCP3304Channel::CH5_CH4) << endl;
	out << F("CH6-CH7 = ") << adc3304.read_channel(MCP3304Channel::CH6_CH7) << endl;
	out << F("CH7-CH6 = ") << adc3304.read_channel(MCP3304Channel::CH7_CH6) << endl;
	time::delay_ms(1000);

}
