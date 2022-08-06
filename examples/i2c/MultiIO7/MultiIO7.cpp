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
 * LED chaser example, using MCP23008 I2C device (GPIO expander).
 * This program uses FastArduino MCP23008 support API.
 * 
 * Wiring:
 * - MCP23008:
 *   - GP4-GP7: each pin is connected to LED through a ~1K resistor to the ground
 * 
 * NB: you should add pullup resistors (10K-22K typically) on both SDA and SCL lines.
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to MCP23008 SDA pin
 *   - A5 (PC5, SCL): connected to MCP23008 SCL pin
 */

#include <fastarduino/time.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/devices/mcp23008.h>

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

class LedChaser
{
public:
	LedChaser()
		: mcp_{manager_, 0x00}
	{
		manager_.begin();
		time::delay_ms(100);
		mcp_.begin();
		mcp_.configure_gpio(0x0F, 0x0F);
	}

	void loop()
	{
		while (true)
		{
			for (uint8_t i = 0; i < 4; ++i)
			{
				mcp_.values(shift_pattern(PATTERN, i) << 4);
				time::delay_ms(250);
			}
		}
	}

private:
	static constexpr uint8_t PATTERN = 0x01;

	static inline uint8_t shift_pattern(uint8_t pattern, uint8_t shift)
	{
		uint16_t result = (pattern << shift);
		return result | (result >> 4);
	}

	using MCP = devices::mcp230xx::MCP23008<MANAGER>;

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
	MANAGER manager_{i2c_buffer};
#else
	MANAGER manager_;
#endif
	MCP mcp_;
};

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	LedChaser chaser;
	chaser.loop();
}
