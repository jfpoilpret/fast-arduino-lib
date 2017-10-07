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
 * Use potentiometer to set LED light level through PWM.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D6: LED connected to GND through a 1K resistor 
 * - on Arduino MEGA:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D4: LED connected to GND through a 1K resistor 
 * - on ATtinyX4 based boards:
 *   - A0 (PA0): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D10 (PB2): LED connected to GND through a 1K resistor 
 */

#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/analog_input.h>
#include <fastarduino/pwm.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED = board::PWMPin::D6_PD6_OC0A;
static constexpr const board::Timer TIMER = board::Timer::TIMER0;
#elif defined (ARDUINO_LEONARDO)
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED = board::PWMPin::D11_PB7_OC0A;
static constexpr const board::Timer TIMER = board::Timer::TIMER0;
#elif defined (ARDUINO_MEGA)
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED = board::PWMPin::D4_PG5_OC0B;
static constexpr const board::Timer TIMER = board::Timer::TIMER0;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED = board::PWMPin::D10_PB2_OC0A;
static constexpr const board::Timer TIMER = board::Timer::TIMER0;
#else
#error "Current target is not yet supported!"
#endif

using ANALOG_INPUT = analog::AnalogInput<POT, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;
using LED_OUTPUT = analog::PWMOutput<LED>;
using TIMER_TYPE = timer::Timer<TIMER>;
using CALC = timer::Calculator<TIMER>;

// Frequency for PWM
constexpr const uint16_t PWM_FREQUENCY = 450;
constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALC::FastPWM_prescaler(PWM_FREQUENCY);

int main()
{
	board::init();
	// Initialize timer and pins
	TIMER_TYPE timer{timer::TimerMode::FAST_PWM, PRESCALER};
	LED_OUTPUT led{timer};
	ANALOG_INPUT pot;

	// Start timer
	timer._begin();
	
	// Enable interrupts
	sei();
	
	// Loop of samplings
	LED_OUTPUT::TYPE duty = 0;
	while (true)
	{
		ANALOG_INPUT::TYPE value = pot.sample();
		if (duty != value)
		{
			duty = value;
			led.set_duty(duty);
		}
		time::delay_ms(100);
	}
	return 0;
}
