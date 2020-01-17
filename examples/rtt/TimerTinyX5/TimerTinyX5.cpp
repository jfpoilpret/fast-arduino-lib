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
 * Timer compilation example specific for ATtinyX5.
 * This checks FastArduino implementation works with specificities of ATtinyX5
 * Timer1.
 * LED0 toggles at every COMPARE MATCH A
 * LED1 toggles at every COMPARE MATCH B
 * LED2 toggles at every OVERFLOW
 * Actual pulses should be measured and checked with an oscilloscope (or a digital 
 * analyzer).
 * 
 * Wiring:
 *   - D0 (PB0) LED0 connected to ground through a resistor
 *   - D1 (PB1) LED1 connected to ground through a resistor
 *   - D2 (PB2) LED2 connected to ground through a resistor
 */

#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>
#include <fastarduino/time.h>

#if !defined(BREADBOARD_ATTINYX5)
#error "Current target is not yet supported!"
#endif

using LED_COMPA = gpio::FastPinType<board::DigitalPin::D0_PB0>;
using LED_COMPB = gpio::FastPinType<board::DigitalPin::D1_PB1>;
using LED_OVF = gpio::FastPinType<board::DigitalPin::D2_PB2>;

constexpr const board::Timer NTIMER = board::Timer::TIMER1;
using CALC = timer::Calculator<NTIMER>;
using TIMER = timer::Timer<NTIMER>;
constexpr const uint32_t PERIOD_US = 100000;

constexpr const TIMER::PRESCALER PRESCALER = CALC::CTC_prescaler(PERIOD_US);
static_assert(CALC::is_adequate_for_CTC(PRESCALER, PERIOD_US), 
		"CALC::is_adequate(PRESCALER, PERIOD_US)");
constexpr const TIMER::TYPE COUNTER = CALC::CTC_counter(PRESCALER, PERIOD_US);

// Define vectors we need in the example
ISR(TIMER1_COMPA_vect)
{
	LED_COMPA::toggle();
}
ISR(TIMER1_COMPB_vect)
{
	LED_COMPB::toggle();
}
ISR(TIMER1_OVF_vect)
{
	LED_OVF::toggle();
}

using timer::TimerInterrupt;
using timer::TimerMode;

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Set all 3 LEDs pins as output
	LED_COMPA::set_mode(gpio::PinMode::OUTPUT);
	LED_COMPB::set_mode(gpio::PinMode::OUTPUT);
	LED_OVF::set_mode(gpio::PinMode::OUTPUT);
	sei();

	// Wait long enough in order to start a digital analyzer
	time::delay_ms(10000);

	// First off, try timer in CTC mode
	TIMER timer{
		TimerMode::CTC, PRESCALER, 
		TimerInterrupt::OUTPUT_COMPARE_A | TimerInterrupt::OUTPUT_COMPARE_B | TimerInterrupt::OVERFLOW};
	timer.begin(COUNTER);
	time::delay_ms(2000);
	timer.end();

	LED_COMPA::clear();
	LED_COMPB::clear();
	LED_OVF::clear();
	time::delay_ms(1000);

	// Then try timer in Normal mode
	timer.set_timer_mode(TimerMode::NORMAL);
	timer.begin(COUNTER);
	time::delay_ms(2000);
	timer.end();

	LED_COMPA::clear();
	LED_COMPB::clear();
	LED_OVF::clear();
	time::delay_ms(1000);

	// Then try PWM
	timer.set_timer_mode(TimerMode::FAST_PWM);
	timer.begin(COUNTER);
	time::delay_ms(2000);
	timer.end();

	// Switch off all LEDs
	LED_COMPA::clear();
	LED_COMPB::clear();
	LED_OVF::clear();
}
