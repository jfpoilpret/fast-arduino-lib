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
 * BlueFruit BLE Friend SPI example.
 * This program shows usage of FastArduino support for SPI and BlueFruit SPI device.
 * 
 * Wiring:
 * - BlueFruit breakout:
 * 	 - TODO
 * - on ATmega328P based boards (including Arduino UNO):
 * 	 - TODO other (IRQ)
 *   - D1 (TX) used for tracing program activities
 *   - D13 (SCK), D12 (MISO), D11 (MOSI), D7 (CS): SPI interface to WinBond
 */

#include <fastarduino/devices/bluefruit_spi.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#include <fastarduino/uart.h>
constexpr const board::DigitalPin CS = board::DigitalPin::D8_PB0;
constexpr const board::DigitalPin IRQ = board::DigitalPin::D7_PD7;
constexpr const board::DigitalPin RESET = board::DigitalPin::D4_PD4;
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
using BLUE = devices::BlueFruitSpi<CS, IRQ, RESET>;
using CommandStatus = BLUE::CommandStatus;

ostream& operator<<(ostream& o, CommandStatus status)
{
	switch (status)
	{
		case CommandStatus::NONE:
		return o << F("NONE");

		case CommandStatus::OK:
		return o << F("OK");

		case CommandStatus::ERROR:
		return o << F("ERROR");

		case CommandStatus::TIMEOUT:
		return o << F("TIMEOUT");
	}
}

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Start UART
	serial::hard::UATX<UART> uart{output_buffer};
	uart.begin(115200);
	ostream out = uart.out();
	
	out << F("Started") << endl;

	spi::init();
	BLUE ble;
	ble.begin();
	time::delay_ms(1000);
	out << F("ATZ") << endl;
//	CommandStatus status = ble.at_command("ATZ", true);
	CommandStatus status = ble.at_command("ATZ", false);
	out << F("status=") << status << endl;
	const uint8_t* result = ble.latest_reply();
	out << F("result = ") << (const char*)(result) << endl;
	
	out << F("\nFinished") << endl;
}
