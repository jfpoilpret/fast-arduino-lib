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
 * This program is the Hello World of Arduino: blink LEDs in sequence.
 * It checks FastArduino FastPort support.
 * So far this is just an active loop with a busy loop wait delay.
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
 * - on ATmega644 based boards:
 *   - D16-D23 (port C) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 */

#include <fastarduino/gpio.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO) || defined(ARDUINO_LEONARDO)
static constexpr const board::Port LED_PORT = board::Port::PORT_D;
#elif defined (ARDUINO_MEGA)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
#elif defined (BREADBOARD_ATMEGAXX4P)
static constexpr const board::Port LED_PORT = board::Port::PORT_C;
#else
#error "Current target is not yet supported!"
#endif

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Set Port D direction to all outputs
	gpio::FastPort<LED_PORT> ledPort{0xFF, 0x00};
	uint8_t value = 0;
	// Loop of the LED chaser
	while (true)
	{
		ledPort.set_PORT(value);
		time::delay_ms(250);
		value <<= 1;
		if (!value) value = 0x01;
	}
	return 0;

}
