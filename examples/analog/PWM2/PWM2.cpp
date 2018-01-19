//   Copyright 2016-2018 Jean-Francois Poilpret
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
 * Use 2 potentiometers to set 2 LED light levels through 2 PWM channel of one timer.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - A1: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D5: LED connected to GND through a 1K resistor 
 *   - D6: LED connected to GND through a 1K resistor 
 * - on Arduino MEGA:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - A1: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D9: LED connected to GND through a 1K resistor 
 *   - D10: LED connected to GND through a 1K resistor 
 * - on ATtinyX4 based boards:
 *   - A0 (PA0): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - A1 (PA1): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D10 (PB2): LED connected to GND through a 1K resistor 
 *   - D7 (PA7): LED connected to GND through a 1K resistor 
 */

#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/analog_input.h>
#include <fastarduino/pwm.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A0;
static constexpr const board::AnalogPin POT2 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D6_PD6_OC0A;
static constexpr const board::DigitalPin LED2 = board::PWMPin::D5_PD5_OC0B;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
#elif defined (ARDUINO_LEONARDO)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A0;
static constexpr const board::AnalogPin POT2 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D11_PB7_OC0A;
static constexpr const board::DigitalPin LED2 = board::PWMPin::D3_PD0_OC0B;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
#elif defined (ARDUINO_MEGA)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A0;
static constexpr const board::AnalogPin POT2 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D10_PB4_OC2A;
static constexpr const board::DigitalPin LED2 = board::PWMPin::D9_PH6_OC2B;
static constexpr const board::Timer NTIMER = board::Timer::TIMER2;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A0;
static constexpr const board::AnalogPin POT2 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D10_PB2_OC0A;
static constexpr const board::DigitalPin LED2 = board::PWMPin::D7_PA7_OC0B;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
#elif defined (BREADBOARD_ATTINYX5)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::AnalogPin POT2 = board::AnalogPin::A2;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D0_PB0_OC0A;
static constexpr const board::DigitalPin LED2 = board::PWMPin::D1_PB1_OC0B;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
#else
#error "Current target is not yet supported!"
#endif

using ANALOG_INPUT1 = analog::AnalogInput<POT1, uint8_t, board::AnalogReference::AVCC, board::AnalogClock::MAX_FREQ_200KHz>;
using ANALOG_INPUT2 = analog::AnalogInput<POT2, uint8_t, board::AnalogReference::AVCC, board::AnalogClock::MAX_FREQ_200KHz>;
using LED_OUTPUT1 = analog::PWMOutput<LED1>;
using LED_OUTPUT2 = analog::PWMOutput<LED2>;
using CALC = timer::Calculator<NTIMER>;
using TIMER = timer::Timer<NTIMER>;

// Frequency for PWM
constexpr const uint16_t PWM_FREQUENCY = 450;
constexpr const TIMER::PRESCALER PRESCALER = CALC::FastPWM_prescaler(PWM_FREQUENCY);

int main()
{
	board::init();
	// Initialize timer and pins
	TIMER timer{timer::TimerMode::FAST_PWM, PRESCALER};
	LED_OUTPUT1 led1{timer};
	LED_OUTPUT2 led2{timer};
	ANALOG_INPUT1 pot1;
	ANALOG_INPUT2 pot2;

	// Start timer
	timer.begin_();
	
	// Enable interrupts
	sei();
	
	// Loop of samplings
	LED_OUTPUT1::TYPE duty1 = 0;
	LED_OUTPUT2::TYPE duty2 = 0;
	while (true)
	{
		ANALOG_INPUT1::TYPE value1 = pot1.sample();
		ANALOG_INPUT2::TYPE value2 = pot2.sample();
		if (duty1 != value1)
		{
			duty1 = value1;
			led1.set_duty(duty1);
		}
		if (duty2 != value2)
		{
			duty2 = value2;
			led2.set_duty(duty2);
		}
		time::delay_ms(100);
	}
	return 0;
}
