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
 * Analog Comparator example.
 * This program shows usage of FastArduino AnalogComparator API.
 * It compares AIN0 and AIN1 values through an ISR, toggling a LED.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D6 (AIN0): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D7 (AIN1): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D13 (LED): internal Arduino LED
 * - on ATtinyX4 based boards:
 *   - D1 (PA1, AIN0): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D2 (PA2, AIN1): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D7 (PA7, LED): LED in series with 330 Ohm resistor, connected to ground
 * - on ATtinyX5 based boards:
 *   - D0 (PB0, AIN0): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D1 (PB1, AIN1): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D4 (PB4): LED in series with 330 Ohm resistor, connected to ground
 * - on ATmega644 based boards:
 *   - D10 (PB2, AIN0): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D11 (PB3, AIN1): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D8 (PB0): LED in series with 330 Ohm resistor, connected to ground
 */

#include <fastarduino/power.h>
#include <fastarduino/analog_comparator.h>
#include <fastarduino/gpio.h>

#if defined(BREADBOARD_ATTINYX5)
static constexpr const board::DigitalPin LED = board::DigitalPin::D4_PB4;
#else
static constexpr const board::DigitalPin LED = board::DigitalPin::LED;
#endif

void toggle_led()
{
	gpio::FastPinType<LED>::toggle();
}

REGISTER_ANALOG_COMPARE_ISR_FUNCTION(toggle_led)

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	gpio::FastPinType<LED>::set_mode(gpio::PinMode::OUTPUT);

	// Declare Analog comparator
	analog::AnalogComparator comparator;
	comparator.begin<>(analog::ComparatorInterrupt::TOGGLE);

	// Infinite loop sleeping idle
	while (true)
	{
		power::Power::sleep();
	}
	return 0;
}
