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

constexpr const i2c::I2CMode I2C_MODE = i2c::I2CMode::Fast;

using MCP = devices::MCP23017<I2C_MODE>;
using MCP_PORT = devices::MCP23017Port;

static inline uint8_t shift_pattern(uint8_t pattern, uint8_t shift, bool direction)
{
	if (direction) shift = 8 - shift;
	uint16_t result = (pattern << shift);
	return result | (result >> 8);
}

static inline uint8_t calculate_pattern(uint8_t switches)
{
	switch ((~switches) & 0x07)
	{
		case 0x00:
		return 0x01;

		case 0x01:
		return 0x03;

		case 0x02:
		return 0x07;

		case 0x03:
		return 0x0F;

		case 0x04:
		return 0x55;

		case 0x05:
		return 0x33;

		case 0x06:
		return 0x11;

		case 0x07:
		return 0xDB;
	}
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
	mcp.begin();
	mcp.configure_gpio<MCP_PORT::PORT_A>(0x00);
	mcp.configure_gpio<MCP_PORT::PORT_B>(0x0F, 0x0F);

	mcp.values<MCP_PORT::PORT_A>(0x11);
	time::delay_ms(1000);
	mcp.values<MCP_PORT::PORT_A>(0x00);
	// Loop of the LED chaser
	uint8_t switches = mcp.values<MCP_PORT::PORT_B>() & 0x0F;
	bool direction = (~switches) & 0x08;
	uint8_t pattern = calculate_pattern(switches);
	while (true)
	{
		uint8_t new_switches = mcp.values<MCP_PORT::PORT_B>() & 0x0F;
		if (switches != new_switches)
		{
			switches = new_switches;
			direction = (~switches) & 0x08;
			pattern = calculate_pattern(switches);
		}
		for (uint8_t i = 0; i < 8; ++i)
		{
			mcp.values<MCP_PORT::PORT_A>(shift_pattern(pattern, i, direction));
			time::delay_ms(250);
		}
	}
	
	// Stop TWI interface
	//===================
	manager.end();
}
