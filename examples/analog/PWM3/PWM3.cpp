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
 * Use potentiometer to set LED light level or blink through PulseTimer-based PWM.
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
#include <fastarduino/pulse_timer.h>
#include <fastarduino/analog_input.h>
#include <fastarduino/pwm.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D9_PB1_OC1A;
static constexpr const board::Timer TIMER1 = board::Timer::TIMER1;
#elif defined (ARDUINO_MEGA)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D11_PB5_OC1A;
static constexpr const board::Timer TIMER1 = board::Timer::TIMER1;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D6_PA6_OC1A;
static constexpr const board::Timer TIMER1 = board::Timer::TIMER1;
#else
#error "Current target is not yet supported!"
#endif

using CALC1 = timer::Calculator<TIMER1>;
using PRESCALER1_TYPE = CALC1::TIMER_PRESCALER;

// Constants for LED1
constexpr const uint16_t PULSE1_MAXWIDTH_US = 2000;
constexpr const uint16_t PULSE1_MINWIDTH_US = 1000;

// Pulse Frequency
constexpr const uint16_t PULSE_FREQUENCY = 50;
constexpr const PRESCALER1_TYPE PRESCALER1 = CALC1::PulseTimer_prescaler(PULSE1_MAXWIDTH_US, PULSE_FREQUENCY);

using ANALOG1_INPUT = analog::AnalogInput<POT1, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;
using LED1_OUTPUT = analog::PWMOutput<LED1>;
using TIMER1_TYPE = timer::PulseTimer<TIMER1, PRESCALER1>;
using TIMER1_DUTY_TYPE = TIMER1_TYPE::TIMER_TYPE;

int main()
{
	// Initialize timer and pins
	TIMER1_TYPE timer1{PULSE_FREQUENCY};
	LED1_OUTPUT led1{timer1};
	ANALOG1_INPUT pot1;

	// Start timer
	timer1._begin();
	
	// Enable interrupts
	sei();
	
	// Loop of samplings
	uint16_t pulse1 = 0;
	while (true)
	{
		uint32_t input1 = pot1.sample();
		uint16_t pulse = utils::map(input1, 0UL, 256UL, PULSE1_MINWIDTH_US, PULSE1_MAXWIDTH_US);
		if (pulse1 != pulse)
		{
			pulse1 = pulse;
			led1.set_duty(CALC1::PulseTimer_value(PRESCALER1, pulse1));
		}
		time::delay_ms(100);
	}
	return 0;
}
