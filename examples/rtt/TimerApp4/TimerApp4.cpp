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
 * Timer compilation example.
 * Shows how to use 2 CTC Timers (not RTT) to blink 1 LED for some period of time, then stop completely.
 * 
 * Wiring: 
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D7 (PA7) LED connected to ground through a resistor
 */

#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>
#include <fastarduino/time.h>

#if defined(BREADBOARD_ATTINYX5)
// ATtinyX5 timers are only 8 bits, even with max prescaler (16384), it is not 
// possible to generate ticks at 4s period, only 0.5s is possible (counter = 244)
constexpr const uint32_t SUSPEND_PERIOD_US = 500000;
#else
constexpr const uint32_t SUSPEND_PERIOD_US = 4000000;
#endif

constexpr const board::Timer BLINK_NTIMER = board::Timer::TIMER0;
using BLINK_CALC = timer::Calculator<BLINK_NTIMER>;
using BLINK_TIMER = timer::Timer<BLINK_NTIMER>;
constexpr const uint32_t BLINK_PERIOD_US = 10000;
constexpr const BLINK_TIMER::PRESCALER BLINK_PRESCALER = BLINK_CALC::CTC_prescaler(BLINK_PERIOD_US);
static_assert(BLINK_CALC::is_adequate_for_CTC(BLINK_PRESCALER, BLINK_PERIOD_US), 
		"BLINK_TIMER_TYPE::is_adequate(BLINK_PRESCALER, BLINK_PERIOD_US)");
constexpr const BLINK_TIMER::TYPE BLINK_COUNTER = BLINK_CALC::CTC_counter(BLINK_PRESCALER, BLINK_PERIOD_US);

constexpr const board::Timer SUSPEND_NTIMER = board::Timer::TIMER1;
using SUSPEND_CALC = timer::Calculator<SUSPEND_NTIMER>;
using SUSPEND_TIMER = timer::Timer<SUSPEND_NTIMER>;
constexpr const SUSPEND_TIMER::PRESCALER SUSPEND_PRESCALER = SUSPEND_CALC::CTC_prescaler(SUSPEND_PERIOD_US);
static_assert(SUSPEND_CALC::is_adequate_for_CTC(SUSPEND_PRESCALER, SUSPEND_PERIOD_US), 
		"SUSPEND_TIMER_TYPE::is_adequate(SUSPEND_PRESCALER, SUSPEND_PERIOD_US)");
constexpr const SUSPEND_TIMER::TYPE SUSPEND_COUNTER = SUSPEND_CALC::CTC_counter(SUSPEND_PRESCALER, SUSPEND_PERIOD_US);

class BlinkHandler
{
public:
	BlinkHandler(): _led{gpio::PinMode::OUTPUT, false} {}
	
	void on_timer()
	{
		_led.set();
		time::delay_us(1000);
		_led.clear();
	}
	
private:
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;
};

class SuspendHandler
{
public:
	SuspendHandler(BLINK_TIMER& blink_timer):_blink_timer{blink_timer} {}
	
	void on_timer()
	{
		if (_blink_timer.is_suspended())
			_blink_timer.resume_();
		else
			_blink_timer.suspend_();
	}
	
private:
	BLINK_TIMER& _blink_timer;
};

// Define vectors we need in the example
REGISTER_TIMER_COMPARE_ISR_METHOD(0, BlinkHandler, &BlinkHandler::on_timer)
REGISTER_TIMER_COMPARE_ISR_METHOD(1, SuspendHandler, &SuspendHandler::on_timer)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	BlinkHandler blink_handler;
	BLINK_TIMER blink_timer{timer::TimerMode::CTC, BLINK_PRESCALER, timer::TimerInterrupt::OUTPUT_COMPARE_A};
	SuspendHandler suspend_handler{blink_timer};
	SUSPEND_TIMER suspend_timer{timer::TimerMode::CTC, SUSPEND_PRESCALER, timer::TimerInterrupt::OUTPUT_COMPARE_A};
	interrupt::register_handler(blink_handler);
	interrupt::register_handler(suspend_handler);
	blink_timer.begin_(BLINK_COUNTER);
	suspend_timer.begin_(SUSPEND_COUNTER);
	sei();
	
	while (true) ;
}
