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
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D9_PB1_OC1A;
static constexpr const board::Timer TIMER1 = board::Timer::TIMER1;
#elif defined (ARDUINO_MEGA)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D4_PG5_OC1B;
static constexpr const board::Timer TIMER1 = board::Timer::TIMER1;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
static constexpr const board::DigitalPin LED1 = board::PWMPin::D11_PB2_OC1A;
static constexpr const board::Timer TIMER1 = board::Timer::TIMER1;
#else
#error "Current target is not yet supported!"
#endif

using ANALOG1_INPUT = analog::AnalogInput<POT1, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;
using LED1_OUTPUT = analog::PWMOutput<LED1>;
using TIMER1_TYPE = timer::PulseTimer<TIMER1>;
using TIMER1_DUTY_TYPE = TIMER1_TYPE::TIMER_TYPE;

// Pulse Frequency
constexpr const uint16_t PULSE_FREQUENCY = 50;

// Constants for LED1
constexpr const uint16_t PULSE1_MAXWIDTH_US = 2000;
constexpr const uint16_t PULSE1_MINWIDTH_US = 1000;

//TODO Implement later constexpr struct defining MIN/MAX pulse
//template<typename PWM, typename INPUT, typename PULSE>
//void update_pulse(PWM pin, INPUT value, PULSE& duty)
//{
//	
//}

int main()
{
	// Initialize timer and pins
	TIMER1_TYPE timer1{PULSE1_MAXWIDTH_US, PULSE_FREQUENCY};
	LED1_OUTPUT led1{timer1};
	ANALOG1_INPUT pot1;

	// Start timer
	timer1.register_pin(LED1_OUTPUT::COM);
	timer1._begin();
	
	// Enable interrupts
	sei();
	
	// Loop of samplings
	LED1_OUTPUT::TYPE pulse1 = 0;
	while (true)
	{
		ANALOG1_INPUT::TYPE value1 = pot1.sample();
		led1.set_duty(value1 << 4);
//		LED1_OUTPUT::TYPE pulse = value1 * (PULSE1_MAXWIDTH_US - PULSE1_MINWIDTH_US) / 256UL + PULSE1_MINWIDTH_US;
//		if (pulse1 != pulse)
//		{
//			pulse1 = pulse;
//			led1.set_duty(pulse1);
//		}
		time::delay_ms(100);
	}
	return 0;
}
