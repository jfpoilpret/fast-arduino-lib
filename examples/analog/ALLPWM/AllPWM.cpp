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
 * Check all PWM pins on the built target.
 * For each PWM pin, a LED is lit from 0% to 100%, then switched off.
 * Wiring:
 * - connect a LED to each PWM pin (detailed later) and a 1K resistor to GND
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D3, D5, D6, D9, D10, D11
 * - on Arduino LENOARDO:
 *   - D3, D5, D9, D10, D11 (2 timers)
 * - on Arduino MEGA:
 *   - D2-D12, D13 (2 timers), D44-46
 * - on ATtinyX4 based boards:
 *   - PA5-7, PB2
 * - on ATtinyX5 based boards:
 *   - PB0, PB1
 * - on ATmega644 based boards:
 *   - D11 (PB3), D12 (PB4), D28 (PD4), D29 (PD5), D30 (PD6), D31 (PD7)
 * - on ATmega1284 based boards:
 *   - D11 (PB3), D12 (PB4), D28 (PD4), D29 (PD5), D30 (PD6), D31 (PD7), D14 (PB6), D15 (PB7)
 */

#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/pwm.h>

using namespace board;

// Frequency for PWM
constexpr const uint16_t PWM_FREQUENCY = 450;

template<Timer NTIMER, PWMPin PIN> void check_PWM()
{
	using TIMER = timer::Timer<NTIMER>;
	using CALC = timer::Calculator<NTIMER>;
	using LED_OUTPUT = analog::PWMOutput<PIN>;
	constexpr const typename TIMER::PRESCALER PRESCALER = CALC::FastPWM_prescaler(PWM_FREQUENCY);

	TIMER timer{timer::TimerMode::FAST_PWM, PRESCALER};
	LED_OUTPUT led{timer};
	// Start timer
	timer.begin_();
	// Loop of samplings
	using TYPE = typename LED_OUTPUT::TYPE;
	constexpr const TYPE MAX = LED_OUTPUT::MAX;
	for (uint16_t duty = 0; duty < MAX; duty += MAX / 10)
	{
		led.set_duty(TYPE(duty));
		time::delay_ms(1000);
	}
	led.set_duty(0);
	timer.end_();
	time::delay_ms(2000);
}

int main() __attribute__((OS_main));
int main()
{
	init();

	// Enable interrupts
	sei();

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
	check_PWM<Timer::TIMER0, PWMPin::D6_PD6_OC0A>();
	check_PWM<Timer::TIMER0, PWMPin::D5_PD5_OC0B>();
	check_PWM<Timer::TIMER1, PWMPin::D9_PB1_OC1A>();
	check_PWM<Timer::TIMER1, PWMPin::D10_PB2_OC1B>();
	check_PWM<Timer::TIMER2, PWMPin::D11_PB3_OC2A>();
	check_PWM<Timer::TIMER2, PWMPin::D3_PD3_OC2B>();
#elif defined (ARDUINO_LEONARDO)
	check_PWM<Timer::TIMER0, PWMPin::D11_PB7_OC0A>();
	check_PWM<Timer::TIMER0, PWMPin::D3_PD0_OC0B>();
	check_PWM<Timer::TIMER1, PWMPin::D9_PB5_OC1A>();
	check_PWM<Timer::TIMER1, PWMPin::D10_PB6_OC1B>();
	check_PWM<Timer::TIMER1, PWMPin::D11_PB7_OC1C>();
	check_PWM<Timer::TIMER3, PWMPin::D5_PC6_OC3A>();
#elif defined (ARDUINO_MEGA)
	check_PWM<Timer::TIMER0, PWMPin::D13_PB7_OC0A>();
	check_PWM<Timer::TIMER0, PWMPin::D4_PG5_OC0B>();
	check_PWM<Timer::TIMER1, PWMPin::D11_PB5_OC1A>();
	check_PWM<Timer::TIMER1, PWMPin::D12_PB6_OC1B>();
	check_PWM<Timer::TIMER1, PWMPin::D13_PB7_OC1C>();
	check_PWM<Timer::TIMER2, PWMPin::D10_PB4_OC2A>();
	check_PWM<Timer::TIMER2, PWMPin::D9_PH6_OC2B>();
	check_PWM<Timer::TIMER3, PWMPin::D5_PE3_OC3A>();
	check_PWM<Timer::TIMER3, PWMPin::D2_PE4_OC3B>();
	check_PWM<Timer::TIMER3, PWMPin::D3_PE5_OC3C>();
	check_PWM<Timer::TIMER4, PWMPin::D6_PH3_OC4A>();
	check_PWM<Timer::TIMER4, PWMPin::D7_PH4_OC4B>();
	check_PWM<Timer::TIMER4, PWMPin::D8_PH5_OC4C>();
	check_PWM<Timer::TIMER5, PWMPin::D46_PL3_OC5A>();
	check_PWM<Timer::TIMER5, PWMPin::D45_PL4_OC5B>();
	check_PWM<Timer::TIMER5, PWMPin::D44_PL5_OC5C>();
#elif defined (BREADBOARD_ATTINYX4)
	check_PWM<Timer::TIMER0, PWMPin::D10_PB2_OC0A>();
	check_PWM<Timer::TIMER0, PWMPin::D7_PA7_OC0B>();
	check_PWM<Timer::TIMER1, PWMPin::D6_PA6_OC1A>();
	check_PWM<Timer::TIMER1, PWMPin::D5_PA5_OC1B>();
#elif defined (BREADBOARD_ATTINYX5)
	check_PWM<Timer::TIMER0, PWMPin::D0_PB0_OC0A>();
	check_PWM<Timer::TIMER0, PWMPin::D1_PB1_OC0B>();
#elif defined (BREADBOARD_ATMEGAXX4P)
	check_PWM<Timer::TIMER0, PWMPin::D11_PB3_OC0A>();
	check_PWM<Timer::TIMER0, PWMPin::D12_PB4_OC0B>();
	check_PWM<Timer::TIMER1, PWMPin::D29_PD5_OC1A>();
	check_PWM<Timer::TIMER1, PWMPin::D28_PD4_OC1B>();
	check_PWM<Timer::TIMER2, PWMPin::D31_PD7_OC2A>();
	check_PWM<Timer::TIMER2, PWMPin::D30_PD6_OC2B>();
#ifdef __AVR_ATmega1284P__
	check_PWM<Timer::TIMER3, PWMPin::D14_PB6_OC3A>();
	check_PWM<Timer::TIMER3, PWMPin::D15_PB7_OC3B>();
#endif
#else
#error "Current target is not yet supported!"
#endif
	return 0;
}
