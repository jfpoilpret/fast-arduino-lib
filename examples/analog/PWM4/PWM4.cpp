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
 * Use potentiometer to set 2 LEDs light level through PulseTimer-based PWM.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - A1: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D5: LED connected to GND through a 1K resistor 
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

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
static constexpr const board::AnalogPin POT0 = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED0 = board::PWMPin::D6_PD6_OC0A;
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D5_PD5_OC0B;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
#elif defined (ARDUINO_LEONARDO)
static constexpr const board::AnalogPin POT0 = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED0 = board::PWMPin::D11_PB7_OC0A;
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D3_PD0_OC0B;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
#elif defined (ARDUINO_MEGA)
static constexpr const board::AnalogPin POT0 = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED0 = board::PWMPin::D13_PB7_OC0A;
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D4_PG5_OC0B;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::AnalogPin POT0 = board::AnalogPin::A0;
static constexpr const board::DigitalPin LED0 = board::PWMPin::D10_PB2_OC0A;
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D7_PA7_OC0B;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
#else
#error "Current target is not yet supported!"
#endif

using CALC = timer::Calculator<NTIMER>;
using PRESCALER = CALC::PRESCALER;

// Constants for LED0
constexpr const uint16_t PULSE0_MAXWIDTH_US = 2000;
constexpr const uint16_t PULSE0_MINWIDTH_US = 1000;

// Pulse Frequency
constexpr const uint16_t PULSE_FREQUENCY = 50;
constexpr const PRESCALER PRESCALER0 = CALC::PulseTimer_prescaler(PULSE0_MAXWIDTH_US, PULSE_FREQUENCY);
#if F_CPU == 8000000UL
static_assert(PRESCALER0 == PRESCALER::DIV_64, "");
#else
static_assert(PRESCALER0 == PRESCALER::DIV_256, "");
#endif

// Register ISR needed for PulseTimer (8 bits specific)
// NOTE ISR to register are different based on pins used and their number (1 or 2)
REGISTER_PULSE_TIMER8_AB_ISR(0, PRESCALER0, LED0, LED1)
//REGISTER_PULSE_TIMER8_A_ISR(0, PRESCALER0, LED0)
//REGISTER_PULSE_TIMER8_B_ISR(0, PRESCALER0, LED1)

using ANALOG0_INPUT = analog::AnalogInput<POT0, uint8_t, board::AnalogReference::AVCC, board::AnalogClock::MAX_FREQ_200KHz>;
using LED0_OUTPUT = analog::PWMOutput<LED0, true>;
using ANALOG1_INPUT = analog::AnalogInput<POT1, uint8_t, board::AnalogReference::AVCC, board::AnalogClock::MAX_FREQ_200KHz>;
using LED1_OUTPUT = analog::PWMOutput<LED1, true>;
using TIMER = timer::PulseTimer<NTIMER, PRESCALER0>;
using TIMER_DUTY_TYPE = TIMER::TYPE;

template<typename IN, typename OUT>
void update(IN& in, OUT& out, uint16_t old_pulse)
{
	uint32_t input = in.sample();
	uint16_t pulse = utils::map(input, 0UL, 256UL, PULSE0_MINWIDTH_US, PULSE0_MAXWIDTH_US);
	if (old_pulse != pulse)
	{
		old_pulse = pulse;
		out.set_duty(CALC::PulseTimer_value(PRESCALER0, pulse));
	}
}

int main()
{
	board::init();
	// Initialize timer and pins
	TIMER timer{PULSE_FREQUENCY};
	LED0_OUTPUT led0{timer};
	ANALOG0_INPUT pot0;
	LED1_OUTPUT led1{timer};
	ANALOG1_INPUT pot1;

	// Start timer
	timer.begin_();
	
	// Enable interrupts
	sei();
	
	// Loop of samplings
	uint16_t pulse0 = 0;
	uint16_t pulse1 = 0;
	while (true)
	{
		update(pot0, led0, pulse0);
		update(pot1, led1, pulse1);
		time::delay_ms(100);
	}
	return 0;
}
