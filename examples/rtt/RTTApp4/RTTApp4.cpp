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
 * Real Time Timer example. Take #4
 * This program shows usage of FastArduino Timer-based RTT (Real Time Timer) support along with periodic job scheduling.
 * The program blinks a LED at a half-period of 5 seconds, forever.
 * 
 * Wiring:
 * - on Arduino UNO and Arduino MEGA:
 *   - no wiring needed as the program uses default LED on D13
 * - on ATmega328P based boards:
 *   - D13 (PB5) connected to a LED through a 330Ohm resistor then linked to GND
 * - on ATtinyX4 based boards:
 *   - D7 (LED, PA7) connected to a LED through a 330Ohm resistor then linked to GND
 */

#include <fastarduino/gpio.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/events.h>
#include <fastarduino/scheduler.h>

using namespace events;
using EVENT = Event<void>;

static const uint16_t RTT_EVENT_PERIOD = 1024;

// Define vectors we need in the example
REGISTER_RTT_EVENT_ISR(0, EVENT, RTT_EVENT_PERIOD)

static const uint32_t PERIOD = 5000;

class LedHandler: public Job
{
public:
	LedHandler() : Job{0, PERIOD}, _led{gpio::PinMode::OUTPUT, false} {}

protected:
	virtual void on_schedule(UNUSED uint32_t millis) override
	{
		_led.toggle();
	}
	
private:
	typename gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;
};

// Define event queue
static const uint8_t EVENT_QUEUE_SIZE = 32;
static EVENT buffer[EVENT_QUEUE_SIZE];
static containers::Queue<EVENT> event_queue{buffer};

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	timer::RTTEventCallback<EVENT, RTT_EVENT_PERIOD> callback{event_queue};
	timer::RTT<board::Timer::TIMER0> rtt;
	interrupt::register_handler(callback);
	
	// Prepare Dispatcher and Handlers
	Dispatcher<EVENT> dispatcher;
	Scheduler<timer::RTT<board::Timer::TIMER0>, EVENT> scheduler{rtt, Type::RTT_TIMER};
	dispatcher.insert(scheduler);

	LedHandler job;
	scheduler.schedule(job);

	// Start RTT clock
	rtt.begin();

	// Event Loop
	while (true)
	{
		EVENT event = pull(event_queue);
		dispatcher.dispatch(event);
	}
}
