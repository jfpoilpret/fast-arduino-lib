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
 * Potentiometer value reading example.
 * This program shows usage of FastArduino AnalogInput API.
 * It reads and converts the analog level on a pin and displays it as a level (in binary) in a range of 8 LEDs.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D0-D7 (port D) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on Arduino MEGA:
 *   - TODO
 *   - D22-D29 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on ATtinyX4 based boards:
 *   - TODO
 *   - D0-D7 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 */

#include <fastarduino/time.h>
#include <fastarduino/analog_input.h>
#include <fastarduino/fast_io.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::Port LED_PORT = board::Port::PORT_D;
static constexpr const uint8_t LED_MASK = 0xFF;
#elif defined (ARDUINO_MEGA)
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
static constexpr const uint8_t LED_MASK = 0xFF;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::AnalogPin POT = board::AnalogPin::A7;
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
static constexpr const uint8_t LED_MASK = 0x7F;
#else
#error "Current target is not yet supported!"
#endif

using ANALOG_INPUT = analog::AnalogInput<POT, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;

int main()
{
	// Enable interrupts at startup time
	sei();
	
	gpio::FastMaskedPort<LED_PORT> leds{LED_MASK, 0xFF};
	// Declare Analog input
	ANALOG_INPUT pot;

	// Loop of samplings
	while (true)
	{
		ANALOG_INPUT::TYPE value = pot.sample();
		leds.set_PORT(value);
		time::delay_ms(1000);
	}
	return 0;
}
