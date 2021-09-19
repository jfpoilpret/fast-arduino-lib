//   Copyright 2016-2021 Jean-Francois Poilpret
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
 * Timer compilation example.
 * Shows how to use a CTC Timer (not RTT) to blink a LED.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D7 (PA7) LED connected to ground through a resistor
 * - on ATtinyX5 based boards:
 *   - D0 (PB0) LED connected to ground through a resistor
 * - on ATmega644 based boards:
 *   - D8 (PB0) LED connected to ground through a resistor
 */

#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>

#if defined(BREADBOARD_ATTINYX5)
// ATtinyX5 timers are only 8 bits, even with max prescaler (16384), it is not 
// possible to generate ticks at 1s period, only 0.5s is possible (counter = 244)
constexpr const uint32_t PERIOD_US = 500000;
#else
constexpr const uint32_t PERIOD_US = 1000000;
#endif

constexpr const board::Timer NTIMER = board::Timer::TIMER1;
using CALCULATOR = timer::Calculator<NTIMER>;
using TIMER = timer::Timer<NTIMER>;

constexpr const TIMER::PRESCALER PRESCALER = CALCULATOR::CTC_prescaler(PERIOD_US);
static_assert(CALCULATOR::is_adequate_for_CTC(PRESCALER, PERIOD_US), "TIMER_TYPE::is_adequate(PRESCALER, PERIOD_US)");
constexpr const TIMER::TYPE COUNTER = CALCULATOR::CTC_counter(PRESCALER, PERIOD_US);

class Handler
{
public:
	Handler(): _led{gpio::PinMode::OUTPUT, false} {}
	
	void on_timer()
	{
		_led.toggle();
	}
	
private:
	gpio::FAST_PIN<board::DigitalPin::LED> _led;
};

// Define vectors we need in the example
REGISTER_TIMER_COMPARE_ISR_METHOD(1, Handler, &Handler::on_timer)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	Handler handler;
	interrupt::register_handler(handler);
	TIMER timer{timer::TimerMode::CTC, PRESCALER, timer::TimerInterrupt::OUTPUT_COMPARE_A};
	timer.begin(COUNTER);
	
	while (true) ;
}
