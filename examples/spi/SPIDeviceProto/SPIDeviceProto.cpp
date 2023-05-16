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
 * This is a skeleton program to help connect, debug and understand how a given
 * SPI device (not already supported by FastArduino) works.
 * That helps creating a new specific support API for that device for reuse in 
 * other programs and potential integration to FastArduino project.
 * To ease wiring and debugging, I suggest using a real Arduino UNO board
 * and a small breadboard for connecting the SPI device.
 * 
 * This example shows how to start debugging support for MCP3008 chip, an 
 * 8-channel Analog-Digital Converter, which communication protocol is super 
 * simple (because the number of features for such a chip is quite limited).
 * In source code below, there are references to 
 * [MCP3008 datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/21295C.pdf).
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (SCK): connected to SPI device SCK pin
 *   - D12 (MISO): connected to SPI device MISO pin (sometimes called Dout)
 *   - D11 (MOSI): connected to SPI device MOSI pin (sometimes called Din)
 *   - D10 (SS): connected to SPI device CS pin
 *   - direct USB access (traces output)
 */

#include <fastarduino/spi.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/utilities.h>

// Define vectors we need in the example
REGISTER_UATX_ISR(0)

// UART for traces
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

// SPI Device specific stuff goes here
//=====================================

// Spec §1.0 (Clock frequency max 3.6MHz for Vdd=5V)
static constexpr const uint32_t SPI_CLOCK = 3'600'000UL;
static constexpr const spi::ChipSelect CHIP_SELECT = spi::ChipSelect::ACTIVE_LOW;
static constexpr const spi::DataOrder DATA_ORDER = spi::DataOrder::MSB_FIRST;
static constexpr const spi::Mode MODE = spi::Mode::MODE_0;

// For testing we use default SS pin as CS
static constexpr const board::DigitalPin CS = board::DigitalPin::D10_PB2;

static constexpr const spi::ClockRate CLOCK_RATE = spi::compute_clockrate(SPI_CLOCK);

// Subclass SPIDevice to make protected methods available from main()
class PublicDevice: public spi::SPIDevice<CS, CHIP_SELECT, CLOCK_RATE, MODE, DATA_ORDER>
{
public:
	PublicDevice(): SPIDevice() {}
	friend int main();
};

using streams::endl;
using streams::dec;
using streams::hex;

int main()
{
	board::init();
	sei();

	// Init UART output for traces
	serial::hard::UATX<board::USART::USART0> uart{output_buffer};
	uart.begin(115200);
	streams::ostream out = uart.out();
	out.width(2);
	
	// Start SPI interface
	spi::init();
	out << F("SPI initialized") << endl;
	
	PublicDevice device;
	
	// Start or init SPI device if needed
	
	// Loop to read and show measures
	while (true)
	{
		// Read measures and display them to UART

		// On MCP3008 we will perform single-ended analog-digital conversion on channel CH0
		out << F("Reading channel 0") << endl;
		// Spec §5.0
		device.start_transfer();
		// Spec §6.1, figure 6.1: send a start bit as a byte (left filled with 0s)
		device.transfer(0x01);
		// Spec §6.1, figure 6.1: send 4 command bits as a byte (right filled with 0s), and capture result (2 MSB)
		// Command bits are 1000 (single-ended input mode, channel CH0)
		uint8_t result1 = device.transfer(0x80);
		// Spec §6.1, figure 6.1: send an empty byte to capture returned result (8 LSB)
		uint8_t result2 = device.transfer(0x00);
		device.end_transfer();

		// Trace intermediate results (for debugging)
		out << F("Intermediate results:") << hex << result1 << ' ' << result2 << endl;
		// Combine result
		uint16_t value =  utils::as_uint16_t(result1 & 0x03, result2);
		out << F("Calculated value: ") << dec << value << endl;

		time::delay_ms(1000);
	}
	
	// Stop SPI device if needed
	out << F("End") << endl;
}
