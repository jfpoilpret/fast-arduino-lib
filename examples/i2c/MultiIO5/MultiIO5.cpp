//   Copyright 2016-2020 Jean-Francois Poilpret
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
 */

#include <fastarduino/int.h>
#include <fastarduino/time.h>
#include <fastarduino/new_i2c_device.h>
#include <fastarduino/devices/new_mcp23008.h>

static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static constexpr uint8_t MAX_FUTURES = 128;
static i2c::I2CCommand i2c_buffer[I2C_BUFFER_SIZE];
REGISTER_I2C_ISR(i2c::I2CMode::FAST)

#define INT_NUM 0
static constexpr const board::ExternalInterruptPin INT_PIN = board::ExternalInterruptPin::D2_PD2_EXT0;

class LedChaser
{
public:
	LedChaser()
		:	manager_{i2c_buffer, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS}, 
			mcp_{manager_, 0x00}, signal_{interrupt::InterruptTrigger::RISING_EDGE}
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
			for (uint8_t i = 0; i < 4; ++i)
			{
				mcp_.values(shift_pattern(pattern_, i, direction_) << 4);
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
		uint8_t switches = mcp_.values() & 0x0F;
		direction_ = (~switches) & 0x08;
		pattern_ = calculate_pattern(switches);
	}

	static constexpr const i2c::I2CMode I2C_MODE = i2c::I2CMode::FAST;
	using MCP = devices::mcp230xx::MCP23008<I2C_MODE>;

	i2c::I2CManager<I2C_MODE> manager_;
	MCP mcp_;
	interrupt::INTSignal<INT_PIN> signal_;

	bool direction_;
	uint8_t pattern_;

	DECL_INT_ISR_HANDLERS_FRIEND
};

REGISTER_INT_ISR_METHOD(INT_NUM, INT_PIN, LedChaser, &LedChaser::on_change)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	// Initialize FutureManager
	future::FutureManager<MAX_FUTURES> future_manager;

	LedChaser chaser;
	chaser.loop();
}
