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
#include <fastarduino/timer.h>
#include <fastarduino/analog_input.h>
#include <fastarduino/pwm.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const board::AnalogPin POT0 = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED0 = board::PWMPin::D6_PD6_OC0A;
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D5_PD5_OC0B;
static constexpr const board::Timer TIMER0 = board::Timer::TIMER0;
#elif defined (ARDUINO_MEGA)
static constexpr const board::AnalogPin POT0 = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED0 = board::PWMPin::D4_PG5_OC0B;
static constexpr const board::Timer TIMER0 = board::Timer::TIMER0;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::AnalogPin POT0 = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED0 = board::PWMPin::D00_PB2_OC0A;
static constexpr const board::Timer TIMER0 = board::Timer::TIMER0;
#else
#error "Current target is not yet supported!"
#endif

using CALC0 = timer::Calculator<TIMER0>;
using PRESCALER0_TYPE = CALC0::TIMER_PRESCALER;

// Constants for LED0
constexpr const uint16_t PULSE0_MAXWIDTH_US = 2000;
constexpr const uint16_t PULSE0_MINWIDTH_US = 1000;

// Pulse Frequency
constexpr const uint16_t PULSE_FREQUENCY = 50;
constexpr const PRESCALER0_TYPE PRESCALER0 = CALC0::PulseTimer_prescaler(PULSE0_MAXWIDTH_US, PULSE_FREQUENCY);
static_assert(PRESCALER0 == PRESCALER0_TYPE::DIV_256, "");

// Register ISR needed for PulseTimer (8 bits specific)
REGISTER_PULSE_TIMER8_ISR(0, PRESCALER0, LED0, LED1)

using ANALOG0_INPUT = analog::AnalogInput<POT0, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;
using LED0_OUTPUT = analog::PWMOutput<LED0>;
using ANALOG1_INPUT = analog::AnalogInput<POT1, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;
using LED1_OUTPUT = analog::PWMOutput<LED1>;
using TIMER0_TYPE = timer::PulseTimer8<TIMER0, PRESCALER0>;
using TIMER0_DUTY_TYPE = TIMER0_TYPE::TIMER_TYPE;

// Rework useful Arduino functions map() and constrain()
//TODO push to utilities?
template<typename T>
constexpr T constrain(T value, T min, T max)
{
	return value < min ? min : value > max ? max : value;
}
template<typename TI, typename TO>
constexpr TO map(TI value, TI input_range, TO output_min, TO output_max)
{
	return TO (value * (output_max - output_min) / input_range + output_min);
}
template<typename TI, typename TO>
constexpr TO map(TI value, TI input_min, TI input_max, TO output_min, TO output_max)
{
	return map(value, input_max - input_min, output_min, output_max);
}

int main()
{
	// Initialize timer and pins
	TIMER0_TYPE timer0{PULSE_FREQUENCY};
	LED0_OUTPUT led0{timer0};
	ANALOG0_INPUT pot0;
	LED1_OUTPUT led1{timer0};
	ANALOG1_INPUT pot1;

	// Start timer
	timer0._begin();
	
	// Enable interrupts
	sei();
	
	// Loop of samplings
	uint16_t pulse0 = 0;
	uint16_t pulse1 = 0;
	while (true)
	{
		uint32_t input0 = pot0.sample();
		uint16_t pulse = map(input0, 256UL, PULSE0_MINWIDTH_US, PULSE0_MAXWIDTH_US);
		if (pulse0 != pulse)
		{
			pulse0 = pulse;
			led0.set_duty(CALC0::PulseTimer_value(PRESCALER0, pulse0));
		}
		uint32_t input1 = pot1.sample();
		pulse = map(input1, 256UL, PULSE0_MINWIDTH_US, PULSE0_MAXWIDTH_US);
		if (pulse1 != pulse)
		{
			pulse1 = pulse;
			led1.set_duty(CALC0::PulseTimer_value(PRESCALER0, pulse1));
		}
		time::delay_ms(100);
	}
	return 0;
}
