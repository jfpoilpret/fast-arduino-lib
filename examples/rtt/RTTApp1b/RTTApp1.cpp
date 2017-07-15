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
 * Real Time Timer example. Take #1
 * This program shows usage of FastArduino Timer-based RTT (Real Time Timer) support.
 * It checks RTT with all available timers of the target board.
 * For each available timer on the target platform, the program blinks a LED 5 times with a period of 10 seconds.
 * 
 * Wiring:
 * - on Arduino UNO, Arduino LEONARDO and Arduino MEGA:
 *   - no wiring needed as the program uses default LED on D13
 * - on ATmega328P based boards:
 *   - D13 (PB5) connected to a LED through a 330Ohm resistor then linked to GND
 * - on ATtinyX4 based boards:
 *   - D7 (LED, PA7) connected to a LED through a 330Ohm resistor then linked to GND
 */

#include <fastarduino/gpio.h>
#include <fastarduino/realtime_timer.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
// Define vectors we need in the example
REGISTER_RTT_ISR(0)
REGISTER_RTT_ISR(1)
REGISTER_RTT_ISR(2)
#elif defined (ARDUINO_LEONARDO)
// Define vectors we need in the example
REGISTER_RTT_ISR(0)
REGISTER_RTT_ISR(1)
REGISTER_RTT_ISR(3)
#elif defined (ARDUINO_MEGA)
// Define vectors we need in the example
REGISTER_RTT_ISR(0)
REGISTER_RTT_ISR(1)
REGISTER_RTT_ISR(2)
REGISTER_RTT_ISR(3)
REGISTER_RTT_ISR(4)
REGISTER_RTT_ISR(5)
#elif defined (BREADBOARD_ATTINYX4)
// Define vectors we need in the example
REGISTER_RTT_ISR(0)
REGISTER_RTT_ISR(1)
#else
#error "Current target is not yet supported!"
#endif

const constexpr uint32_t BLINK_DELAY = 10000;

template<board::Timer TIMER>
void check_timer()
{
	typename gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT, false};
	timer::RTT<TIMER> rtt;
	rtt.register_rtt_handler();
	rtt.begin();
	// Event Loop
	for (uint8_t i = 0; i < 5; ++i)
	{
		led.toggle();
		rtt.delay(BLINK_DELAY);
	}
	rtt.end();
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

#if defined (BREADBOARD_ATTINYX4)
	check_timer<board::Timer::TIMER0>();
	check_timer<board::Timer::TIMER1>();
#elif defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
	check_timer<board::Timer::TIMER0>();
	check_timer<board::Timer::TIMER1>();
	check_timer<board::Timer::TIMER2>();
#elif defined (ARDUINO_LEONARDO)
	check_timer<board::Timer::TIMER0>();
	check_timer<board::Timer::TIMER1>();
	check_timer<board::Timer::TIMER3>();
#elif defined (ARDUINO_MEGA)
	check_timer<board::Timer::TIMER0>();
	check_timer<board::Timer::TIMER1>();
	check_timer<board::Timer::TIMER2>();
	check_timer<board::Timer::TIMER3>();
	check_timer<board::Timer::TIMER4>();
	check_timer<board::Timer::TIMER5>();
#endif
}
