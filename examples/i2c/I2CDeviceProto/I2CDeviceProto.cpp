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
 * I2C device (not already supported by FastArduino) works.
 * That helps creating a new specific support API for that device for reuse in 
 * other programs and potential integration to FastArduino project.
 * To ease wiring and debugging, I suggest using a real Arduino board (I typically 
 * use UNO) and a small breadboard for connecting the I2C device.
 * 
 * Wiring:
 * NB: you should add pullup resistors (10K-22K typically) on both SDA and SCL lines.
 * - on Arduino UNO:
 *   - A4 (PC4, SDA): connected to I2C SDA pin
 *   - A5 (PC5, SCL): connected to I2C SCL pin
 *   - direct USB access (traces output)
 */

#include <fastarduino/time.h>
#include <fastarduino/future.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/i2c_device_utilities.h>
#include <fastarduino/utilities.h>
#include <fastarduino/uart.h>
#include <fastarduino/i2c_debug.h>

// I2C Device specific constants go here
//======================================
static constexpr const i2c::I2CMode MODE = i2c::I2CMode::FAST;
static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;

static constexpr const uint8_t DEBUG_SIZE = 32;
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
using MANAGER = i2c::I2CSyncStatusDebugManager<MODE, DEBUGGER&, DEBUGGER&>;
#define DEBUG(OUT) debugger.trace(OUT)

// The following type aliases will be useful for declaring proper Futures and calling I2CDevice API
using PARENT = i2c::I2CDevice<MANAGER>;
template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

// Define vectors we need in the example
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<board::USART::USART0>)
REGISTER_FUTURE_NO_LISTENERS()

// UART for traces
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<board::USART::USART0> uart{output_buffer};
static streams::ostream out = uart.out();

// Subclass I2CDevice to make protected methods available
class PublicDevice: public PARENT
{
public:
	PublicDevice(MANAGER& manager): PARENT{manager, DEVICE_ADDRESS, i2c::Mode<MODE>{}, true} {}
	friend int main();
};

using streams::endl;
using streams::dec;
using streams::hex;

int main()
{
	board::init();
	sei();
	
	uart.begin(115200);
	out.width(2);
	
	// Start TWI interface
	//====================
	DEBUGGER debugger;
	MANAGER manager{debugger, debugger};
	manager.begin();
	out << streams::boolalpha << streams::unitbuf;
	out << F("I2C interface started") << endl;
	
	PublicDevice device{manager};
	
	// Init I2C device if needed

	// Output all debug traces
	DEBUG(out);
	
	// Loop to show measures
	while (true)
	{
		// Read measures and display them to UART

		// Output all debug traces
		DEBUG(out);

		time::delay_ms(1000);
	}
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End") << endl;
}
