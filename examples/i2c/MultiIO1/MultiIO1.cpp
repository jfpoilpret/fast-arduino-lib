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

#include <fastarduino/time.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/devices/mcp23017.h>

// constexpr const i2c::I2CMode I2C_MODE = i2c::I2CMode::Standard;
constexpr const i2c::I2CMode I2C_MODE = i2c::I2CMode::Fast;

using MCP = devices::MCP23017<I2C_MODE>;

static inline uint8_t shift_pattern(uint8_t pattern, uint8_t shift)
{
	uint16_t result = (pattern << shift);
	return result | (result >> 8);
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
	// Start TWI interface
	//====================
	i2c::I2CManager<I2C_MODE> manager;
	manager.begin();

	// Initialize chip
	//=================
	time::delay_ms(100);
	MCP mcp{manager, 0x00};
	mcp.configure_portA_gpio(0x00);
	// mcp.configure_gpio(SWITCH_MASK, SWITCH_MASK);

	mcp.portA_values(0x11);
	time::delay_ms(1000);
	mcp.portA_values(0x00);
	// Loop of the LED chaser
	while (true)
	{
		for (uint8_t i = 0; i < 8; ++i)
		{
			mcp.portA_values(shift_pattern(0x01, i));
			time::delay_ms(250);
		}
	}
	
	// Stop TWI interface
	//===================
	manager.end();
}
