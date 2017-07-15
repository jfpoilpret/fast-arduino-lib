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
 * This program shows usage of FastArduino FastIO API with masked ports.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D0-D7 (port D) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on Arduino LEONARDO:
 *	- D3-D2-D0-D1-D4-TXLED-D12-D6 (port D) branch 8 LED (except for TXLED) in series with 330 Ohm resistors
 * - on Arduino MEGA:
 *   - D22-D29 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on ATtinyX4 based boards:
 *   - D0-D7 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 */

#include <fastarduino/gpio.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO) || defined(ARDUINO_LEONARDO)
static constexpr const board::Port LED_PORT = board::Port::PORT_D;
#elif defined (ARDUINO_MEGA)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
#else
#error "Current target is not yet supported!"
#endif

static const uint8_t NUM_LEDS = 8;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	// Prepare ports to write to LEDs
	gpio::FastMaskedPort<LED_PORT> pins[NUM_LEDS];
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
		pins[i] = gpio::FastMaskedPort<LED_PORT>{_BV(i), 0xFF};
	
	// Loop of the LED chaser
	while (true)
	{
		for (uint8_t i = 0; i < 8; ++i)
		{
			pins[i].set_PORT(0xFF);
			time::delay_ms(250);
			pins[i].set_PORT(0x00);
			time::delay_ms(250);
		}
	}
	return 0;
}
