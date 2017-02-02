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
 * This program is the Hello World of Arduino: blink LED on D13.
 * It checks FastArduino FastPin support.
 * So far this is just an active loop with a active wait delay.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D7 (PA7) LED connected to ground through a resistor
 */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <fastarduino/fast_io.h>

int main()
{
	sei();
	FastPinType<Board::DigitalPin::LED>::TYPE PinLED{PinMode::OUTPUT};
	while (true)
	{
		PinLED.toggle();
		_delay_ms(400.0);
	}
	return 0;
}
