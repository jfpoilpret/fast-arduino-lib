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
 * WinBond W25Q80BV SPI flash memory example.
 * This program shows usage of FastArduino support for SPI and WinBond device.
 * It checks all WinBond API implemented by WinBond.hh and traces all result to serial.
 * 
 * Wiring:
 * - WinBond IC:
 *   - /WP : connect to Vcc
 *   - /HOLD: connect to Vcc
 *   - 100nF cap between Vcc and GND
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D1 (TX) used for tracing program activities
 *   - D13 (SCK), D12 (MISO), D11 (MOSI), D7 (CS): SPI interface to WinBond
 * - on Arduino LEONARDO:
 *   - D1 (TX) used for tracing program activities
 *   - Board-ICSP (SCK, MISO, MOSI), D7 (CS): SPI interface to WinBond
 * - on Arduino MEGA:
 *   - D1 (TX) used for tracing program activities
 *   - D52 (SCK), D50 (MISO), D51 (MOSI), D7 (CS): SPI interface to WinBond
 * - on ATtinyX4 based boards:
 *   - D1 (TX) used for tracing program activities
 *   - D4 (SCK), D6 (MISO), D5 (MOSI), D7 (CS): SPI interface to WinBond
 * - on ATtinyX5 based boards:
 *   - D4 (TX) used for tracing program activities
 *   - D0 (MISO), D1 (MOSI), D2 (SCK), D3 (CS): SPI interface to WinBond
 * - on ATmega644 based boards:
 *   - D25 (PD1): TX output used for tracing program activities
 *   - D12 (PB4, CS), D13 (PB5, MOSI), D14 (PB6, MISO), D15 (PB7, SCK): SPI interface to WinBond
 */

#include <fastarduino/devices/winbond.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PD7;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const size_t DATA_SIZE = 256;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PE6;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const size_t DATA_SIZE = 256;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART1;
REGISTER_UATX_ISR(1)
#elif defined (ARDUINO_MEGA)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PH4;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const size_t DATA_SIZE = 256;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#include <fastarduino/soft_uart.h>
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
constexpr const board::DigitalPin CS = board::DigitalPin::D7_PA7;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const size_t DATA_SIZE = 128;
#elif defined (BREADBOARD_ATTINYX5)
#include <fastarduino/soft_uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D3_PB3;
constexpr const board::DigitalPin TX = board::DigitalPin::D4_PB4;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const size_t DATA_SIZE = 128;
#elif defined (BREADBOARD_ATMEGAXX4P)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D12_PB4;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const size_t DATA_SIZE = 256;
// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

static uint8_t data[DATA_SIZE];

constexpr const uint32_t PAGE = 0x010000;

using namespace streams;

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
	devices::WinBond<CS> flash;
	time::delay_ms(1000);
	
	out << F("S: ") << hex << flash.status().value << endl;
	uint64_t id = flash.read_unique_ID();
	out << F("UID: ") << hex << uint16_t(id >> 48) << ' ' << uint16_t(id >> 32) << ' ' 
					<< uint16_t(id >> 16) << ' ' << uint16_t(id) << endl;
	devices::WinBond<CS>::Device device = flash.read_device();
	out << F("M ID: ") << hex << device.manufacturer_ID << endl;
	out << F("D ID: ") << hex << device.device_ID << endl;

	out << F("B4 RD 1 pg, S: ") << hex << flash.status().value << endl;
	flash.read_data(PAGE, data, sizeof data);
	out << F("Af RD, S: ") << hex << flash.status().value << endl;

	out << F("Pg RD:") << endl;
	for (uint16_t i = 0; i < sizeof data; ++i)
	{
		out << hex << data[i] << ' ';
		if ((i + 1) % 16 == 0)
			out << endl;
	}
	out << endl;
	
	out << F("B4 erase, S: ") << hex << flash.status().value << endl;
	flash.enable_write();
	out << F("Af enable WR, S: ") << hex << flash.status().value << endl;
	flash.erase_sector(PAGE);
	out << F("Af erase, S: ") << hex << flash.status().value << endl;

	flash.wait_until_ready(10);
	out << F("Af wait, S: ") << hex << flash.status().value << endl;

	for (uint16_t i = 0; i < sizeof data; ++i)
		data[i] = uint8_t(i);

	out << F("B4 WR, S: ") << hex << flash.status().value << endl;
	flash.enable_write();
	flash.write_page(PAGE, data, (DATA_SIZE >= 256 ? 0 : DATA_SIZE));
	out << F("Af WR, S: ") << hex << flash.status().value << endl;
	
	flash.wait_until_ready(10);
	out << F("Af wait, S: ") << hex << flash.status().value << endl;
	
	for (uint16_t i = 0; i < sizeof data; ++i)
		data[i] = 0;
	
	out << F("B4 RD 1 byte, S: ") << hex << flash.status().value << endl;
	uint8_t value = flash.read_data(PAGE + sizeof(data) / 2);
	out << F("RD ") << value << F(", S: ") << hex << flash.status().value << endl;
	
	out << F("B4 RD 1 pg, S: ") << hex << flash.status().value << endl;
	flash.read_data(PAGE, data, sizeof data);
	out << F("Af RD, S: ") << hex << flash.status().value << endl;

	out << F("Pg RD:") << endl;
	for (uint16_t i = 0; i < sizeof data; ++i)
	{
		out << hex << data[i] << ' ';
		if ((i + 1) % 16 == 0)
			out << endl;
	}

	out << F("\nFinished") << endl;
}
