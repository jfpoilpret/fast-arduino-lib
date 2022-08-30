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
 * Configurable LED chaser example, using MCP23008 I2C device (GPIO expander).
 * This program uses FastArduino MCP23008 support API, including MCP23008 interrupts
 * to be notified when an input switch changes states.
 * 
 * Wiring:
 * - MCP23008:
 *   - GP4-GP7: each pin is connected to LED through a ~1K resistor to the ground
 *   - GP0-GP3: each pin shall be connected to a DIP switch, itself connected to the ground. 
 *     3 first switches define a "LED pattern" that will progress through the 8 LEDs chain
 *     last switch defines the progress direction of the pattern
 * 
 * NB: you should add pullup resistors (10K-22K typically) on both SDA and SCL lines.
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to MCP23008 SDA pin
 *   - A5 (PC5, SCL): connected to MCP23008 SCL pin
 *   - D2 (PD2): connected to MCP23008 INT pin
 * - on ATtinyX4 based boards:
 *   - D6 (PA6, SDA): connected to MPU6050 SDA pin
 *   - D4 (PA4, SCL): connected to MPU6050 SDA pin
 *   - D10 (PB2): connected to MCP23008 INT pin
 */

#include <fastarduino/int.h>
#include <fastarduino/time.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/devices/mcp23008.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_MEGA)
#define INT_NUM 0
static constexpr const board::ExternalInterruptPin INT_PIN = board::ExternalInterruptPin::D2_PD2_EXT0;
#elif defined(BREADBOARD_ATTINYX4)
#define INT_NUM 0
static constexpr const board::ExternalInterruptPin INT_PIN = board::ExternalInterruptPin::D10_PB2_EXT0;
#else
#error "Current target is not yet supported!"
#endif

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

class LedChaser
{
public:
	LedChaser()
		: mcp_{manager_, 0x00}, signal_{interrupt::InterruptTrigger::RISING_EDGE}
	{
		interrupt::register_handler(*this);
		manager_.begin();
		time::delay_ms(100);
		mcp_.begin();
		mcp_.configure_gpio(0x0F, 0x0F);
		mcp_.configure_interrupts(0x0F, 0x00, 0x00);
		on_change();
		signal_.enable();
	}

	void loop()
	{
		while (true)
		{
			bool direction;
			uint8_t pattern;
			for (uint8_t i = 0; i < 4; ++i)
			{
				if (changed_)
				{
					changed_ = false;
					uint8_t switches = mcp_.values() & 0x0F;
					direction = (~switches) & 0x08;
					pattern = calculate_pattern(switches);
				}
				mcp_.values(shift_pattern(pattern, i, direction) << 4);
				time::delay_ms(250);
			}
		}
	}

private:
	static inline uint8_t shift_pattern(uint8_t pattern, uint8_t shift, bool direction)
	{
		if (direction) shift = 4 - shift;
		uint16_t result = (pattern << shift);
		return result | (result >> 4);
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
			return 0x05;

			case 0x05:
			case 0x06:
			case 0x07:
			return 0x09;
		}
	}

	void on_change()
	{
		changed_ = true;
	}

	using MCP = devices::mcp230xx::MCP23008<MANAGER>;

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
	MANAGER manager_{i2c_buffer};
#else
	MANAGER manager_;
#endif
	MCP mcp_;
	interrupt::INTSignal<INT_PIN> signal_;
	volatile bool changed_ = true;

	DECL_INT_ISR_HANDLERS_FRIEND
};

REGISTER_INT_ISR_METHOD(INT_NUM, INT_PIN, LedChaser, &LedChaser::on_change)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	LedChaser chaser;
	chaser.loop();
}
