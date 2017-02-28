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
 */

#include <fastarduino/fast_io.h>
#include <fastarduino/timer.h>

constexpr const board::Timer TIMER = board::Timer::TIMER1;
using TIMER_TYPE = timer::Timer<TIMER>;
constexpr const uint32_t PERIOD_US = 1000000;

constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = TIMER_TYPE::prescaler(PERIOD_US);
static_assert(TIMER_TYPE::is_adequate(PRESCALER, PERIOD_US), "TIMER_TYPE::is_adequate(PRESCALER, PERIOD_US)");
constexpr const TIMER_TYPE::TIMER_TYPE COUNTER = TIMER_TYPE::counter(PRESCALER, PERIOD_US);

class Handler
{
public:
	Handler(): _led{gpio::PinMode::OUTPUT, false} {}
	
	void on_timer()
	{
		_led.toggle();
	}
	
private:
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;
};

// Define vectors we need in the example
REGISTER_TIMER_ISR_METHOD(1, Handler, &Handler::on_timer)

int main() __attribute__((OS_main));
int main()
{
	sei();
	Handler handler;
	register_handler(handler);
	TIMER_TYPE timer;
	timer.begin(PRESCALER, COUNTER);
	
	while (true) ;
}
