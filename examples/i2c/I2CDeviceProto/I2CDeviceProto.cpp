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
#include <fastarduino/uart.h>

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)

// UART for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<board::USART::USART0> uart{output_buffer};
static streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();

// I2C Device specific stuff goes here
//=====================================
// HMC5883L specific
static constexpr const i2c::I2CMode MODE = i2c::I2CMode::Fast;
const uint8_t DEVICE_ADDRESS = 0x1E << 1;
const uint8_t NUM_REGISTERS = 13;

// Subclass I2CDevice to make protected methods available
class PublicDevice: public i2c::I2CDevice<MODE>
{
public:
	PublicDevice(MANAGER& manager): I2CDevice(manager) {}
	friend int main();
};

using streams::flush;

void trace_status(uint8_t expected_status, uint8_t actual_status)
{
	out << F("status expected = ") << expected_status << F(", actual = ") << actual_status << '\n' << flush;
}

void trace_registers(uint8_t registers[NUM_REGISTERS])
{
	out << F("HMC5883L registers\n");
	for (uint8_t i = 0; i < NUM_REGISTERS; ++i)
		out << F("    ") << i << F(": ") << registers[i] << '\n' << flush;
	out << flush;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
	uart.register_handler();
	uart.begin(115200);
	out.width(2);
	out.base(streams::FormatBase::Base::hex);
	out << F("Start\n") << flush;
	
	// Start TWI interface
	//====================
	i2c::I2CManager<MODE> manager{trace_status};
	manager.begin();
	out << F("I2C interface started\n") << flush;
	out << F("status #1 ") << manager.status() << '\n' << flush;
	
	PublicDevice compass{manager};
	uint8_t registers[NUM_REGISTERS];
	
	// Read device registers
	//======================
	compass.write(DEVICE_ADDRESS, uint8_t(0), i2c::BusConditions::START_NO_STOP);
	compass.read(DEVICE_ADDRESS, registers, i2c::BusConditions::REPEAT_START_STOP);
	trace_registers(registers);

	// Stop TWI interface
	//===================
	manager.end();
	out << F("status #2 ") << manager.status() << '\n' << flush;
	out << F("End\n") << flush;
}
