//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * Configurable LED chaser example, using MCP23017 I2C device (GPIO expander).
 * This program uses FastArduino MCP23017 support API, by addressing each MCP23017
 * port individually. It also uses MCP23017 interrupts to be notified when an
 * input switch changes states.
 * 
 * Wiring:
 * - MCP23017:
 *   - GPA0-GPA7: each pin is connected to LED through a ~1K resistor to the ground
 *   - GPB0-GPB3: each pin shall be connected to a DIP switch, itself connected to the ground. 
 *     3 first switches define a "LED pattern" that will progress through the 8 LEDs chain
 *     last switch defines the progress direction of the pattern
 * 
 * NB: you should add pullup resistors (10K-22K typically) on both SDA and SCL lines.
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to MCP23017 SDA pin
 *   - A5 (PC5, SCL): connected to MCP23017 SCL pin
 *   - D2 (PD2): connected to MCP23017 INTB pin
 */

#include <fastarduino/int.h>
#include <fastarduino/time.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/devices/mcp23017.h>

#define FORCE_SYNC

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncManager<
	i2c::I2CMode::FAST, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
REGISTER_I2C_ISR(MANAGER)
#else
using MANAGER = i2c::I2CSyncManager<i2c::I2CMode::FAST>;
#endif
REGISTER_FUTURE_NO_LISTENERS()

constexpr const board::ExternalInterruptPin INT_PIN = board::ExternalInterruptPin::D2_PD2_EXT0;

using MCP = devices::mcp230xx::MCP23017<MANAGER>;
using MCP_PORT = devices::mcp230xx::MCP23017Port;

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

bool direction;
uint8_t pattern;
MCP* pmcp;

void mcp_on_change()
{
	uint8_t switches = pmcp->values<MCP_PORT::PORT_B>() & 0x0F;
	direction = (~switches) & 0x08;
	pattern = calculate_pattern(switches);
}

REGISTER_INT_ISR_FUNCTION(0, INT_PIN, mcp_on_change)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
	// Start TWI interface
	//====================
#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
	MANAGER manager{i2c_buffer};
#else
	MANAGER manager;
#endif
	manager.begin();

	// Initialize chip
	//=================
	time::delay_ms(100);
	MCP mcp{manager, 0x00};
	mcp.begin();
	pmcp = &mcp;
	mcp.configure_gpio<MCP_PORT::PORT_A>(0x00);
	mcp.configure_gpio<MCP_PORT::PORT_B>(0x0F, 0x0F);
	mcp.configure_interrupts<MCP_PORT::PORT_B>(0x0F, 0x00, 0x00);

	// Initialize UNO interrupts
	interrupt::INTSignal<INT_PIN> int_signal{interrupt::InterruptTrigger::RISING_EDGE};
	int_signal.enable();

	// Loop of the LED chaser
	mcp_on_change();
	while (true)
	{
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
