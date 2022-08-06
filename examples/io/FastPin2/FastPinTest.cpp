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
 * This program is the Hello World of Arduino: blink LED on D13.
 * It checks FastArduino FastPin support.
 * So far this is just an active loop with a busy loop wait delay.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino LEONARDO:
 *   - D13 (PC7) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D7 (PA7) LED connected to ground through a resistor
 * - on ATtinyX5 based boards:
 *   - D0 (PB0) LED connected to ground through a resistor
 * - on ATmega644 based boards:
 *   - D8 (PB0) LED connected to ground through a resistor
 */

#include <fastarduino/gpio.h>
#include <fastarduino/time.h>

int main()
{
	board::init();
	sei();
	gpio::FAST_PIN<board::DigitalPin::LED> PinLED{gpio::PinMode::OUTPUT};
	while (true)
	{
		PinLED.toggle();
		time::delay_ms(500);
	}
	return 0;
}
