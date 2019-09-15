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
 * Analog Comparator example.
 * This program shows usage of FastArduino AnalogComparator API.
 * It compares AIN0 and A0 values through an ISR, toggling a LED everytime comparison bit rises.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D6 (AIN0): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D13 (LED): internal Arduino LED
 * TODO other targets
 * - on Arduino LEONARDO:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *	 - D3-D2-D0-D1-D4-TXLED-D12-D6 (port D) branch 8 LED (except for TXLED) in series with 330 Ohm resistors
 * - on Arduino MEGA:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D22-D29 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on ATtinyX4 based boards:
 *   - A7: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D0-D7 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 */

#include <fastarduino/power.h>
#include <fastarduino/analog_comparator.h>
#include <fastarduino/gpio.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A0;
#elif defined(ARDUINO_MEGA)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A0;
#elif defined(ARDUINO_LEONARDO)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A0;
#elif defined(BREADBOARD_ATTINYX4)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A0;
#elif defined(BREADBOARD_ATTINYX5)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A1;
#else
#error "Current target is not yet supported!"
#endif

class Compare
{
public:
	Compare() : comparator_{}, led_{gpio::PinMode::OUTPUT}
	{
		interrupt::register_handler(*this);
		comparator_.begin<INPUT>(analog::ComparatorInterrupt::RISING_EDGE);
	}

	void callback()
	{
		led_.toggle();
	}

private:
	analog::AnalogComparator comparator_;
	gpio::FastPinType<board::DigitalPin::LED>::TYPE led_;
};

REGISTER_ANALOG_COMPARE_ISR_METHOD(Compare, &Compare::callback)

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Declare Analog comparator
	Compare compare;

	// Infinite loop sleeping idle
	while (true)
	{
		power::Power::sleep();
	}
	return 0;
}
