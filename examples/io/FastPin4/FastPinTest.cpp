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

/*
 * FastIO LED chaser.
 * This program shows usage of FastArduino FastIO API with both outputs and inputs.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D0-D7 (port D) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 *   - D14-D17 (A0-A3, port C) branch 4 switches connected to ground
 * - on Arduino MEGA:
 *   - D22-D29 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 *   - D21-D18 (port D) branch 4 switches connected to ground
 * - on ATtinyX4 based boards:
 *   - D0-D7 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 *   - D8-D10 (port B) branch 3 switches connected to ground
 */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <fastarduino/fast_io.h>

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

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const board::Port LED_PORT = board::Port::PORT_D;
static constexpr const board::Port SWITCH_PORT = board::Port::PORT_C;
#elif defined (ARDUINO_MEGA)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
static constexpr const board::Port SWITCH_PORT = board::Port::PORT_D;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
static constexpr const board::Port SWITCH_PORT = board::Port::PORT_B;
#else
#error "Current target is not yet supported!"
#endif

int main()
{
	// Enable interrupts at startup time
	sei();
	// Prepare ports to read settings and write to LEDs
	FastPort<SWITCH_PORT> switchPort{0x00, 0x0F};
	FastPort<LED_PORT> ledPort{0xFF};
	
	// Loop of the LED chaser
	while (true)
	{
		// Read settings everytime a LED chasing loop is about to start
		uint8_t settings = switchPort.get_PIN();
		uint8_t pattern = calculate_pattern(settings & 0x07);
		bool direction = settings & 0x08;
		for (uint8_t i = 0; i < 8; ++i)
		{
			ledPort.set_PORT(shift_pattern(pattern, (direction ? i : 7 - i)));
			_delay_ms(250.0);
		}
	}
	return 0;
}
