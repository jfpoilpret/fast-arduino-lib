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

#include <fastarduino/time.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/utilities.h>
#include <fastarduino/uart.h>

#include <math.h>

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)

// UART for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<board::USART::USART0> uart{output_buffer};
static streams::ostream out = uart.out();

// I2C Device specific stuff goes here
//=====================================
static constexpr const i2c::I2CMode MODE = i2c::I2CMode::Fast;
static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;

// Subclass I2CDevice to make protected methods available
class PublicDevice: public i2c::I2CDevice<MODE>
{
public:
	PublicDevice(MANAGER& manager): I2CDevice(manager) {}
	friend int main();
};

using streams::endl;
using streams::dec;
using streams::hex;

void trace_i2c_status(uint8_t expected_status, uint8_t actual_status)
{
	if (expected_status != actual_status)
		out << F("status expected = ") << expected_status << F(", actual = ") << actual_status << endl;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
	uart.register_handler();
	uart.begin(115200);
	out.width(2);
	
	// Start TWI interface
	//====================
	PublicDevice::MANAGER manager{trace_i2c_status};
	manager.begin();
	out << F("I2C interface started") << endl;
	
	PublicDevice device{manager};
	
	// Init MPU6050: wake it up, set ranges for accelerometer and gyroscope
	
	//TODO loop to show measures
	while (true)
	{
		// Read accel/temperature/gyro in one read
		time::delay_ms(1000);
	}
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End") << endl;
}
