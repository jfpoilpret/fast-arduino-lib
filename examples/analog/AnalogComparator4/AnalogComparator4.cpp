//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * It compares Bandgap voltage (~1.1V) and A0 values through an ISR, toggling a LED everytime comparison bit changes.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D13 (LED): internal Arduino LED
 * - on Arduino LEONARDO:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D13 (LED): internal Arduino LED
 * - on Arduino MEGA:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D13 (LED): internal Arduino LED
 * - on ATtinyX4 based boards:
 *   - A0 (PA0): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D7 (PA7, LED): LED in series with 330 Ohm resistor, connected to ground
 * - on ATtinyX5 based boards:
 *   - A1 (PB2): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D4 (PB4): LED in series with 330 Ohm resistor, connected to ground
 * - on ATmega644 based boards:
 *   - A0 (PA0): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D8 (PB0): LED in series with 330 Ohm resistor, connected to ground
 */

#include <fastarduino/power.h>
#include <fastarduino/analog_comparator.h>
#include <fastarduino/gpio.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED = board::DigitalPin::LED;
#elif defined(ARDUINO_MEGA)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED = board::DigitalPin::LED;
#elif defined(ARDUINO_LEONARDO)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED = board::DigitalPin::LED;
#elif defined(BREADBOARD_ATTINYX4)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED = board::DigitalPin::LED;
#elif defined(BREADBOARD_ATTINYX5)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED = board::DigitalPin::D4_PB4;
#elif defined (BREADBOARD_ATMEGAXX4P)
static constexpr const board::AnalogPin INPUT = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED = board::DigitalPin::LED;
#else
#error "Current target is not yet supported!"
#endif

class Compare
{
public:
	Compare() : comparator_{}, led_{gpio::PinMode::OUTPUT}
	{
		interrupt::register_handler(*this);
		comparator_.begin<INPUT, true>(analog::ComparatorInterrupt::TOGGLE);
	}

	void callback()
	{
		if (comparator_.output()) led_.set(); else led_.clear();
	}

private:
	analog::AnalogComparator comparator_;
	gpio::FAST_PIN<LED> led_;
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
