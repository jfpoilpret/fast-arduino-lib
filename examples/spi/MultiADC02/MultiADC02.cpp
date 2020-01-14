//   Copyright 2016-2019 Jean-Francois Poilpret
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
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PD7;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PE6;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART1;
REGISTER_UATX_ISR(1)
#elif defined (ARDUINO_MEGA)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PH4;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#include <fastarduino/soft_uart.h>
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PA7;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
#elif defined (BREADBOARD_ATTINYX5)
#include <fastarduino/soft_uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D3_PB3;
constexpr const board::DigitalPin TX = board::DigitalPin::D4_PB4;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
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

}
