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

using MCP = devices::MCP23017<i2c::I2CMode::Fast>;
// // 8 LEDs connected to port A
// static constexpr const uint16_t LED_MASK = 0x00FF;
// // 4 Switches connected to port B
// static constexpr const uint16_t SPEED_MASK = 0x0700;
// static constexpr const uint8_t SPEED_SHIFT = 8;
// static constexpr const uint16_t DIRECTION_MASK = 0x0800;
// static constexpr const uint16_t SWITCH_MASK = SPEED_MASK | DIRECTION_MASK;

// 8 LEDs connected to port A
static constexpr const uint16_t LED_MASK = 0xFF00;
static constexpr const uint8_t LED_SHIFT = 8;
// 4 Switches connected to port B
static constexpr const uint16_t SPEED_MASK = 0x0007;
static constexpr const uint8_t SPEED_SHIFT = 0;
static constexpr const uint16_t DIRECTION_MASK = 0x0008;
static constexpr const uint16_t SWITCH_MASK = SPEED_MASK | DIRECTION_MASK;

static inline uint8_t shift_pattern(uint8_t pattern, uint8_t shift)
{
	uint16_t result = (pattern << shift);
	return result | (result >> 8);
}

static inline uint8_t calculate_pattern(uint8_t num_bits)
{
	uint16_t pattern = (1 << (num_bits + 1)) - 1;
	return pattern;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
	// Start TWI interface
	//====================
	i2c::I2CManager<i2c::I2CMode::Fast> manager;
	manager.begin();

	// Initialize chip
	//=================
	time::delay_ms(100);
	MCP mcp{manager, 0x00};
	mcp.configure_gpio(SWITCH_MASK, SWITCH_MASK);
	
	// Loop of the LED chaser
	while (true)
	{
		// Read settings everytime a LED chasing loop is about to start
		uint8_t settings = mcp.values();
		uint8_t pattern = calculate_pattern((settings & SPEED_MASK) >> SPEED_SHIFT);
		bool direction = settings & DIRECTION_MASK;
		for (uint8_t i = 0; i < 8; ++i)
		{
			mcp.values(shift_pattern(pattern, (direction ? i : 7 - i)) << LED_SHIFT);
			time::delay_ms(250);
		}
	}
	
	// Stop TWI interface
	//===================
	manager.end();
}
