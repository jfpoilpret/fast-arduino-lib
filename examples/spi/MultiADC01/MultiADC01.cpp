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
 * MCP3008 SPI multiple ADC example.
 * This program shows usage of FastArduino support for SPI and MCP3008 device.
 * 
 * Wiring:
 * - MCP3008 IC:
 *   - CH0-8 : connect to 8 pots wiper (terminals connected to GND and 5V) 
 *   - Vref connected to 5V
 *   - AGND connected to GND
 *   - 100nF cap between Vcc and GND
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D1 (TX) used for tracing program activities
 *   - D13 (SCK), D12 (MISO), D11 (MOSI), D7 (CS): SPI interface to MCP3008
 * - on Arduino LEONARDO:
 *   - D1 (TX) used for tracing program activities
 *   - Board-ICSP (SCK, MISO, MOSI), D7 (CS): SPI interface to MCP3008
 * - on Arduino MEGA:
 *   - D1 (TX) used for tracing program activities
 *   - D52 (SCK), D50 (MISO), D51 (MOSI), D7 (CS): SPI interface to MCP3008
 * - on ATtinyX4 based boards:
 *   - D1 (TX) used for tracing program activities
 *   - D4 (SCK), D6 (MISO), D5 (MOSI), D7 (CS): SPI interface to MCP3008
 * - on ATtinyX5 based boards:
 *   - D4 (TX) used for tracing program activities
 *   - D0 (MISO), D1 (MOSI), D2 (SCK), D3 (CS): SPI interface to MCP3008
 * - on ATmega644 based boards:
 *   - D25 (PD1): TX output used for tracing program activities
 *   - D12 (PB4, CS), D13 (PB5, MOSI), D14 (PB6, MISO), D15 (PB7, SCK): SPI interface to MCP3008
 */

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
#elif defined (BREADBOARD_ATMEGA644P)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D12_PB4;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace streams;
using devices::mcp3x0x::MCP3008Channel;

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
	devices::mcp3x0x::MCP3008<CS> adc;
	time::delay_ms(1000);
	
	while (true)
	{
		out << F("CH0 = ") << adc.read_channel(MCP3008Channel::CH0) << endl;
		out << F("CH1 = ") << adc.read_channel(MCP3008Channel::CH1) << endl;
		out << F("CH2 = ") << adc.read_channel(MCP3008Channel::CH2) << endl;
		out << F("CH3 = ") << adc.read_channel(MCP3008Channel::CH3) << endl;
		out << F("CH4 = ") << adc.read_channel(MCP3008Channel::CH4) << endl;
		out << F("CH5 = ") << adc.read_channel(MCP3008Channel::CH5) << endl;
		out << F("CH6 = ") << adc.read_channel(MCP3008Channel::CH6) << endl;
		out << F("CH7 = ") << adc.read_channel(MCP3008Channel::CH7) << endl;
		time::delay_ms(1000);
	}
}
