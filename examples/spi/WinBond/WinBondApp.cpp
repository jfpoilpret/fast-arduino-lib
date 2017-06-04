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
 * WinBond W25Q80BV SPI flash memory example.
 * This program shows usage of FastArduino support for SPI and WinBond device.
 * It checks all WinBond API implemented by WinBond.hh and traces all result to serial.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D1 (TX) used for tracing program activities
 *   - D13 (SCK), D12 (MISO), D11 (MOSI), D7 (CS): SPI interface to WinBond
 * - on Arduino MEGA:
 *   - NOT SUPPORTED YET
 * - on ATtinyX4 based boards:
 *   - D1 (TX) used for tracing program activities
 *   - D4 (SCK), D6 (MISO), D5 (MOSI), D7 (CS): SPI interface to WinBond
 */

#include <util/delay.h>
#include <fastarduino/devices/winbond.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
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
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

static uint8_t data[DATA_SIZE];

constexpr const uint32_t PAGE = 0x010000;

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Start UART
#if defined (BREADBOARD_ATTINYX4)
	serial::soft::UATX<TX> uart{output_buffer};
	uart.begin(115200);
#else
	serial::hard::UATX<UART> uart{output_buffer};
	uart.register_handler();
	uart.begin(115200);
#endif
	streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();
	
	out << "Started\n";

	spi::init();
	devices::WinBond<CS> flash;
	_delay_ms(1000);
	
	out << "S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	uint64_t id = flash.read_unique_ID();
	out << "UID: " << streams::hex << uint16_t(id >> 48) << ' ' << uint16_t(id >> 32) << ' ' 
					<< uint16_t(id >> 16) << ' ' << uint16_t(id) << streams::endl;
	devices::WinBond<CS>::Device device = flash.read_device();
	out << "M ID: " << streams::hex << device.manufacturer_ID << streams::endl;
	out << "D ID: " << streams::hex << device.device_ID << streams::endl << streams::flush;

	out << "B4 RD 1 pg, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	flash.read_data(PAGE, data, sizeof data);
	out << "Af RD, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;

	out << "Pg RD:" << streams::endl << streams::flush;
	for (uint16_t i = 0; i < sizeof data; ++i)
	{
		out << streams::hex << data[i] << ' ';
		if ((i + 1) % 16 == 0)
			out << streams::endl << streams::flush;
	}
	out << streams::endl << streams::flush;
	
	out << "B4 erase, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	flash.enable_write();
	out << "Af enable WR, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	flash.erase_sector(PAGE);
	out << "Af erase, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;

	flash.wait_until_ready(10);
	out << "Af wait, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;

	for (uint16_t i = 0; i < sizeof data; ++i)
		data[i] = uint8_t(i);

	out << "B4 WR, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	flash.enable_write();
	flash.write_page(PAGE, data, sizeof data);
	out << "Af WR, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	
	flash.wait_until_ready(10);
	out << "Af wait, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	
	for (uint16_t i = 0; i < sizeof data; ++i)
		data[i] = 0;
	
	out << "B4 RD 1 byte, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	uint8_t value = flash.read_data(PAGE + sizeof(data) / 2);
	out << "RD " << value << ", S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	
	out << "B4 RD 1 pg, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;
	flash.read_data(PAGE, data, sizeof data);
	out << "Af RD, S: " << streams::hex << flash.status().value << streams::endl << streams::flush;

	out << "Pg RD:" << streams::endl << streams::flush;
	for (uint16_t i = 0; i < sizeof data; ++i)
	{
		out << streams::hex << data[i] << ' ';
		if ((i + 1) % 16 == 0)
			out << streams::endl << streams::flush;
	}

	out << "\nFinished\n" << streams::flush;
}
