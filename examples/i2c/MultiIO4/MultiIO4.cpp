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

/*
 * Configurable LED chaser example, using MCP23017 I2C device (GPIO expander).
 * This program uses FastArduino MCP23017 support API, by addressing each MCP23017
 * port individually. It also uses MCP23017 interrupts to be notified when an
 * input switch changes states.
 * This example is similar to MultiIO3, except it better encapsulates LED chaser
 * functionality to a class.
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

#define INT_NUM 0
static constexpr const board::ExternalInterruptPin INT_PIN = board::ExternalInterruptPin::D2_PD2_EXT0;

class LedChaser
{
public:
	LedChaser() : manager_{}, mcp_{manager_, 0x00}, signal_{interrupt::InterruptTrigger::RISING_EDGE}
	{
		interrupt::register_handler(*this);
		manager_.begin();
		time::delay_ms(100);
		mcp_.begin();
		mcp_.configure_gpio<MCP_PORT::PORT_A>(0x00);
		mcp_.configure_gpio<MCP_PORT::PORT_B>(0x0F, 0x0F);
		mcp_.configure_interrupts<MCP_PORT::PORT_B>(0x0F, 0x00, 0x00);
		on_change();
		signal_.enable();
	}

	void loop()
	{
		while (true)
		{
			for (uint8_t i = 0; i < 8; ++i)
			{
				mcp_.values<MCP_PORT::PORT_A>(shift_pattern(pattern_, i, direction_));
				time::delay_ms(250);
			}
		}
	}

private:
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

	void on_change()
	{
		uint8_t switches = mcp_.values<MCP_PORT::PORT_B>() & 0x0F;
		direction_ = (~switches) & 0x08;
		pattern_ = calculate_pattern(switches);
	}

	static constexpr const i2c::I2CMode I2C_MODE = i2c::I2CMode::Fast;
	using MCP = devices::mcp23017::MCP23017<I2C_MODE>;
	using MCP_PORT = devices::mcp23017::MCP23017Port;

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
	LedChaser chaser;
	chaser.loop();
}
