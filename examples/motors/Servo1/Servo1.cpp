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
 * Use potentiometer to set servo arm angle through Servo API.
 * This example uses an 8-bit timer.
 * The servo I use in this example is a TowerPro SG90.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A1: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D6: connected to servo signal pin (orange wire)
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/analog_input.h>
#include <fastarduino/time.h>
#include <fastarduino/pulse_timer.h>
#include <fastarduino/devices/servo.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define TIMER_NUM 0
constexpr const board::Timer NTIMER = board::Timer::TIMER0;
// PIN connected to servo signal
constexpr const board::PWMPin SERVO_PIN1 = board::PWMPin::D6_PD6_OC0A;
constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
#define REGISTER_PULSE_TIMER8_ISR REGISTER_PULSE_TIMER8_A_ISR
#elif defined(ARDUINO_MEGA)
#define TIMER_NUM 0
constexpr const board::Timer NTIMER = board::Timer::TIMER0;
// PIN connected to servo signal
constexpr const board::PWMPin SERVO_PIN1 = board::PWMPin::D4_PG5_OC0B;
constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
#define REGISTER_PULSE_TIMER8_ISR REGISTER_PULSE_TIMER8_B_ISR
#elif defined(ARDUINO_LEONARDO)
#define TIMER_NUM 0
constexpr const board::Timer NTIMER = board::Timer::TIMER0;
// PIN connected to servo signal
constexpr const board::PWMPin SERVO_PIN1 = board::PWMPin::D11_PB7_OC0A;
constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
#define REGISTER_PULSE_TIMER8_ISR REGISTER_PULSE_TIMER8_A_ISR
#elif defined(BREADBOARD_ATTINYX4)
#define TIMER_NUM 0
constexpr const board::Timer NTIMER = board::Timer::TIMER0;
// PIN connected to servo signal
constexpr const board::PWMPin SERVO_PIN1 = board::PWMPin::D10_PB2_OC0A;
constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
#define REGISTER_PULSE_TIMER8_ISR REGISTER_PULSE_TIMER8_A_ISR
#else
#error "Current target is not yet supported!"
#endif

using TCALC = timer::Calculator<NTIMER>;
using TPRESCALER = TCALC::PRESCALER;

// Constants for servo and prescaler to be used for TIMER
constexpr const uint16_t MAX_PULSE_US = 2400;
constexpr const uint16_t MIN_PULSE_US = 544;
constexpr const uint16_t NEUTRAL_PULSE_US = 1500;
constexpr const uint16_t PULSE_FREQUENCY = 50;
constexpr const TPRESCALER PRESCALER = TCALC::PulseTimer_prescaler(MAX_PULSE_US, PULSE_FREQUENCY);

// Predefine types used for Timer and Servo
using PULSE_TIMER = timer::PulseTimer<NTIMER, PRESCALER>;
using SERVO1 = devices::servo::Servo<PULSE_TIMER, SERVO_PIN1>;

using ANALOG1_INPUT = analog::AnalogInput<POT1, uint8_t, board::AnalogReference::AVCC, board::AnalogClock::MAX_FREQ_200KHz>;

// Register ISR needed for PulseTimer (8 bits specific)
REGISTER_PULSE_TIMER8_ISR(TIMER_NUM, PRESCALER, SERVO_PIN1)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	
	// Instantiate pulse timer for servo
	PULSE_TIMER servo_timer{PULSE_FREQUENCY};
	// Instantiate servo
	SERVO1 servo1{servo_timer, MIN_PULSE_US, MAX_PULSE_US, NEUTRAL_PULSE_US};
	// Start pulse timer
	servo_timer.begin_();
	// Enable interrupts
	sei();

	ANALOG1_INPUT pot1;
	
//	servo1.detach();
	while (true)
	{
		uint16_t input1 = pot1.sample();
		// 3 API methods are available to set the Servo signal
		// 1. Direct timer counter value (0..255 on 8-bits timer, constrained to servo range)
//		servo1.set_counter(input1);
		// 2. Pulse duration in us (MIN_PULSE_US..MAX_PULSE_US)
//		servo1.set_pulse(MIN_PULSE_US + input1 * 4);
		// 3. Angle in degrees (-90..+90)
		servo1.rotate(int16_t(input1) - 128);
		
		time::delay_ms(100);
	}
}
